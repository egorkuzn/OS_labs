//Yegor Kuznetsov, 2022, NSU
//One thread creation. Default attributes.
// Parent and child printing 10 strings.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void printer(const char* name){
    for(int i = 0; i < 10; i++)
        printf("%s: %d\n", name, i + 1);
}

void* childPrint(){
    printer("Child");
}

void parentPrint(){
    printer("Parent");
} 

void pthreadCreationFailure(const int code, const char programName){
    char buffer[256];
    strerror_r(code, buffer, sizeof buffer);
    fprintf(stderr, "%s: creating thread: %s\n", programName, buffer);
}

int main(int argc, char *argv[]){
    pthread_t newThread;
    int code;
    code = pthread_create(&newThread, NULL, childPrint, NULL);

    if (code != 0){
        pthreadCreationFailure(code, argv[0]);
        return 1;
    }
    
    parentPrint();
    pthread_exit(NULL);
    return 0;
}
