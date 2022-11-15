#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <string.h>

#define MAX_FD_COUNT 1024 /* open files limit  */
#define MAX_CLIENTS  510  /* connections limit */
#define MAX_QUEUE    5    /* max queue process */
#define BUFFER_SIZE  80   /* max message size  */ 
#define TIMEOUT      1    /* in seconds        */
#define CH(a) check_fun(a, __FILE__, __FUNCTION__, __LINE__)

/* Structure for server params: */
typedef struct {
    int port_node;    /* where to translate       */
    int port_clients; /* where to listen          */
    char* ip_name;    /* address to translate     */
    int listener;     /* fd for clients listening */
    ssize_t read_bytes;
    char message[BUFFER_SIZE + 1];
    int clients[MAX_CLIENTS];
    int clients_translator[MAX_CLIENTS];
    struct pollfd fds[MAX_FD_COUNT];
} server_t;

server_t server;

void check_fun(int ret, const char program_name[], const char fun_name[], int line) {
    if (ret == -1) {
        fprintf(stderr, "%s:%s:%d exception\n", program_name, fun_name, line);
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
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (server.clients[i] == -1) {
            return i;
        }
    }

    return -1;
}

int accept_new_client(int new_client_index) {
    server.clients[new_client_index] = accept(server.listener, (struct sockaddr*) NULL, NULL);

    if (server.clients[new_client_index] != -1) {
        server.fds[new_client_index].fd = server.clients[new_client_index];
        server.fds[new_client_index].events = POLLIN | POLLOUT;
    }

    return server.clients[new_client_index];
}

int create_new_node_connect(int new_client_index) {
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;

    if (inet_pton(AF_INET, server.ip_name, &client_addr.sin_addr) <= 0) {
        printf("\n inet_pton() error\n");
        exit(EXIT_FAILURE);
    }

    do {
        close(server.clients_translator[new_client_index]);
        server.clients_translator[new_client_index] = socket(AF_INET, SOCK_STREAM, 0);
        client_addr.sin_port = htons(server.port_node);
    } while (connect(server.clients_translator[new_client_index], (struct sockaddr*) &client_addr, sizeof(client_addr)) != 0);

    printf("Translate %d to %d\n", new_client_index, server.clients_translator[new_client_index]);

    if (server.clients_translator[new_client_index] != -1) {
        server.fds[new_client_index + MAX_CLIENTS].fd = server.clients_translator[new_client_index];
        server.fds[new_client_index + MAX_CLIENTS].events = POLLIN | POLLOUT;
    }

    return server.clients_translator[new_client_index];
}

void listener_fun(int fd) {
    int new_client_index = find_new_client_index();
    
    if (new_client_index == -1) {
        return; /* Failed connection try */
    }
    /* Successful connection try */
    if (accept_new_client(new_client_index) == -1) {
        return;
    }

    if (create_new_node_connect(new_client_index) == -1) {
        return;
    }
}

void disconnect(int i) {

}

void client_fun(int i) {
    if (read(server.fds[i].fd, server.message, BUFFER_SIZE) <= 0) {
        disconnect(i);
    }
}

void node_fun(int i) {
    if (read(server.fds[i].fd, server.message, BUFFER_SIZE) <= 0) {
        disconnect(i);
    }
}

void node_client_fun(int i) {
    if (i < MAX_CLIENTS) {
        client_fun(i);
    } else {
        node_fun(i);
    }
}

void poll_iterate() {
    for (int i = 0; i < MAX_FD_COUNT; i++) {
        if (server.fds[i].revents & POLLIN) {
            switch (i) {
                case MAX_FD_COUNT - 1:
                    listener_fun(i);
                default:
                    node_client_fun(i);
            }
        }
    }
}

void server_fun() {
    server.fds[0].fd = server.listener;
    server.fds[0].events = POLLIN;

    while (true) {
        CH(poll(server.fds, MAX_FD_COUNT, TIMEOUT * 1000));
        poll_iterate();        
    }    
}

void sockaddr_init(struct sockaddr_in* ip_of_server) {
    memset(ip_of_server, NULL, sizeof(*ip_of_server));
    
    ip_of_server -> sin_family = AF_INET;
    ip_of_server -> sin_addr.s_addr = htonl(INADDR_ANY);
    ip_of_server -> sin_port = htons(server.port_clients);
}

void server_clients_init() {
    memset(&server.clients, -1, MAX_CLIENTS * sizeof(int));
    memset(&server.clients_translator, -1, MAX_CLIENTS * sizeof(int));
}

void server_init() {
    struct sockaddr_in ip_of_server;
    sockaddr_init(&ip_of_server);
    server.listener = socket(AF_INET, SOCK_STREAM, NULL);
    CH(bind(server.listener, (struct sockaddr*) &ip_of_server, sizeof(ip_of_server)));
    CH(listen(server.listener, MAX_QUEUE));
    server_clients_init();
}

int main(int argc, char* argv[]) {
    get_argv(argc, argv);
    server_init();
    server_fun();
}
