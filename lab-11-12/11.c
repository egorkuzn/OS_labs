//Yegor Kuznetsov, 2022, NSU
//One thread creation. Default attributes.
//Parent and child printing 10 strings.
//Syncronized modification with using only mutexes.


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>

#define PCH(a) pthreadFailureCheck(__LINE__, a, __FUNCTION__, __FILE__)
#define COUNT_OF_ITERATIONS 5

pthread_mutexattr_t attr;
pthread_mutex_t m1;
pthread_mutex_t m2;
pthread_mutex_t m3;

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

void destroyMutex() {
    PCH(pthread_mutex_destroy(&m1));
    PCH(pthread_mutex_destroy(&m2));
    PCH(pthread_mutex_destroy(&m3));
}

void stop(char *errorMsg) {
    destroyMutex();
    perror(errorMsg);
    exit(EXIT_FAILURE);
}

void lockMutex(pthread_mutex_t *m) {
    if (pthread_mutex_lock(m))
        stop("Error lock mutex");
}

void unlockMutex(pthread_mutex_t *m) {
    if (pthread_mutex_unlock(m))
        stop("Error unlock mutex");
}

void initMutexes() {
    pthread_mutexattr_init(&attr);

    PCH(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK));

    if (pthread_mutex_init(&m1, &attr) ||
        pthread_mutex_init(&m2, &attr) ||
        pthread_mutex_init(&m3, &attr)) {
        destroyMutex();
        perror("Error creating mutex");
        exit(EXIT_FAILURE);
    }
}

void *childPrint(void* param) {
    // Really important mutex for child blocking:
    lockMutex(&m2);
    
    for (int i = 0; i < COUNT_OF_ITERATIONS; ++i) {
        lockMutex(&m1);
        printf("|Child\t|%i\t|\n", i);

        unlockMutex(&m2);
        lockMutex(&m3);
        unlockMutex(&m1);
        lockMutex(&m2);
        unlockMutex(&m3);
    }
    // In case of 79 string:
    unlockMutex(&m2);

    return NULL;
}

void parentPrint() {
    for (int i = 0; i < COUNT_OF_ITERATIONS; ++i) {
        printf("|Parent\t|%i\t|\n", i);

        lockMutex(&m3);
        unlockMutex(&m1);
        lockMutex(&m2);
        unlockMutex(&m3);
        lockMutex(&m1);
        unlockMutex(&m2);
    }
    //In case of 96 string:
    unlockMutex(&m1);
}

int main() {
    printf("|WHO\t|TIME\t|\n");

    pthread_t myThread;

    initMutexes();
    // Blocking for getting parent first:
    lockMutex(&m1);
    // Child thread creation:
    PCH(pthread_create(&myThread, NULL, childPrint, NULL)); 
    parentPrint();

    pthread_exit(NULL);    
    destroyMutex();
    return EXIT_SUCCESS;
}
