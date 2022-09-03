//Yegor Kuznetsov, 2022, NSU
//Make sort for 100 words. One thread for each word.
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define DEFAULT_FILENAME "string_collection.txt"
#define DEFAULT_COUNT 100

void* sortFunc(void* param){
    return NULL;
}

void initStringCollection(char* string_collection[]){
    FILE* in = fopen(DEFAULT_FILENAME, "r");
    
    if(!in)
        printf("File didn't opened");

    for(int i = 0; i < DEFAULT_COUNT; i++)
        fscsanf(in, &string_collection[BUFSIZ]);
}

int main(){
    pthread_t thread_pool[DEFAULT_COUNT];
    char* strings_collection[DEFAULT_COUNT];
    initStringCollection(strings_collection);

    for(int i = 0; i < DEFAULT_COUNT; i++){
        pthread_create(&thread_pool[i], NULL, sortFunc, strings_collection[i])
        
    }

    pthread_exit(NULL);
    exit(EXIT_SUCCESS);
}
