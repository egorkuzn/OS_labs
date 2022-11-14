#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <stdbool.h>

#define CLIENTS_AMOUNT 510
#define BUFFER_SIZE 80
#define TIMEOUT 5
/* Node params: */
typedef struct {
    int countfd;
} config_t;

config_t node = {
    .countfd = 2
}; 

void nodeFunction() {
    struct pollfd fds[node.countfd];
    
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = STDOUT_FILENO;
    fds[1].events = POLLOUT;

    while (true) {
        if (!poll(fds, node.countfd, TIMEOUT * 1000)) {
            printf("TIMEOUT\n");
        } else {
            if (fds[0].revents & POLLIN) {
                char str[BUFFER_SIZE + 1];
                scanf("%s", str);
                printf("%s\n", str);
            } 
            
            // if (fds[1].revents & POLLOUT) {
            //     printf("Poll out is writeable\n");
            // }
        }
    }
    
}

int main(int argc, char *argv[]) {
    nodeFunction();
}
