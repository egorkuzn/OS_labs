//Yegor Kuznetsov, 2022, NSU
//4 threads that makes the same one function.
//The function prints strings that are parameters.
//Different threads prints different strings.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <syscall.h>

#define DEFAULT_COUNT_OF_THREADS 4
#define DEFAULT_COUNT_OF_STRINGS 3

typedef struct{
    char strings[DEFAULT_COUNT_OF_STRINGS][BUFSIZ];
    int countOfStrings;
} sequence_t;

void* printer(void* param){
    sequence_t* sequence = (sequence_t*)param;

    for(int i = 0; i < sequence -> countOfStrings; i++)
        printf("Thread id: %ld ||| Sequence: %s\n", syscall(SYS_gettid), sequence -> strings[i]);
        
    return 0;
}

void pthreadFailureCheck(const int code, const char problem[], const char programName[]){
    if(code){
        char buffer[256];
        strerror_r(code, buffer, sizeof buffer);
        fprintf(stderr, "%s: %s thread: %s\n", programName, problem, buffer);
        exit(EXIT_FAILURE);
    }
}

void threadsExecution(const sequence_t sequence[], const char programName[]){
    pthread_t myThread;
    int code;

    for(int i = 0; i < DEFAULT_COUNT_OF_THREADS; i++){    
        code = pthread_create(&myThread, NULL, printer, (void*)(sequence + i));
        pthreadFailureCheck(code, "creating", programName);            
    }
}
//Adding +1 to avoid starting from 0
int getNextCount(const int i, const int j){
    return i * DEFAULT_COUNT_OF_STRINGS + j + 1;
}

void initSequence(sequence_t sequence[DEFAULT_COUNT_OF_THREADS]){
    for(int i = 0; i < DEFAULT_COUNT_OF_THREADS; i++){
        sequence[i].countOfStrings = DEFAULT_COUNT_OF_STRINGS;

        for(int j = 0; j < DEFAULT_COUNT_OF_STRINGS; j++)
            sprintf(sequence[i].strings[j], "%d", getNextCount(i, j));
    }
}

int main(int argc, char *argv[]){    
    sequence_t sequence[DEFAULT_COUNT_OF_THREADS];
    initSequence(sequence);  
    threadsExecution(sequence, argv[0]);
    pthread_exit(NULL);
    exit(EXIT_SUCCESS);
}
