// #include "list.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <pthread.h>
// #include <string.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <signal.h>
// #include <unistd.h>
// #include <errno.h>
// #include <string.h>
// #include <sys/types.h>


// #define MAX_CLIENTS 2

// int localPort;
// char remoteMachine[256];
// int remotePort;
// int sockfd;
// int connection = 0;

// static _Atomic unsigned int cli_count = 0;


// int main(int argc, char **argv){

//     if (argc != 5) {
//         printf("Usage: s-talk [my port number] [remote machine name] [remote port number]\n");
//         return -1;
//     }

//     localPort = atoi(argv[1]);
//     strncpy(remoteMachine, argv[2], sizeof(remoteMachine));
//     remotePort = atoi(argv[3]);
    
//     sockfd = socket(AF_INET, SOCK_DGRAM, 0);

//     struct sockaddr_in local_addr, remote_addr;

//     local_addr.sin_family = AF_INET;
//     local_addr.sin_port = htons(localPort);
//     local_addr.sin_addr.s_addr = INADDR_ANY;

//     bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr));

//     remote_addr.sin_family = AF_INET;
//     remote_addr.sin_port = htons(remotePort);
//     inet_pton(AF_INET, remoteMachine, &(remote_addr.sin_addr));

//     printf("Welcome to s-talk.\n");

//     while(1){
//         socklen_t clients = sizeof(remote_addr);
//         connection = accept(sockfd, (struct sockaddr*)&remote_addr, &clients);
//         if((cli_count + 1 == MAX_CLIENTS)){
//             printf("Max connections\n");
//             close(connection);
//             continue;
//         }
//     }

// //     if (argc != 4) {
// //         printf("Usage: %s [my port number] [remote machine name] [remote port number]\n", argv[0]);
// //         return -1;
// //     }

// //     printf("Welcome to s-talk.\n");

// //     char* my_port = argv[1];
// //    // char* remote_machine_name = argv[2];
// //     //char* remote_port = argv[3];

// //     messages_to_send = List_create();
// //     messages_to_display = List_create();

// //     pthread_t tid1, tid2, tid3, tid4;

// //     pthread_create(&tid1, NULL, keyboard_input_thread, NULL);
// //     pthread_create(&tid2, NULL, udp_output_thread, (void*)argv + 2); // pass remote_machine_name and remote_port as arguments
// //     pthread_create(&tid3, NULL, udp_input_thread, my_port);
// //     pthread_create(&tid4, NULL, display_output_thread, NULL);

// //     pthread_join(tid1, NULL);
// //     pthread_join(tid2, NULL);
// //     pthread_join(tid3, NULL);
// //     pthread_join(tid4, NULL);

// //     List_free(messages_to_send, free);
// //     List_free(messages_to_display, free);


//     return 0;
// }

// #include <stdio.h>
// #include <string.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netdb.h>
// #include <arpa/inet.h>
// #include <unistd.h>
// #include <errno.h>

// #define BUFFER_SZ 4096

// int main(int argc, char *argv[]) {
//     struct addrinfo hints, *res, *p;
//     char port_str[6];
//     sprintf(port_str, "%d", 6420);
//     int status;
//     char ipstr[INET6_ADDRSTRLEN];
    

//     char buffer[BUFFER_SZ];

//     // Get the hostname of the current machine
//     if(argc != 2){
//         fprintf(stderr,"Usage: showip hostname\n");
//         return 1;
//     }

//     memset(&hints, '\0', sizeof hints);
//     hints.ai_family = AF_UNSPEC; 
//     hints.ai_socktype = SOCK_DGRAM;
//     hints.ai_protocol = 0;

//     if ((status = getaddrinfo(argv[1], port_str, &hints, &res)) != 0) {
//         fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
//         return 1;
//     }

//     printf("Hostname: %s\n", argv[1]);

//     for(p = res; p != NULL; p = p->ai_next) {
//         void *addr;
//         char *ipver;
//         if (p->ai_family == AF_INET) {
//             struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
//             addr = &(ipv4->sin_addr);
//             ipver = "IPv4";
//         } else { // IPv6
//             struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
//             addr = &(ipv6->sin6_addr);
//             ipver = "IPv6";
//         }

//         // Convert the IP to a string and print it:
//         inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
//         printf("IP Address: %s : %s\n", ipver, ipstr);
//     }

//     int sockfd;
//     struct sockaddr_in serv_addr;
//     struct sockaddr_in clin_addr;
//     //pthread_t tid;
//     socklen_t addr_Size = sizeof(clin_addr);

//     sockfd = socket(AF_INET, SOCK_DGRAM, 0);
//     memset(&serv_addr, '\0', sizeof(serv_addr));
//     serv_addr.sin_family = AF_INET;
//     serv_addr.sin_addr.s_addr = INADDR_ANY;
//     serv_addr.sin_port = htons(6420);

//     if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
//         printf("ERROR: BINDING");
//         return 1;
//     }
//     printf("WELCOME TO CHAT ROOM\n");

//     recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr*)&clin_addr , &addr_Size);
//     freeaddrinfo(res); // Free the linked list

//     // You would typically then use recvfrom() to receive UDP packets

//     close(sockfd);
//     return 0;
// }


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>

#include "list.h"


// Global declarations
List* messagesToSend;
List* messagesToDisplay;
pthread_mutex_t sendListMutex, displayListMutex;
pthread_cond_t sendListCond, displayListCond;
bool terminate = false; // Global flag to indicate when to stop the program

// ... [Other function declarations and implementations] ...

struct host
{
    char *machinename;
    char *port;
    char *msg;
};



int create_socket(char *host, char *port)
{
    struct addrinfo hints, *result, *p;
    int sfd;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_protocol = 0;          /* Any protocol */

    if (host == NULL)
    {
        hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */    
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
        sfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol);
        
        if (sfd == -1)
            continue;

        if (host == NULL)
        {
            
            if (bind(sfd, p->ai_addr, p->ai_addrlen) == 0) /* server socket */
               break;                  /* Success */
        }
        else
        {
            if (connect(sfd, p->ai_addr, p->ai_addrlen) != -1) /* client socket */
                break;                  /* Success */
        }

        close(sfd);
    }

    if (p == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not bind/connect\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);           

    return sfd;
}

void displayList(List* pList) {
    if (!pList) {
        printf("List is NULL.\n");
        return;
    }

    Node* temp = pList->head;
    if (!temp) {
        printf("List is empty.\n");
        return;
    }

    printf("[");
    while (temp) {
        if(temp->item) {
              if(temp->item)
            printf("%s ", (char*)temp->item);  // Dereference the item pointer as an int
        } else {
            printf("NULL");
        }
        temp = temp->next;
    }
    printf("]\n");
}

#define BUF_SIZE 500
void* keyboard_input_thread(void* arg) {
    char buffer[BUF_SIZE];
    while (1) {
        if (fgets(buffer, BUF_SIZE, stdin) == NULL) {
            // Handle fgets() error or EOF if necessary
            break;
        }

        // Remove the trailing newline character if it exists
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';  // Replace newline with null terminator
        }

        if (strcmp(buffer, "!") == 0) {
            terminate = true;
            pthread_cond_signal(&sendListCond);
            break;
        }

        pthread_mutex_lock(&sendListMutex);
        List_prepend(messagesToSend, strdup(buffer));
        pthread_cond_signal(&sendListCond);
        pthread_mutex_unlock(&sendListMutex);
    }
    return NULL;
}


// ... Other declarations ...

int sender_socket_fd;  // Socket descriptor for sending messages
int receiver_socket_fd;  // Socket descriptor for receiving messages

// UDP Datagram Sender Thread function
void* udp_sender_thread(void* arg) {
    struct host* remoteInfo = (struct host*) arg;

    sender_socket_fd = create_socket(remoteInfo->machinename, remoteInfo->port);
    
    while (1) {
        pthread_mutex_lock(&sendListMutex);
        while (List_count(messagesToSend) == 0 && !terminate) {
            pthread_cond_wait(&sendListCond, &sendListMutex);
        }
        if (terminate) {
            pthread_mutex_unlock(&sendListMutex);
            close(sender_socket_fd);
            break;
        }
        char* msgToSend = (char*)List_trim(messagesToSend);
        pthread_mutex_unlock(&sendListMutex);

        if (msgToSend) {
            write(sender_socket_fd, msgToSend, strlen(msgToSend));
            free(msgToSend);
        }
        
    }
    return NULL;
}

// UDP Datagram Receiver Thread function
void* udp_receiver_thread(void* arg) {
    struct host* localInfo = (struct host*) arg;

    receiver_socket_fd = create_socket(localInfo->machinename, localInfo->port);
    
    while (1) {
        char buf[BUF_SIZE];
        ssize_t nread = recvfrom(receiver_socket_fd, buf, BUF_SIZE, 0, NULL, NULL);

        if (nread > 0) {
            pthread_mutex_lock(&displayListMutex);
            List_prepend(messagesToDisplay, strndup(buf, nread));
            pthread_cond_signal(&displayListCond);
            pthread_mutex_unlock(&displayListMutex);
        }
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
            pthread_mutex_unlock(&displayListMutex);
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

// ... Main function and others ...




// Signal handler for SIGINT
void handle_sigint(int sig) {
    terminate = true;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s [my port number] [remote machine name] [remote port number]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // // Set up signal handling
    // struct sigaction sa;
    // sa.sa_handler = handle_sigint;
    // sa.sa_flags = 0;
    // sigemptyset(&sa.sa_mask);
    // if (sigaction(SIGINT, &sa, NULL) == -1) {
    //     perror("sigaction");
    //     exit(EXIT_FAILURE);
    // }

    // Initialize shared lists and synchronization tools
    messagesToSend = List_create();
    messagesToDisplay = List_create();
    pthread_mutex_init(&sendListMutex, NULL);
    pthread_mutex_init(&displayListMutex, NULL);
    pthread_cond_init(&sendListCond, NULL);
    pthread_cond_init(&displayListCond, NULL);

    // Prepare the host structures
    struct host localInfo;
    struct host remoteInfo;
    localInfo.machinename = NULL; 
    localInfo.port = argv[1];
    
    remoteInfo.machinename = argv[2];
    remoteInfo.port = argv[3];
    //Create threads
    pthread_t keyboardThread, udpSenderThread, udpReceiverThread, screenOutputThread;
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

    return EXIT_SUCCESS;
}
