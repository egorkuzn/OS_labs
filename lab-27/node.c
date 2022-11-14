#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <stdbool.h>

#define CLIENTS_AMOUNT 510
#define BUFFER_SIZE 80

typedef struct {    
    int port;
    int listenFd;
    int readBytes;
    struct sockaddr_in servAddr;
    int clients[CLIENTS_AMOUNT];
    char sendBuff[1025];
} config_t;

typedef struct {
    fd_set lfds;
    fd_set cfds;
    fd_set readfds;
    struct timeval timeout;
    char c[BUFFER_SIZE + 1];
} select_config_t;

config_t node = {
    .clients = {-1},
    .sendBuff = {0}
};
select_config_t setting = {
    .timeout = {
        .tv_sec = 0,
        .tv_usec = 0
    }
};

int findFreeUserIndex(int clients[]) {
    for(int i = 0; i < CLIENTS_AMOUNT; i++){
        if(clients[i] == -1) {
            return i;
        }
    }
    return -1;
}

void userDisconnect(int clients[], int clientIndex){
    printf("client %d disconnected\n", clientIndex);
    close(clients[clientIndex]);
    clients[clientIndex] = -1;
}

void servAddrInit() {
    memset(&node.servAddr, 0, sizeof(node.servAddr));
    node.servAddr.sin_family = AF_INET;
    node.servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    node.servAddr.sin_port = htons(node.port);
}

void initConfigStruct() {
    memset(node.clients, -1, sizeof(int) * CLIENTS_AMOUNT);    
    node.listenFd  = socket(AF_INET, SOCK_STREAM, 0);
    shutdown(node.listenFd, SHUT_RDWR);
    
    if (node.port >= 65535){
        printf("Port must be less than 65535");
        exit(EXIT_FAILURE);
    }
    
    servAddrInit();

    if (bind(node.listenFd, (struct sockaddr*)&node.servAddr, sizeof(node.servAddr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    listen(node.listenFd, 5);
}

void getConfig(int argc, char* argv[]) {
    if (argc == 2) {
        node.port = atoi(argv[1]);
    } else {
        printf("Invalid number of arguments\n"); 
        printf("Expected:\n");
        printf("* Port number.\n");
        printf("Try again, please.\n");
        exit(EXIT_FAILURE);
    }
}

bool listenFunction(int listener) {
    if(listener){            
        int newClientIndex = findFreeUserIndex(node.clients);
            
        if(newClientIndex == -1){
            return false;
        }
            
        node.clients[newClientIndex] = accept(node.listenFd, (struct sockaddr*)NULL, NULL);            
        printf("client %d accepted\n", node.clients[newClientIndex]);
    }

    return true;
}

bool clientsRespondFunction() {
    for (int i = 0; i < CLIENTS_AMOUNT; i++) {
                
        if(node.clients[i] == -1){
            return false;
        }
            
        FD_ZERO(&setting.cfds);
        FD_SET(node.clients[i], &setting.cfds);
            
        if(select(node.clients[i] + 1, &setting.cfds, NULL, NULL, &setting.timeout)) {
            if((node.readBytes = read(node.clients[i], &setting.c, BUFFER_SIZE)) <=0) {
                userDisconnect(node.clients, i); // so many questions
                return false;
            }

            setting.c[node.readBytes] = '\0';
            printf("client %d: %s\n", i, setting.c);
        }
                
    }

    return true;
}

bool clientsBroadcastFunction(int broadcast) {
    if(broadcast) {
        if(node.readBytes = read(STDIN_FILENO, &setting.c, BUFFER_SIZE)) {
            setting.c[node.readBytes] = '\0';
                
            for(int i = 0; i < CLIENTS_AMOUNT; i++) {
                if(node.clients[i] == -1){
                    return false;
                }

                write(node.clients[i], setting.c, node.readBytes);
            }
        }
    }

    return true;
}

void closeAll() {
    
}

void nodeFunction() {
    while(true) {
        FD_ZERO(&setting.lfds);
        FD_SET(node.listenFd, &setting.lfds);

        if (!listenFunction(select(node.listenFd + 1, &setting.lfds, NULL, NULL, &setting.timeout))) {
            continue;
        }
            
        if (!clientsRespondFunction()) {
            continue;
        }
            
        FD_ZERO(&setting.readfds);
        FD_SET(STDIN_FILENO, &setting.readfds);
        
        if (!clientsBroadcastFunction(select(STDIN_FILENO + 1, &setting.readfds, NULL, NULL, &setting.timeout))) {
            continue;
        }
    }
}

int main(int argc, char *argv[]) {
    getConfig(argc, argv);
    initConfigStruct();     
    nodeFunction();
}

