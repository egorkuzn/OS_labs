#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define MAX_FD_COUNT 1024 /* open files limit  */
#define MAX_CLIENTS  510  /* connections limit */
#define MAX_QUEUE    5    /* max queue process */
#define BUFFER_SIZE  80   /* max message size  */
#define TIMEOUT      1    /* in seconds        */
#define CH(a) check_fun(a, __FILE__, __FUNCTION__, __LINE__)

/* Structure for server params: */
typedef struct {
    int port_server;                         /* server port                   */
    int listener;                            /* fd for clients listening      */
    ssize_t read_bytes;                      /* read bytes real amount        */
    int clients_translator[MAX_CLIENTS];     /* clients translators on server */
    struct pollfd fds[MAX_FD_COUNT];         /* fd array for poll             */
    char client_message[BUFFER_SIZE + 1];    /* client message buffer         */
    char broadcast_message[BUFFER_SIZE + 1]; /* broadcast notification        */
} node_t;

typedef enum{
    READ,
    WRITE
} client_mode_t;

node_t node;

void check_fun(int ret, const char program_name[], const char fun_name[], int line) {
    if (ret == -1) {
        fprintf(stderr, "%s:%s:%d exception\n", program_name, fun_name, line);
        exit(EXIT_FAILURE);
    }
}
/* Getting info from console input: */
void get_argv(int argc, char* argv[]) {
    if (argc == 2) {
        node.port_server = atoi(argv[1]);
    } else {
        printf("Expected:\n");
        printf("* Server port.\n");
        printf("Don't worry, just try again.\n");
        exit(EXIT_FAILURE);
    }
}

int find_new_client_index() {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (node.clients_translator[i] == -1) {
            return i;
        }
    }

    return -1;
}

int accept_new_client(int new_client_index) {
    node.clients_translator[new_client_index] = accept(node.listener, (struct sockaddr*) NULL, NULL);

    if (node.clients_translator[new_client_index] != -1) {
        node.fds[new_client_index].fd = node.clients_translator[new_client_index];
        node.fds[new_client_index].events = POLLIN | POLLOUT;
    }

    return node.clients_translator[new_client_index];
}

void listener_fun() {
    int new_client_index = find_new_client_index();

    if (new_client_index == -1) {
        return; /* Failed connection try */
    }
    /* Successful connection try */
    if (accept_new_client(new_client_index) == -1) {
        return;
    }
}

void disconnect(int i) {
    close(node.fds[i].fd);
    node.fds[i].fd = -1;
    node.clients_translator[i] = -1;
}

void client_fun_io(int i, client_mode_t mode) {
    switch (mode) {
        case READ:
            memset(node.client_message, NULL, BUFFER_SIZE);
            node.read_bytes = read(node.fds[i].fd, node.client_message, BUFFER_SIZE);
            break;
        case WRITE:
            node.read_bytes = write(node.fds[i].fd, node.broadcast_message, BUFFER_SIZE);
            break;
        default:
            break;
    }
}

void client_fun(int i, client_mode_t mode) {
    if (i >= MAX_CLIENTS * 2) {
        printf("Reader out of bounds\n");
        return;
    }

    client_fun_io(i, mode);

    if (node.read_bytes <= 0) {
        disconnect(i);
    } else if (mode == READ) {
        node.client_message[node.read_bytes] = '\0';
        printf("client$%d > %s\n", i, node.client_message);
    }
}

void broadcast_fun() {
    node.read_bytes = read(STDIN_FILENO, node.broadcast_message, BUFFER_SIZE);
    CH(node.read_bytes);
    node.broadcast_message[node.read_bytes] = '\0';
}

void poll_iterate() {
    for (int i = 0; i < MAX_FD_COUNT; i++) {
        if (node.fds[i].fd == -1) {
            continue;
        }

        if (node.fds[i].revents & POLLIN) {
            switch (i) {
                case MAX_FD_COUNT - 1:
                    listener_fun();
                    break;
                case MAX_FD_COUNT - 2:
                    broadcast_fun();
                    break;
                default:
                    client_fun(i, READ);
                    break;
            }
        }

        if (node.fds[i].revents & POLLOUT) {
            client_fun(i, WRITE);
        }
    }
}

void init_poll() {
    for (int i = 0; i < MAX_FD_COUNT; i++) {
        node.fds[i].fd = -1;
    }
}

void node_fun() {
    init_poll();

    node.fds[MAX_FD_COUNT - 1].fd = node.listener;
    node.fds[MAX_FD_COUNT - 1].events = POLLIN;

    node.fds[MAX_FD_COUNT - 2].fd = STDIN_FILENO;
    node.fds[MAX_FD_COUNT - 2].events = POLLIN;

    while (poll(node.fds, MAX_FD_COUNT, TIMEOUT * 1000) != -1) {
        poll_iterate();
    }

    printf("Poll exception.\n");
}

void sockaddr_init(struct sockaddr_in* ip_of_node) {
    memset(ip_of_node, NULL, sizeof(*ip_of_node));

    ip_of_node -> sin_family = AF_INET;
    ip_of_node -> sin_addr.s_addr = htonl(INADDR_ANY);
    ip_of_node -> sin_port = htons(node.port_server);
}

void node_clients_init() {
    memset(&node.clients_translator, -1, MAX_CLIENTS * sizeof(int));
}

void node_init() {
    struct sockaddr_in ip_of_node;
    sockaddr_init(&ip_of_node);
    node.listener = socket(AF_INET, SOCK_STREAM, NULL);
    CH(bind(node.listener, (struct sockaddr*) &ip_of_node, sizeof(ip_of_node)));
    CH(listen(node.listener, MAX_QUEUE));
    node_clients_init();
}

int main(int argc, char* argv[]) {
    get_argv(argc, argv);
    node_init();
    node_fun();
}
