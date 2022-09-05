//Yegor Kuznetsov, 2022, NSU
//Child prints text while parent don't stop.
//Parent stop after 2 seconds.

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

void fileFailureCheck(const FILE* file, const char filename[]){
    if(!file){
        fprintf(stderr, "Can't open \"%s\"\n", filename);
        exit(EXIT_FAILURE);
    }
}

void pthreadFailureCheck(const int code, const char problem[], const char programName[]){
    if(code){
        char buffer[256];
        strerror_r(code, buffer, sizeof buffer);
        fprintf(stderr, "%s: %s thread: %s\n", programName, problem, buffer);
        exit(EXIT_FAILURE);
    }
}

void* printer(void* param){
    FILE* in;
    in = fopen(DEFAULT_FILE_NAME, "r");
    fileFailureCheck(in, DEFAULT_FILE_NAME);
    char buffer[BUFSIZ];

    while(fscanf(in, "%s", buffer) != EOF)       
        printf("%s ", buffer);

    fclose(in);
}

int main(int argc, char *argv[]){   
    pthread_t newThread;     
    int code;
//Starting child running:
    code = pthread_create(&newThread, NULL, printer, NULL);
    pthreadFailureCheck(code, "creating", argv[0]);
//Waiting 2 seconds:
    sleep(2);
//Canceling child work running:
    code = pthread_cancel(newThread);
    pthreadFailureCheck(code, "canceling", argv[0]);
    cleanupNotification(NULL);

    pthread_exit(NULL);
    exit(EXIT_SUCCESS);
}
