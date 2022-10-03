//Yegor Kuznetsov, 2022, NSU
//One thread creation. Default attributes.
//Parent and child printing 10 strings.
//Syncronized modification.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <locale.h>

#define pthread_check(a) pthreadFailureCheck(__LINE__, a, __FUNCTION__, __FILE__)
#define COUNT_OF_STRINGS 100000

char previousActor;
pthread_mutex_t terminal_printing_mx;
pthread_cond_t terminal_printing_cond;

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

void stopWaiting(){
    pthread_mutex_lock(&terminal_printing_mx);
    pthread_cond_broadcast(&terminal_printing_cond);
    pthread_mutex_unlock(&terminal_printing_mx);
}

void syncPrint(const char* name, const int i){
    pthread_mutex_lock(&terminal_printing_mx);

    int count = 0;

    do{
        if(name[0] != previousActor){
            printf("|%s\t|%d\t|%10d|\n", name, i + 1, count);
            previousActor = name[0];
            break;
        } else
            pthread_cond_wait(&terminal_printing_cond, &terminal_printing_mx);
        
        count++;
    } while (true);   

    pthread_mutex_unlock(&terminal_printing_mx); 
}


void printer(const char* name){
    for(int i = 0; i < COUNT_OF_STRINGS; i++){
        //It stops other thread delay: 
        stopWaiting();
        syncPrint(name, i);
    }
    // F.e.: one thread finished it's work, however other in block:
    stopWaiting();
}

void* childPrint(void* param){
    printer("Child");
}

void parentPrint(){
    printer("Parent");
} 

void pthreadCreationFailure(const int code, const char programName[]){
    char buffer[256];
    strerror_r(code, buffer, sizeof buffer);
    fprintf(stderr, "%s: creating thread: %s\n", programName, buffer);
}

int main(int argc, char *argv[]){
    printf("|WHO\t|TIME\t|ITERATIONS|\n");
    pthread_t newThread;
    pthread_mutexattr_t terminal_attr;
    // Mutex initialization:
    pthread_check(pthread_cond_init(&terminal_printing_cond, NULL));
    pthread_check(pthread_mutexattr_init(&terminal_attr));
    pthread_check(pthread_mutexattr_settype(&terminal_attr, PTHREAD_MUTEX_ERRORCHECK));
    pthread_check(pthread_mutex_init(&terminal_printing_mx, &terminal_attr));
    // Child thread creation:
    pthread_check(pthread_create(&newThread, NULL, childPrint, NULL));   
    // Parent thread work: 
    parentPrint();
    pthread_exit(NULL);
    // Mutex destroy after 2 threads finished their works:
    pthread_mutex_destroy(&terminal_printing_mx);
    pthread_check(pthread_cond_destroy(&terminal_printing_cond));

    exit(EXIT_SUCCESS);
}
