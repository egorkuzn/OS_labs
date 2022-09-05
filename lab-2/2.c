//Yegor Kuznetsov, 2022, NSU
//One thread creation. Default attributes.
//Parent and child printing 10 strings.
//Child should wait it's parent.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

void printer(const char name[]){
    for(int i = 0; i < 10; i++)
        printf("%s: %d\n", name, i + 1);
}

void* childPrint(){
    printer("Child");
    return (void*)true;
}

void parentPrint(){
    printer("Parent");
} 

void pthreadFailureCheck(const int code, const char problem[], const char programName[]){
    if(code){
        char bufffer[256];
        strerror_r(code, bufffer, sizeof bufffer);
        fprintf(stderr, "%s: %s thread: %s\n", programName, problem, bufffer);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]){
    pthread_t newThread;
    int code;
    code = pthread_create(&newThread, NULL, childPrint, NULL);
    pthreadFailureCheck(code, "creating", argv[0]);

//Optional part: taking return value.
    bool message = false;
    code = pthread_join(newThread, (void**)&message);
    pthreadFailureCheck(code, "joining", argv[0]);

    if(message){
        printf("Joined!\n");
    }

    parentPrint();
    pthread_exit(NULL);
    exit(EXIT_SUCCESS);
}
