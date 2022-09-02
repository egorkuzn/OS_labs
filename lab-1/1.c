//Yegor Kuznetsov, 2022, NSU
//One thread creation. Default attributes.
// Parent and child printing 10 strings.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void printer(char* name){
    for(int i = 0; i < 10; i++)
        printf("%s: %d\n", name, i + 1);
}

void* childPrint(){
    printer("Child");
}

void parentPrint(){
    printer("Parent");
} 

void pthreadCreationFailure(int code, char* argv[]){
    char buf[256];
    strerror_r(code, buf, sizeof buf);
    fprintf(stderr, "%s: creating thread: %s\n", argv[0], buf);
}

int main(int argc, char *argv[]){
    pthread_t thread;
    int code;
    code = pthread_create(&thread, NULL, childPrint, NULL);

    if (code != 0){
        pthreadCreationFailure(code, argv);
        return 1;
    }
    
    parentPrint();
    pthread_exit(NULL);
    return 0;
}
