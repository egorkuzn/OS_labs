//Yegor Kuznetsov, 2022, NSU
//Child prints text while parent don't stop.
//Child should notify when finish running.
//Parent stop child after 2 seconds.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define DEFAULT_FILE_NAME "index.html"

void cleanupNotification(void* param){
    printf("\n---\n");
    printf("FINISH\n");
}

void fileFailureCheck(FILE* file, char filename[]){
    if(!file){
        char buf[256];
        fprintf(stderr, "Can't open \"%s\"\n", filename);
        exit(EXIT_FAILURE);
    }
}

void* printer(void* param){
    pthread_cleanup_push(cleanupNotification, NULL);
    FILE* in;
    in = fopen(DEFAULT_FILE_NAME, "r");
    fileFailureCheck(in, DEFAULT_FILE_NAME);
    char buff[BUFSIZ];    

    while(fscanf(in, "%s", buff))       
        printf("%s ", buff);

    fclose(in);
    pthread_cleanup_pop(1);    
    return NULL;
}

void pthreadFailureCheck(int code, char problem[], char* argv[]){
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
    pthreadFailureCheck(code, "creating", argv);

    sleep(2);

    code = pthread_cancel(thread);
    pthreadFailureCheck(code, "canceling", argv);
    pthread_exit(NULL);
    exit(EXIT_SUCCESS);
}
