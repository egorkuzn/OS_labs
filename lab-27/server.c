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
    int readBytes;
    int portNode;
    int portClient;
    int listenFd;
    int connfd;
    char* ipAddrName;    
    struct sockaddr_in servAddr;    
    char c[BUFFER_SIZE + 1];
    int clients[CLIENTS_AMOUNT];
    int clientsTrans[CLIENTS_AMOUNT];
    char sendBuff[1025]; 
} config_t;

typedef struct {
    struct timeval timeout;
    fd_set lfds;
    fd_set cfds;
} select_config_t;

config_t server = {
    .listenFd = 0,
    .connfd = 0,
    .sendBuff = {0}
};
select_config_t setting = {
    .timeout = {
        .tv_sec = 1,
        .tv_usec = 0
    }
};

int findFreeUserIndex(int clients[]) {
    for (int i = 0; i < CLIENTS_AMOUNT; i++) {
        if (clients[i] == -1) {
            return i;
        }
    }

    return -1;
}

void checkPort(int port) {
    if (port >= 65535) {
        printf("Port must be less than 65535");
        exit(EXIT_FAILURE);
    }
}
        
void getConfig(int argc, char* argv[]) {
    if (argc == 4) {
        server.ipAddrName = argv[1];
        server.portNode = atoi(argv[2]);
        checkPort(server.portNode);
        server.portClient = atoi(argv[3]);
        checkPort(server.portClient);
    } else {
        printf("Invalid number of arguments\n"); 
        printf("Expected:\n");
        printf("* IP address;\n");
        printf("* Node port number;\n");
        printf("* Client port number.\n");
        printf("Try again, please.\n");
        exit(EXIT_FAILURE);
    }
}

void initServAddr() {
    server.servAddr.sin_family = AF_INET;
    server.servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    server.servAddr.sin_port = htons(server.portClient);
}

void memsetBlock() {
    memset(&server.servAddr, 0, sizeof(server.servAddr));
    memset(server.clients, -1, CLIENTS_AMOUNT * sizeof(int));
    memset(server.clientsTrans, -1, CLIENTS_AMOUNT * sizeof(int));
}

void initServer() {
    server.listenFd = socket(AF_INET, SOCK_STREAM, 0);
    memsetBlock();
    initServAddr();

    if(bind(server.listenFd, (struct sockaddr*) &server.servAddr, sizeof(server.servAddr)) == -1) {
        perror("bind");
        return exit(EXIT_FAILURE);
    }

    listen(server.listenFd, CLIENTS_AMOUNT);
}

bool clientsListener(int listener){
    if (listener) {            
        int newClientIndex = findFreeUserIndex(server.clients);
        printf("Client try connect\n");

        if (newClientIndex == -1) {
            return false;
        }
            
        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        client_addr.sin_family = AF_INET;

        if (inet_pton(AF_INET, server.ipAddrName, &client_addr.sin_addr) <= 0) {
            printf("\n inet_pton error\n");
            exit(EXIT_FAILURE);
        }

        server.clients[newClientIndex] = accept(server.listenFd, (struct sockaddr*) NULL, NULL);
            
        do {
            close(server.clientsTrans[newClientIndex]);
            server.clientsTrans[newClientIndex] = socket(AF_INET, SOCK_STREAM, 0);
            client_addr.sin_port = htons(server.portNode);
        } while (connect(server.clientsTrans[newClientIndex], (struct sockaddr*) &client_addr, sizeof(client_addr)) != 0);
            
        printf("translate %d to %d\n", newClientIndex, server.clientsTrans[newClientIndex]);
    }

    return true;
}

bool readFromClient(int i) {
    FD_ZERO(&setting.cfds);
    FD_SET(server.clients[i], &setting.cfds);
            
    if(select(server.clients[i] + 1, &setting.cfds, NULL, NULL, &setting.timeout)) {
        if((server.readBytes = read(server.clients[i], &server.c, BUFFER_SIZE)) <= 0) {
            printf("client %d disconnected\n", i);
            close(server.clientsTrans[i]);
            close(server.clients[i]);
            server.clientsTrans[i] = -1;
            server.clients[i] = -1;
            return false;
        } 

        server.c[server.readBytes] = '\0';
        write(server.clientsTrans[i], server.c, server.readBytes);
        printf("client %d: %s\n", i, server.c);
    }

    return true;
}

bool writeInClient(int i) {
    FD_ZERO(&setting.cfds);
    FD_SET(server.clientsTrans[i], &setting.cfds);

    if (select(server.clientsTrans[i] + 1, &setting.cfds, NULL, NULL, &setting.timeout)) {
        if (server.readBytes = read(server.clientsTrans[i], &server.c, BUFFER_SIZE)) {
            server.c[server.readBytes] = '\0';
            write(server.clients[i], server.c, server.readBytes);
            printf("client %d: %s\n", i, server.c);
        }
    }

    return true;
}

void closeAll() {

}

void serverFunction() {
    while(true) {
        FD_ZERO(&setting.lfds);
        FD_SET(server.listenFd, &setting.lfds);

        if(!clientsListener(select(server.listenFd + 1, &setting.lfds, NULL, NULL, &setting.timeout))) {
            continue;
        }        
            
        for(int i = 0; i < CLIENTS_AMOUNT; i++) {
            if((server.clients[i] == -1) || !readFromClient(i)){
                continue;
            }
        }
            
        for(int i = 0; i < CLIENTS_AMOUNT; i++) {
            if((server.clientsTrans[i] == -1) || !writeInClient(i)){
                continue;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    getConfig(argc, argv);
    initServer();  
    serverFunction();
}
