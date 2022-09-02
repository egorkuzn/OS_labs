//Yegor Kuznetsov, 2022, NSU
//One thread creation. Default attributes.
//Parent and child printing 10 strings.
//Child should wait it's parent.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void printer(char name[]){
    for(int i = 0; i < 10; i++)
        printf("%s: %d\n", name, i + 1);
}

void* childPrint(){
    printer("Child");
}

void parentPrint(){
    printer("Parent");
} 

void pthreadFailure(int code, char problem[], char* argv[]){
    char buf[256];
    strerror_r(code, buf, sizeof buf);
    fprintf(stderr, "%s: %s thread: %s\n", argv[0], problem, buf);
}

int main(int argc, char *argv[]){
    pthread_t thread;
    int code;
    code = pthread_create(&thread, NULL, childPrint, NULL);

    if (code != 0) {
        pthreadFailure(code, "creating", argv);
        return 1;
    }
    
    code = pthread_join(thread, NULL);

    if(code != 0){
        pthreadFailure(code, "joining", argv);
        return 2;
    }

    parentPrint();
    pthread_exit(NULL);
    return 0;
}
