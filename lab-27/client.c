#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>

#define BUFFER_SIZE  80   /* max message size  */
#define TIMEOUT      10   /* in seconds        */
#define CH(a) check_fun(a, __FILE__, __FUNCTION__, __LINE__)

/* Structure for server params: */
typedef struct {
    int port_server;             /* server port            */
    char* ip_name;               /* server ip address      */
    ssize_t read_bytes;          /* read bytes real amount */
    time_t last_update_time;     /* time of last update    */
    struct pollfd fds[1];        /* fd array for poll      */
    char message_to_send[BUFFER_SIZE + 1];
    char message_to_receive[BUFFER_SIZE + 1];
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

int time_from_last_update() {
    return (int) difftime(time(NULL), client.last_update_time);
}

void disconnect() {
    printf("Client$ DISCONNECTED\n");
    exit(EXIT_SUCCESS);
}

void socket_fun(client_mode_t mode) {
    CH(client.fds[0].fd);

    switch (mode) {
        case READ:
            client.read_bytes = read(client.fds[0].fd, client.message_to_receive, BUFFER_SIZE);
            break;
        case WRITE:
            if (time_from_last_update() >= 1) {
                client.read_bytes = write(client.fds[0].fd, client.message_to_send, BUFFER_SIZE);
                client.last_update_time = time(NULL);
            }

            break;
        default:
            break;
    }

    CH(client.read_bytes);

    if (mode == READ) {
        if (client.read_bytes == 0) {
            printf("Connection close\n");
            disconnect();
        }

        client.message_to_receive[BUFFER_SIZE] = '\0';

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
}

void init_poll() {
    printf("Poll inited\n");
}

void client_fun() {
    init_poll();

    int ret;

    while ((ret = poll(client.fds, 1, TIMEOUT * 1000)) != -1) {
        if(ret == 0) {
            printf("TIMEOUT\n");
        }

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

    close(client.fds[0].fd);
    client.fds[0].fd = socket(AF_INET, SOCK_STREAM, 0);
    client_addr.sin_port = htons(client.port_server);

    if(connect(client.fds[0].fd, (struct sockaddr*) &client_addr, sizeof(client_addr)) != 0) {
        printf("Connection is failed: server is down.\n");
        printf("Start server or reboot if it works.\n");
        exit(EXIT_FAILURE);
    }

    CH(client.fds[0].fd);
    client.fds[0].events = POLLIN | POLLOUT;
    printf("Client successfully connected to the server.\n");
}

int main(int argc, char* argv[]) {
    get_argv(argc, argv);

    printf("Please, enter some message:");
    scanf("%80[^\n]s", client.message_to_send);
    client_init();
    client_fun();
}
