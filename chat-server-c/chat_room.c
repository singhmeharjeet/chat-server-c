
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "list.h"

#define BUF_SIZE 500

#define DONT_TERMINATE	 0
#define LOCAL_TERMINATE	 1
#define REMOTE_TERMINATE 2

List* messagesToSend;
List* messagesToDisplay;
pthread_mutex_t sendListMutex, displayListMutex;
pthread_cond_t sendListCond, displayListCond;

pthread_t keyboardThread, udpSenderThread, udpReceiverThread, screenOutputThread;

int terminate = DONT_TERMINATE;	 // Global flag to indicate when to stop the program
struct timeval timeout = {0, 100000};

// ... Other declarations ...
int sender_socket_fd;	 // Socket descriptor for sending messages
int receiver_socket_fd;	 // Socket descriptor for receiving messages

struct host {
	char* machineName;
	char* port;
};

typedef struct {
	char* nickname;
} Name;

void* udp_sender_thread(void* arg);
void* udp_receiver_thread(void* arg);
void* keyboard_input_thread(void* arg);
void* screen_output_thread(void* arg);
int create_socket(char* host, char* port);

int main(int argc, char* argv[]) {
	if (argc != 4) {
		fprintf(stderr, "Usage: %s [my port number] [remote machine name] [remote port number]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	char nickname[50];
	printf("Enter your nickname: ");
	fgets(nickname, sizeof(nickname), stdin);
	// Removing the trailing newline character.
	nickname[strcspn(nickname, "\n")] = 0;

	Name name;
	name.nickname = nickname;

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
	pthread_create(&keyboardThread, NULL, keyboard_input_thread, &name);
	pthread_create(&udpSenderThread, NULL, udp_sender_thread, &remoteInfo);
	pthread_create(&udpReceiverThread, NULL, udp_receiver_thread, &localInfo);
	pthread_create(&screenOutputThread, NULL, screen_output_thread, NULL);

	// Join threads when they are done running
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
		if (terminate == LOCAL_TERMINATE) {
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
		FD_ZERO(&input);
		FD_SET(receiver_socket_fd, &input);

		int hasInput = select(receiver_socket_fd + 1, &input, NULL, NULL, &timeout);
		if (hasInput <= 0) {
			if (terminate) {
				break;
			}
			continue;
		}

		char buf[BUF_SIZE];
		bzero(buf, BUF_SIZE);
		nread = recvfrom(receiver_socket_fd, buf, BUF_SIZE, 0, NULL, NULL);

		if (strcmp(buf, "!") == 0) {
			terminate = REMOTE_TERMINATE;
			pthread_cond_signal(&sendListCond);
			pthread_cond_signal(&displayListCond);
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

		if (terminate) {
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

void* keyboard_input_thread(void* arg) {
	Name* name = (Name*)arg;
	char buf[BUF_SIZE];
	char msgWithNickname[BUF_SIZE + 50];

	fd_set input;

	while (1) {
		FD_ZERO(&input);
		FD_SET(STDIN_FILENO, &input);

		int hasInput = select(STDIN_FILENO + 1, &input, NULL, NULL, &timeout);
		if (hasInput <= 0) {  // No input from keyboard
			if (terminate != DONT_TERMINATE) {
				break;
			}
			continue;
		}

		if (hasInput && terminate == REMOTE_TERMINATE) {
			// Edge case when you are already typing but termination is requested from the other end
			fflush(stdin);
			break;
		}

		bzero(buf, BUF_SIZE);
		fgets(buf, BUF_SIZE, stdin);
		fflush(stdin);

		// Remove the trailing newline character if it exists
		size_t len = strlen(buf);
		if (len > 0 && buf[len - 1] == '\n') {
			buf[len - 1] = '\0';  // Replace newline with null terminator
		}

		if (strcmp(buf, "!") == 0) {
			terminate = LOCAL_TERMINATE;
			pthread_cond_broadcast(&sendListCond);
			pthread_cond_broadcast(&displayListCond);
		}

		snprintf(msgWithNickname, sizeof(msgWithNickname), "%s: %s", name->nickname, buf);

		pthread_mutex_lock(&sendListMutex);
		List_prepend(messagesToSend, strdup(msgWithNickname));
		pthread_cond_signal(&sendListCond);
		pthread_mutex_unlock(&sendListMutex);
	}

	return NULL;
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
	for (p = result; p != NULL; p = p->ai_next) {
		sfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

		if (sfd == -1)
			continue;

		if (host == NULL) {
			if (bind(sfd, p->ai_addr, p->ai_addrlen) == 0)
				break;
		} else {
			if (connect(sfd, p->ai_addr, p->ai_addrlen) != -1)
				break;
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