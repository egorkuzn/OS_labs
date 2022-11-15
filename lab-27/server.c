#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define MAX_FD_COUNT 1024 /* open files limit  */
#define MAX_CLIENTS  510  /* connections limit */
#define MAX_QUEUE    5    /* max queue process */
#define BUFFER_SIZE  80   /* max message size  */ 
#define TIMEOUT      1    /* in seconds        */
#define PCH(a) poll_check(a, __FILE__, __FUNCTION__, __LINE__)
#define FCH(a) fd_check(a, __FILE__, __FUNCTION__, __LINE__)

/* Structure for server params: */
typedef struct {
    int port_node;    /* where to translate       */
    int port_clients; /* where to listen          */
    char* ip_name;    /* address to translate     */
    int listener;     /* fd for clients listening */
    int node;         /* fd for node              */
    char message_from_node[BUFFER_SIZE];
    int clients[MAX_CLIENTS];
    int clients_translator[MAX_CLIENTS];
    struct pollfd fds[MAX_FD_COUNT];
} server_t;

server_t server;

void poll_check(int poll_info,        \
                const char progname[],\
                const char funname[], \
                int line                ) {
    if (poll_info == -1) {
        fprintf(stderr, "%s:%s:%d poll exception\n", progname, funname, line);
        exit(EXIT_FAILURE);
    }
}

void fd_check(int fd,               \
              const char progname[],\
              const char funname[], \
            int line                ) {
    if (fd == -1) {
        fprintf(stderr, "%s:%s:%d fd exception\n", progname, funname, line);
        exit(EXIT_FAILURE);
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

int add_new_client(int new_client) {
    for(int i = 0; i < MAX_FD_COUNT; i++) {
        if (server.fds[i].fd == -1) {
            server.fds[i].fd = new_client;
            server.fds[i].events = POLLIN | POLLOUT;
            return i;
        }
    }

    return -1;
}

int find_new_client_index() {
    // TODO: write new client index finder;

    return 1;
}

void listener_fun(int fd) {
    int new_client_index = find_new_client_index();
    server.clients[new_client_index] = accept(server.listener, (struct sockaddr*) NULL, NULL);

}

bool is_active_client() {
    // TODO: пройтись по live_clients_list[], если есть, то живой
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

void sock_addr_init(struct sockaddr_in* ip_of_server) {
    memset(ip_of_server, NULL, sizeof(*ip_of_server));
    ip_of_server -> sin_family = AF_INET;
    ip_of_server -> sin_addr.s_addr = htonl(INADDR_ANY);
    ip_of_server -> sin_port = htons(server.port_clients);
}

void server_clients_init() {
    memset(server.live_clients_list, -1, sizeof(MAX_CLIENTS));
}

void server_init() {
    struct sockaddr_in ip_of_server;
    sockaddr_init(&ip_of_server);
    server.listener = socket(AF_INET, SOCK_STREAM, NULL);
    bind(server.listener, (struct sockaddr*) &ip_of_server, sizeof(ip_of_server));
    listen(server.listener, MAX_QUEUE);

    server_clients_init();
}

int main(int argc, char* argv[]) {
    get_argv(argc, argv);
    server_init();
    server_fun();
}
