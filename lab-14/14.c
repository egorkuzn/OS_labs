//Yegor Kuznetsov, 2022, NSU
//One thread creation. Default attributes.
//Parent and child printing 10 strings.
//Syncronized modification with semaphores.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <locale.h>
#include <semaphore.h>

#define pthread_check(a) pthreadFailureCheck(__LINE__, a, __FUNCTION__, __FILE__)
#define COUNT_OF_STRINGS 5

sem_t printerSemArray[2];

typedef enum enum_actor{
    PARENT,
    CHILD
} enum_actor;

const char* enumActor2Str[2] = {"Parent", "Child"};

void pthreadFailureCheck(const int line,\
                         const int code,\
                         const char function[],\
                         const char programName[]){
    if(code){
        fprintf(stderr, "%s::%s()::%d pthread function: %s\n",\
         programName, function, line, strerror_l(code, LC_CTYPE));
        exit(EXIT_FAILURE);
    }
}

enum_actor otherName(enum_actor name){
    return (enum_actor)((name + 1) % 2);
}

void syncPrint(enum_actor name, const int i){
    sem_wait(&printerSemArray[name]);
    printf("%s\n", enumActor2Str[name]);
    sem_post(&printerSemArray[otherName(name)]);
}


void printer(enum_actor name){
    for(int i = 0; i < COUNT_OF_STRINGS; i++)
        syncPrint(name, i);    
}

void* childPrint(void* param){
    printer(CHILD);
}

void parentPrint(){
    printer(PARENT);
} 

void pthreadCreationFailure(const int code, const char programName[]){
    char buffer[256];
    strerror_r(code, buffer, sizeof buffer);
    fprintf(stderr, "%s: creating thread: %s\n", programName, buffer);
}

void printerSemInit(){
    for(u_char i = 0; i < 2; i++) 
        pthread_check(sem_init(&printerSemArray[i], 0, 1 - i));
}

void printerSemDestroy(){
    for(u_char i = 0; i < 2; i++)
        pthread_check(sem_destroy(&printerSemArray[i]));
}

int main(int argc, char *argv[]){
    pthread_t newThread;
    // Semaphores initialization:
    printerSemInit();
    // Child thread creation:
    pthread_check(pthread_create(&newThread, NULL, childPrint, NULL));   
    // Parent thread work: 
    parentPrint();
    pthread_exit(NULL);
    // Semaphore destroy:
    printerSemDestroy();
    exit(EXIT_SUCCESS);
}