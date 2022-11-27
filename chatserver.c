#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

// defines
#define PORT 8080
#define MAX_MSG_SIZE 128
#define SERVER_SIZE 10 // #users in server
#define SA struct sockaddr

typedef struct sockinfo_t {
    int isUsed;
    char Name[20];
    int sockfd;
    struct sockaddr_in sock_addr;
}sockinfo_t;

typedef struct clientmsg_t {
    int isInit;
    char msg[MAX_MSG_SIZE];
}clientsmg_t;

sockinfo_t client_pool[SERVER_SIZE];
sockinfo_t server;
pthread_t tid[SERVER_SIZE];

// functions
void initServer();
void listenConns();
int giveFreeId(sockinfo_t *client);
void *thread_client(void *ptr);

int main(void) {
    initServer(); // at this point we are able to listen to connections using listen()
    listenConns();
}

// function implementations
void initServer() {
    int i, id;

    bzero(&server, sizeof(server)); // zero server
    server.sock_addr.sin_family = PF_INET;
    server.sock_addr.sin_port = htons(PORT);
    server.sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int addrlen = sizeof(struct sockaddr_in);

    if((server.sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("error while creating socket");
        exit(0);
    }

    int optval = 1; // required for setsockopt()
    setsockopt(server.sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)); // set address to be reusable
    signal(SIGPIPE, SIG_IGN); // ignore signal pipe

    if(bind(server.sockfd, (SA*)&server.sock_addr, sizeof(server.sock_addr)) == -1 ) {
        perror("error while binding socket");
        exit(0);
    } 
}

void listenConns() {
    if(listen(server.sockfd, SERVER_SIZE) == -1) {// listening to socket with max. 10 connections
        perror("error while calling listen()");
        exit(0);
    }

    int i, id;
    int addrlen = sizeof(struct sockaddr_in);
    while(1) {
        if((id = giveFreeId(client_pool)) == SERVER_SIZE) {
            printf("Server is full. Please try later");
            exit(0);
        }

        // accept a connection to the socket
        if((client_pool[id].sockfd = accept(server.sockfd, (SA*)&client_pool[id].sock_addr, &addrlen)) == -1) {
            perror("error while accepting connection to the socket");
            exit(0);
        }

        pthread_create(&tid[id], NULL, thread_client, &client_pool[id]);
    }

    close(server.sockfd); // close server
}

int giveFreeId(sockinfo_t *client) {
    int i;
    for(i = 0; (client_pool[i].isUsed != 0) && i < SERVER_SIZE; i++) {
        // do nothing; TODO: find a better solution
    }

    client_pool[i].isUsed = 1;

    return i;
}

int is_empty(char *buf, size_t size)
{
    char *zero = calloc(0, size);
    int i = memcmp(zero, buf, size);
    free(zero);
    return i;
}

void *thread_client(void *ptr) {
    char recvBuf[MAX_MSG_SIZE]; // size in chars
    char sendBuf[MAX_MSG_SIZE + 20];
    sockinfo_t *user = (sockinfo_t*)ptr;
    int i;
    char str[] = "Enter your nickname: ";
    char str2[] = "Enter your message: ";

    // init
    bzero(recvBuf, sizeof(recvBuf));
    bzero(sendBuf, sizeof(sendBuf));

    send(user->sockfd, str, strlen(str), 0);
    recv(user->sockfd, user->Name, sizeof(user->Name), 0);

    printf("Received connection from : %s / Name : %s\n", inet_ntoa(user->sock_addr.sin_addr), user->Name);
    char buf[MAX_MSG_SIZE] = "\n";
    sprintf(buf, "[SERVER] %s joined!\n", user->Name);
    for(int i = 0; i < SERVER_SIZE; i++) {
            if((client_pool[i].isUsed != 0 ) && (client_pool[i].sockfd != user->sockfd)) {
                send(client_pool[i].sockfd, buf, strlen(buf), 0);
        }
    } 
    bzero(buf, sizeof(buf));
    bzero(recvBuf, sizeof(recvBuf));
    bzero(sendBuf, sizeof(sendBuf));

    while(1) {
        if(recv(user->sockfd, recvBuf, sizeof(recvBuf), 0) > 0) {
            if(1 == 1) {
                sprintf(sendBuf, "# %s > ", user->Name);
                strcat(sendBuf, recvBuf);

                for(int i = 0; i < SERVER_SIZE; i++) {
                    if((client_pool[i].isUsed != 0 ) && (client_pool[i].sockfd != user->sockfd)) {
                        send(client_pool[i].sockfd, sendBuf, strlen(sendBuf), 0);
                    }   
                }

                bzero(sendBuf, sizeof(sendBuf));
                bzero(recvBuf, sizeof(recvBuf));
            }
        }
    }
    sprintf(buf, "[SERVER] %s left!\n", user->Name);
    for(int i = 0; i < SERVER_SIZE; i++) {
            if((client_pool[i].isUsed != 0 ) && (client_pool[i].sockfd != user->sockfd)) {
                send(client_pool[i].sockfd, buf, strlen(buf), 0);
        }
    } 
    bzero(buf, sizeof(buf));

    printf("Closed connection with : %s / Name : %s\n", inet_ntoa(user->sock_addr.sin_addr), user->Name);
    user->isUsed = 0; // set id to not used
    close(user->sockfd); // close connection
}