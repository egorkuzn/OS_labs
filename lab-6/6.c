//Yegor Kuznetsov, 2022, NSU
//Make sort for 100 words. One thread for each word.
//Using sleep(2) and usleep(2)
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define DEFAULT_FILENAME "string_collection.txt"
#define DEFAULT_COUNT 100
#define COEFFICIENT 10000

void* sortFunc(void* param){
    char* string;
    string = (char*) param;
    usleep(strlen(string) * COEFFICIENT);
    printf("Length %lu: %s\n", strlen(string), string);
    return NULL;
}

void fileFailureCheck(FILE* file, char filename[]){
    if(!file){
        char buf[256];
        fprintf(stderr, "Can't open \"%s\"\n", filename);
        exit(EXIT_FAILURE);
    }
}

void pthreadFailureCheck(int code, char problem[], char* argv[]){
    if(code){
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: %s thread: %s\n", argv[0], problem, buf);
        exit(EXIT_FAILURE);
    }
}

void initStringCollection(char string_collection[DEFAULT_COUNT][BUFSIZ]){
    FILE* in = fopen(DEFAULT_FILENAME, "r");
    fileFailureCheck(in, DEFAULT_FILENAME);

    for(int i = 0; i < DEFAULT_COUNT; i++)
        fscanf(in, "%s", string_collection[i]);
}

int main(int argc, char* argv[]){
    pthread_t thread_pool[DEFAULT_COUNT];
    char strings_collection[DEFAULT_COUNT][BUFSIZ];
    int code;
    initStringCollection(strings_collection);

    for(int i = 0; i < DEFAULT_COUNT; i++){
        code = pthread_create(&thread_pool[i], NULL, sortFunc, &strings_collection[i]);
        pthreadFailureCheck(code, "creating", argv);
    }

    pthread_exit(NULL);
    exit(EXIT_SUCCESS);
}
