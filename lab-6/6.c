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

void initStringCollection(char string_collection[DEFAULT_COUNT][BUFSIZ]){
    FILE* in = fopen(DEFAULT_FILENAME, "r");
    fileFailureCheck(in, DEFAULT_FILENAME);

    for(int i = 0; i < DEFAULT_COUNT; i++)
        fscanf(in, "%s", string_collection[i]);
}

void sleepSort(const char stringsCollection[DEFAULT_COUNT][BUFSIZ], const char programName[]){
    int code;
    pthread_t threadPool[DEFAULT_COUNT];

    for(int i = 0; i < DEFAULT_COUNT; i++){
        code = pthread_create(&threadPool[i], NULL, sortFunc, (void*)(stringsCollection + i));
        pthreadFailureCheck(code, "creating", programName);
    }
}

int main(int argc, char* argv[]){
    char stringsCollection[DEFAULT_COUNT][BUFSIZ];
    initStringCollection(stringsCollection);
    sleepSort(stringsCollection, argv[0]);
    pthread_exit(NULL);
    exit(EXIT_SUCCESS);
}
