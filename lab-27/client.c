#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define BUFFER_SIZE  80   /* max message size  */
#define TIMEOUT      1    /* in seconds        */
#define CH(a) check_fun(a, __FILE__, __FUNCTION__, __LINE__)

/* Structure for server params: */
typedef struct {
    int port_server;             /* server port            */
    char* ip_name;               /* server ip address      */
    ssize_t read_bytes;          /* read bytes real amount */
    struct pollfd fds[2];        /* fd array for poll      */
    char message_to_send[BUFFER_SIZE];
    char message_to_receive[BUFFER_SIZE];
} client_t;

typedef enum{
    READ,
    WRITE
} client_mode_t;

client_t client;

void check_fun(int ret, const char program_name[], const char fun_name[], int line) {
    if (ret == -1) {
        fprintf(stderr, "%s:%s:%d exception\n", program_name, fun_name, line);
        exit(EXIT_FAILURE);
    }
}
/* Getting info from console input: */
void get_argv(int argc, char* argv[]) {
    if (argc == 3) {
        client.ip_name = argv[1];
        client.port_server = atoi(argv[2]);
    } else {
        printf("Expected:\n");
        printf("* Server address name;\n");
        printf("* Port for server.\n");
        printf("Don't worry, just try again.\n");
        exit(EXIT_FAILURE);
    }
}

void socket_fun(client_mode_t mode) {
    CH(client.fds[0].fd);

    switch (mode) {
        case READ:
            client.read_bytes = read(client.fds[0].fd, client.message_to_receive, BUFFER_SIZE);
            break;
        case WRITE:
            client.read_bytes = write(client.fds[0].fd, client.message_to_send, BUFFER_SIZE);
            break;
        default:
            break;
    }

    CH(client.read_bytes);

    if (mode == READ && client.read_bytes > 0) {
        printf("Get from server > %s\n", client.message_to_receive);
    }
}



void poll_iterate() {
        if (client.fds[0].revents & POLLIN) {
            socket_fun(READ);
        }

        if (client.fds[0].revents & POLLOUT) {
            socket_fun(WRITE);
        }

        if (client.fds[1].revents & POLLIN) {
            console_reader_fun();
        }
}

void init_poll() {
    for (int i = 0; i < 2; i++) {
        client.fds[i].fd = -1;
    }
}

void client_fun() {
    init_poll();

    client.fds[1].fd = STDIN_FILENO;
    client.fds[1].events = POLLIN;

    while (poll(client.fds, 2, TIMEOUT * 1000) != -1) {
        poll_iterate();
    }

    printf("Poll exception.\n");
}

void client_init() {
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;

    if (inet_pton(AF_INET, client.ip_name, &client_addr.sin_addr) <= 0) {
        printf("\n inet_pton() error\n");
        exit(EXIT_FAILURE);
    }

    do {
        close(client.fds[0].fd);
        client.fds[0].fd = socket(AF_INET, SOCK_STREAM, 0);
        client_addr.sin_port = htons(client.port_server);
    } while (connect(client.fds[0].fd, (struct sockaddr*) &client_addr, sizeof(client_addr)) != 0);

    CH(client.fds[0].fd);
    client.fds[0].events = POLLIN | POLLOUT;
    printf("Client connected to the server.\n");
}

int main(int argc, char* argv[]) {
    get_argv(argc, argv);
    client_init();
    client_fun();
}
