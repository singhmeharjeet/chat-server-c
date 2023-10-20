
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "list.h"

#define BUF_SIZE 500

#define NO_TERMINATE	 0
#define LOCAL_TERMINATE	 1
#define REMOTE_TERMINATE 2

List* messagesToSend;
List* messagesToDisplay;
pthread_mutex_t sendListMutex, displayListMutex;
pthread_cond_t sendListCond, displayListCond;

pthread_t keyboardThread, udpSenderThread, udpReceiverThread, screenOutputThread;

int terminate = NO_TERMINATE;  // Global flag to indicate when to stop the program
struct timeval timeout = {0, 100000};

// ... Other declarations ...
int sender_socket_fd;	 // Socket descriptor for sending messages
int receiver_socket_fd;	 // Socket descriptor for receiving messages

struct host {
	char* machineName;
	char* port;
	char* msg;
};

int create_socket(char* host, char* port);
void* keyboard_input_thread(void* arg);
void* udp_sender_thread(void* arg);
void* udp_receiver_thread(void* arg);
void* screen_output_thread(void* arg);

int main(int argc, char* argv[]) {
	if (argc != 4) {
		fprintf(stderr, "Usage: %s [my port number] [remote machine name] [remote port number]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	// Initialize shared lists and synchronization tools
	messagesToSend = List_create();
	messagesToDisplay = List_create();

	pthread_mutex_init(&sendListMutex, NULL);
	pthread_mutex_init(&displayListMutex, NULL);

	pthread_cond_init(&sendListCond, NULL);
	pthread_cond_init(&displayListCond, NULL);

	// Prepare the host structures
	struct host localInfo;
	localInfo.machineName = NULL;
	localInfo.port = argv[1];

	struct host remoteInfo;
	remoteInfo.machineName = argv[2];
	remoteInfo.port = argv[3];

	// Create threads
	pthread_create(&keyboardThread, NULL, keyboard_input_thread, NULL);
	pthread_create(&udpSenderThread, NULL, udp_sender_thread, &remoteInfo);

	pthread_create(&udpReceiverThread, NULL, udp_receiver_thread, &localInfo);
	pthread_create(&screenOutputThread, NULL, screen_output_thread, NULL);

	// Join threads
	pthread_join(keyboardThread, NULL);
	pthread_join(udpSenderThread, NULL);
	pthread_join(udpReceiverThread, NULL);
	pthread_join(screenOutputThread, NULL);

	// Clean up and exit
	List_free(messagesToSend, free);
	List_free(messagesToDisplay, free);

	pthread_mutex_destroy(&sendListMutex);
	pthread_mutex_destroy(&displayListMutex);

	pthread_cond_destroy(&sendListCond);
	pthread_cond_destroy(&displayListCond);

	if (terminate == LOCAL_TERMINATE)
		printf("Local Host terminated\n");
	else
		printf("Remote Host terminated\n");

	return EXIT_SUCCESS;
}
int create_socket(char* host, char* port) {
	struct addrinfo hints, *result, *p;
	int sfd;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;	/* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
	hints.ai_protocol = 0;			/* Any protocol */

	if (host == NULL) {
		hints.ai_flags = AI_PASSIVE; /* For wildcard IP address */
	}

	int s = getaddrinfo(host, port, &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	/* getaddrinfo() returns a list of address structures.
		Try each address until we successfully connect(2).
		If socket(2) (or connect(2)/bind(2)) fails, we (close the socket
		and) try the next address. */
	for (p = result; p != NULL; p = p->ai_next) {
		// printf("this is the socketaddr data %s\n", rp->ai_addr->sa_data[]);
		sfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

		if (sfd == -1)
			continue;

		if (host == NULL) {
			if (bind(sfd, p->ai_addr, p->ai_addrlen) == 0) /* server socket */
				break;									   /* Success */
		} else {
			if (connect(sfd, p->ai_addr, p->ai_addrlen) != -1) /* client socket */
				break;										   /* Success */
		}

		close(sfd);
	}

	if (p == NULL) { /* No address succeeded */
		fprintf(stderr, "Could not bind/connect\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);

	return sfd;
}

void* keyboard_input_thread(void* arg) {
	char buffer[BUF_SIZE];

	fd_set input;

	while (1) {
		FD_ZERO(&input);
		FD_SET(STDIN_FILENO, &input);

		int hasInput = select(STDIN_FILENO + 1, &input, NULL, NULL, &timeout);
		if (hasInput <= 0) {
			if (terminate != NO_TERMINATE) {
				break;
			}
			continue;
		}

		bzero(buffer, BUF_SIZE);
		fgets(buffer, BUF_SIZE, stdin);
		fflush(stdin);

		// Remove the trailing newline character if it exists
		size_t len = strlen(buffer);
		if (len > 0 && buffer[len - 1] == '\n') {
			buffer[len - 1] = '\0';	 // Replace newline with null terminator
		}

		if (strcmp(buffer, "!") == 0) {
			terminate = LOCAL_TERMINATE;
			pthread_cond_broadcast(&sendListCond);
			pthread_cond_broadcast(&displayListCond);
			// Next iteration will check the terminate flag and break out of the loop
		}

		pthread_mutex_lock(&sendListMutex);
		List_prepend(messagesToSend, strdup(buffer));
		pthread_cond_signal(&sendListCond);
		pthread_mutex_unlock(&sendListMutex);
	}

	return NULL;
}

// UDP Datagram Sender Thread function
void* udp_sender_thread(void* arg) {
	struct host* remoteInfo = (struct host*)arg;

	sender_socket_fd = create_socket(remoteInfo->machineName, remoteInfo->port);

	while (1) {
		pthread_mutex_lock(&sendListMutex);
		while (List_count(messagesToSend) == 0 && !terminate) {
			pthread_cond_wait(&sendListCond, &sendListMutex);
		}

		// Check if terminate flag is set by the rcv or keyboard thread
		if (terminate == NO_TERMINATE) {
		} else if (terminate == LOCAL_TERMINATE) {
			write(sender_socket_fd, "!", 2);
			break;
		} else if (terminate == REMOTE_TERMINATE) {
			break;
		}

		char* msgToSend = (char*)List_trim(messagesToSend);
		pthread_mutex_unlock(&sendListMutex);

		if (msgToSend) {
			write(sender_socket_fd, msgToSend, strlen(msgToSend));
			free(msgToSend);
		}
	}
	close(sender_socket_fd);
	return NULL;
}

// UDP Datagram Receiver Thread function
void* udp_receiver_thread(void* arg) {
	struct host* localInfo = (struct host*)arg;

	receiver_socket_fd = create_socket(localInfo->machineName, localInfo->port);

	fd_set input;
	ssize_t nread;

	while (1) {
		// Check if terminate flag is set by the keyboard thread
		if (terminate != NO_TERMINATE) {
			break;
		}

		FD_ZERO(&input);
		FD_SET(receiver_socket_fd, &input);

		int hasInput = select(receiver_socket_fd + 1, &input, NULL, NULL, &timeout);
		if (hasInput <= 0) {
			if (terminate != NO_TERMINATE) {
				break;
			} else {
				continue;
			}
		}

		char buf[BUF_SIZE];
		bzero(buf, BUF_SIZE);
		nread = recvfrom(receiver_socket_fd, buf, BUF_SIZE, 0, NULL, NULL);

		if (strcmp(buf, "!") == 0) {
			pthread_cond_signal(&sendListCond);
			pthread_cond_signal(&displayListCond);
			terminate = REMOTE_TERMINATE;
			break;
		}

		if (nread > 0) {
			pthread_mutex_lock(&displayListMutex);
			List_prepend(messagesToDisplay, strndup(buf, nread));
			pthread_cond_signal(&displayListCond);
			pthread_mutex_unlock(&displayListMutex);
		}
	}

	if (receiver_socket_fd > 0) {
		close(receiver_socket_fd);
	}

	return NULL;
}

void* screen_output_thread(void* arg) {
	while (1) {
		pthread_mutex_lock(&displayListMutex);
		while (List_count(messagesToDisplay) == 0 && !terminate) {
			pthread_cond_wait(&displayListCond, &displayListMutex);
		}

		if (terminate != NO_TERMINATE) {
			break;
		}

		char* msgToDisplay = (char*)List_trim(messagesToDisplay);
		pthread_mutex_unlock(&displayListMutex);

		if (msgToDisplay) {
			printf("%s\n", msgToDisplay);
			free(msgToDisplay);
		}
	}
	return NULL;
}
