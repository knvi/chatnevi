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

#define PORT 8080
#define MAX_MSG_SIZE 128
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

sockinfo_t server, user;

void send_recv(void);
void *thread_init(void *ptr);
void *thread_send(void *ptr);
void *thread_recv(void *ptr);

int main() {
    char buf[MAX_MSG_SIZE];
    char server_ip[20];

    printf("Enter the server IP: ");
    scanf("%s", server_ip);
    system("clear");

    bzero(&user, sizeof(user));
    bzero(&server, sizeof(server));

    server.sock_addr.sin_family = PF_INET;
    server.sock_addr.sin_port = htons(PORT);
    inet_aton(server_ip, &server.sock_addr.sin_addr);

    if((user.sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("error while creating the connection endpoint");
        exit(0);
    }

    if(connect(user.sockfd, (SA*)&server.sock_addr, sizeof(server.sock_addr)) == -1) {
        printf("cannot connect to server %s \n", server_ip);
        exit(0);
    } else {
        system("clear");
        printf("welcome to server: %s \n", server_ip);
    }

    send_recv();

    close(user.sockfd);
}

void send_recv(void) {
    pthread_t tid_init, tid_send, tid_recv;

    pthread_create(&tid_init, NULL, thread_init, NULL);
    pthread_join(tid_init, NULL);

    pthread_create(&tid_recv, NULL, thread_recv, NULL);
    pthread_create(&tid_send, NULL, thread_send, NULL);
    pthread_join(tid_recv, NULL);
    pthread_join(tid_send, NULL);
}

void *thread_init(void *ptr) {
    char str[] = "\n";
    char buf[MAX_MSG_SIZE];

    bzero(buf, sizeof(buf));
    recv(user.sockfd, buf, sizeof(buf), 0);
    printf("%s", buf);
    scanf("%s", user.Name);
    send(user.sockfd, user.Name, strlen(user.Name), 0);

    system("clear");
    printf("Welcome %s \n", user.Name);
}

void *thread_recv(void *ptr) {
    char recvBuf[MAX_MSG_SIZE];
    bzero(recvBuf, sizeof(recvBuf));
    
    while(1) {
        if(recv(user.sockfd, recvBuf, sizeof(recvBuf), 0) > 0) {
            printf("%s", recvBuf);
        }

        bzero(recvBuf, sizeof(recvBuf));
        fflush(stdout);
    }
}

void *thread_send(void *ptr) {
    char sendBuf[MAX_MSG_SIZE + 20];
    char str[] = "\n";

    bzero(sendBuf, sizeof(sendBuf));

    while(1) {
        gets(sendBuf);
        strcat(sendBuf, str);
        send(user.sockfd, sendBuf, strlen(sendBuf), 0);

        bzero(sendBuf, sizeof(sendBuf));
        fflush(stdin);
    }
}