//Yegor Kuznetsov, 2022, NSU
//Child prints text while parent don't stop.
//Parent stop after 2 seconds.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define DEFAULT_FILE_NAME "index.html"

void* printer(void* param){
    FILE* in;
    in = fopen(DEFAULT_FILE_NAME, "r");
    char buff[BUFSIZ];

    while(fscanf(in, "%s", buff))       
        printf("%s ", buff);

    fclose(in);
}

void pthreadFailure(int code, char problem[], char* argv[]){
    if(code){
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: %s thread: %s\n", argv[0], problem, buf);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]){   
    pthread_t thread;     
    int code;

    code = pthread_create(&thread, NULL, printer, NULL);
    pthreadFailure(code, "creating", argv);

    sleep(2);

    code = pthread_cancel(thread);
    pthreadFailure(code, "canceling", argv);

    printf("\n---\n");
    printf("FINISH\n");

    pthread_exit(NULL);
    return 0;
}
