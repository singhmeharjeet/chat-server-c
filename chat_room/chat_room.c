#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>


#define MAX_CLIENTS 2

int localPort;
char remoteMachine[256];
int remotePort;
int sockfd;
int connection = 0;

static _Atomic unsigned int cli_count = 0;

int main(int argc, char **argv){

    if (argc != 5) {
        printf("Usage: s-talk [my port number] [remote machine name] [remote port number]\n");
        return -1;
    }

    localPort = atoi(argv[1]);
    strncpy(remoteMachine, argv[2], sizeof(remoteMachine));
    remotePort = atoi(argv[3]);
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in local_addr, remote_addr;

    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(localPort);
    local_addr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr));

    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(remotePort);
    inet_pton(AF_INET, remoteMachine, &(remote_addr.sin_addr));

    printf("Welcome to s-talk.\n");

    while(1){
        socklen_t clients = sizeof(remote_addr);
        connection = accept(sockfd, (struct sockaddr*)&remote_addr, &clients);
        if((cli_count + 1 == MAX_CLIENTS)){
            printf("Max connections\n");
            close(connection);
            continue;
        }
    }

    return 0;
}