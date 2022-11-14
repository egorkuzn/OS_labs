#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdbool.h>

#define TIMEOUT_S 1
#define BUFFER_SIZE 1024
int sockFd = 0;
/* Socket config: */
typedef struct {
    int port;
    char* ipAddrName;
    struct sockaddr_in sockAddrIn;
} config_t;
/* Structure for working with select: */
typedef struct {
    int readBytes;
    fd_set sfds;
    fd_set inds;
    struct timeval timeout;
    char recvBuff[BUFFER_SIZE];
} select_config_t;

config_t client;
select_config_t setting = {
    .readBytes = 0,
    .recvBuff = {0},
    .timeout = {
        .tv_sec = TIMEOUT_S,
        .tv_usec = 0
    }
};

void createFunction() {
    if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Error : Could not create socket \n");
        exit(EXIT_FAILURE);
    }

    client.sockAddrIn.sin_family = AF_INET;
    client.sockAddrIn.sin_port = htons(client.port);

    if (inet_pton(AF_INET, client.ipAddrName, &client.sockAddrIn.sin_addr) <= 0) {
        printf("Cant parse address=%s\n", client.ipAddrName);
        exit(EXIT_FAILURE);
    }

    if (connect(sockFd, (struct sockaddr*) &client.sockAddrIn, sizeof(client.sockAddrIn)) < 0) {
        printf("Connect failed\n");
        exit(EXIT_FAILURE);
    }
}

void getConfig(int argc, char* argv[]) {
    if (argc == 3) {
        client.ipAddrName = argv[1];
        client.port = atoi(argv[2]);
        createFunction();
    } else {
        printf("Invalid number of arguments\n"); 
        printf("Expected:\n");
        printf("* IP address;\n");
        printf("* Port number.\n");
        printf("Try again, please.\n");
        exit(EXIT_FAILURE);
    }
}

void selectBegin() {
    FD_ZERO(&setting.sfds);
	FD_SET(sockFd, &setting.sfds);
	FD_ZERO(&setting.inds);
	FD_SET(STDIN_FILENO, &setting.inds);
}

void receive(int select_socket) {
    if(select_socket) {
		if ((setting.readBytes = read(sockFd, setting.recvBuff, sizeof(setting.recvBuff) - 1)) > 0) {
			setting.recvBuff[setting.readBytes] = 0;
                
			if(fputs(setting.recvBuff, stdout) == EOF) {
					printf("Puts error\n");
			}
		}
	}
}

bool submit(int send_socket) {
    if (send_socket) {
		if ((setting.readBytes = read(STDIN_FILENO, setting.recvBuff, BUFFER_SIZE - 1)) > 0) {
			setting.recvBuff[setting.readBytes - 1] = 0;
            write(sockFd, setting.recvBuff, setting.readBytes);

			if (strcmp("/EXIT", setting.recvBuff) == 0) {
				close(sockFd);
				return false;
			}
		}
	}

    return true;
}

void clientFunction() {
    while (true) {
        selectBegin();

        if (errno != 0) {
			close(sockFd);
			break;
		}

		receive(select(sockFd + 1, &setting.sfds, NULL, NULL, &setting.timeout));

        if(!submit(select(STDIN_FILENO + 1, &setting.inds, NULL, NULL, &setting.timeout))) {
            break;
        }		
	}

    if (setting.readBytes < 0) {
		printf("Read error\n");
	}
}

int main(int argc, char *argv[]) {
    getConfig(argc, argv);  
    clientFunction();
	return 0;
}
