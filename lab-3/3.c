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
    char** strings;
    int count_of_strings;
} t_sequence;

void* printer(void* param){
    t_sequence* sequence = (t_sequence*)param;

    for(int i = 0; i < sequence->count_of_strings; i++)
        printf("Thread id: %ld ||| Sequence: %s\n", syscall(SYS_gettid), sequence->strings[i]);
        
    return 0;
}

void pthreadFailure(int code, char problem[], char* argv[]){
    if(code){
        char buf[256];
        strerror_r(code, buf, sizeof buf);
        fprintf(stderr, "%s: %s thread: %s\n", argv[0], problem, buf);
        exit(EXIT_FAILURE);
    }
}

void pthreadBlock(t_sequence sequence[], char* argv[]){
    pthread_t thread;
    int code;

    for(int i = 0; i < DEFAULT_COUNT_OF_THREADS; i++){    
        code = pthread_create(&thread, NULL, printer, sequence + i);
        pthreadFailure(code, "creating", argv);            
    }
}

int main(int argc, char *argv[]){    
    t_sequence sequence[DEFAULT_COUNT_OF_THREADS];

    for(int i = 0; i < DEFAULT_COUNT_OF_THREADS; i++)
        sequence[i].count_of_strings = DEFAULT_COUNT_OF_STRINGS;

    char* first[]  = {"1",   "2",  "3"};
    char* second[] = {"4",   "5",  "6"};
    char* third[]  = {"7",   "8",  "9"};
    char* fourth[] = {"10", "11", "12"};

    sequence[0].strings = first;
    sequence[1].strings = second;
    sequence[2].strings = third;
    sequence[3].strings = fourth;
    
    pthreadBlock(sequence, argv);
    pthread_exit(NULL);
    exit(EXIT_SUCCESS);
}
