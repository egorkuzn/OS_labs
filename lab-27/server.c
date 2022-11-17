#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <assert.h>

#define MAX_FD_COUNT 1024 /* open files limit  */
#define MAX_CLIENTS  510  /* connections limit */
#define MAX_QUEUE    5    /* max queue process */
#define BUFFER_SIZE  80   /* max client_message size  */
#define TIMEOUT      1    /* in seconds        */
#define CH(a) check_fun(a, __FILE__, __FUNCTION__, __LINE__)

/* Structure for server params: */
typedef struct {
    int port_node;    /* where to translate       */
    int port_clients; /* where to listen          */
    char* ip_name;    /* address to translate     */
    int listener;     /* fd for clients listening */
    ssize_t read_bytes;                              /* read bytes real amount      */
    int clients[MAX_CLIENTS];                        /* clients sockets             */
    int clients_translator[MAX_CLIENTS];             /* clients translators on server */
    struct pollfd fds[MAX_FD_COUNT];                 /* fd array for poll           */
    char messages[MAX_CLIENTS * 2][BUFFER_SIZE + 1]; /* all client_message cashed         */
} server_t;

typedef enum{
    READ,
    WRITE
} client_mode_t;

server_t server;

void check_fun(int ret, const char program_name[], const char fun_name[], int line) {
    if (ret == -1) {
        fprintf(stderr, "%s:%s:%d exception\n", program_name, fun_name, line);
        exit(EXIT_FAILURE);
    }
}
/* Getting info from console input: */
void get_argv(int argc, char* argv[]) {
    if (argc == 4) {
        server.ip_name = argv[1];
        server.port_node = atoi(argv[2]);
        server.port_clients = atoi(argv[3]);
    } else {
        printf("Expected:\n");
        printf("* Node address name;\n");
        printf("* Port for node;\n");
        printf("* Port for clients.\n");
        printf("Don't worry, just try again.\n");
        exit(EXIT_FAILURE);
    }
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

void listener_fun() {
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
    int _ = i % MAX_CLIENTS;
    close(server.fds[_].fd);
    close(server.fds[_ + MAX_CLIENTS].fd);
    server.fds[_].fd = -1;
    server.fds[_ + MAX_CLIENTS].fd = -1;
    server.clients[_] = -1;
    server.clients_translator[_] = -1;
}

char* message_to_send(int i) {
    if (i >= MAX_CLIENTS) {
        return server.messages[i - MAX_CLIENTS];
    } else {
        return server.messages[i + MAX_CLIENTS];
    }
}

void client_fun_switch(int i, client_mode_t mode) {
    switch (mode) {
        case READ:
            memset(server.messages[i], NULL, BUFFER_SIZE);
            server.read_bytes = read(server.fds[i].fd, server.messages[i], BUFFER_SIZE);
            break;
        case WRITE:
            server.read_bytes = write(server.fds[i].fd, message_to_send(i), sizeof (message_to_send(i)));
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

    client_fun_switch(i, mode);

    if (server.read_bytes <= 0) {
        disconnect(i);
    } else {
        server.messages[i][server.read_bytes] = '\0';
    }
}

void poll_iterate() {
    for (int i = 0; i < MAX_FD_COUNT; i++) {
        if (server.fds[i].fd == -1) {
            continue;
        }

        if (server.fds[i].revents & POLLIN) {
            if (i == MAX_FD_COUNT - 1) {
                listener_fun();
            } else {
                client_fun(i, READ);
            }
        }

        if (server.fds[i].revents & POLLOUT) {
            client_fun(i, WRITE);
        }
    }
}

void init_poll() {
    for (int i = 0; i < MAX_FD_COUNT; i++) {
        server.fds[i].fd = -1;
    }
}

void server_fun() {
    init_poll();

    server.fds[MAX_FD_COUNT - 1].fd = server.listener;
    server.fds[MAX_FD_COUNT - 1].events = POLLIN;

    while (poll(server.fds, MAX_FD_COUNT, TIMEOUT * 1000) != -1) {
        poll_iterate();        
    }

    printf("Poll exception.\n");
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

void TEST() {
    assert(MAX_CLIENTS * 2 < MAX_FD_COUNT - 1);
}

int main(int argc, char* argv[]) {
    TEST();
    get_argv(argc, argv);
    server_init();
    server_fun();
}
