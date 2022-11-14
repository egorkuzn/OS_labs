#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <stdbool.h>

#define MAX_FD_COUNT 1024 /* open files limit  */
#define MAX_USERS    510  /* connections limit */
#define TIMEOUT      1    /* in seconds        */
#define PCH(a) poll_check(a, __FILE__, __FUNCTION__, __LINE__)

/* Structure for server params: */
typedef struct {
    int port_node;    /* where to translate       */
    int port_clients; /* where to listen          */
    char* ip_name;    /* address to translate     */
    int listener;     /* fd for clients listening */
    int node;         /* fd for node              */
    struct pollfd fds[MAX_FD_COUNT];
    int  live_clients_list[MAX_USERS]
} server_t;

server_t server;

void poll_check(int poll_info,        \
                const char progname[],\
                const char funname[], \
                int line                ) {
    if (poll_info == -1) {
        perror("%s:%s:%d poll exception\n");
    }
}

void get_argv(int argc, char* argv[]) {
    if (argc == 4) {
        server.ip_name = argv[1];
        server.port_node = atoi(argv[2]);
        server.port_clients = atoi(argv[1]);
    } else {
        printf("Expected:\n");
        printf("* Node address name;\n");
        printf("* Port for node;\n");
        printf("* Port for clients.\n");
        exit(EXIT_FAILURE);
    }
}

void listener_fun(int fd) {
    
}

bool is_active_client() {
    // пройтись по live_clients_list[], если есть, то живой
}

void node_client_fun(int i) {
    if (server.fds[i].fd == server.node) {
        node_fun();
    } else if (is_active_client(server.fds[i].fd)) {
        client_fun();
    }
}

void poll_iterate() {
    for (int i = 0; i < MAX_FD_COUNT; i++) {
        switch (server.fds[i].events) {
            case POLLIN:
                listener_fun(i);
                break;
            case POLLIN | POLLOUT:
                node_client_fun(i);
                break;
            default:
                break;
        }
    }
}

void server_fun() {
    server.fds[0].fd = server.listener;
    server.fds[0].events = POLLIN;

    while (true) {
        PCH(poll(server.fds, MAX_FD_COUNT, TIMEOUT * 1000));
        poll_iterate();        
    }    
}

int main(int argc, char* argv[]) {
    get_argv(argc, argv);
    server_fun();
}