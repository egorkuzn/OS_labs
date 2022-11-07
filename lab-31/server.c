// Задача 31(прокси)

// Реализуйте простой кэширующий HTTP-proxy с кэшем в оперативной памяти.

// Прокси должен быть реализован как один процесс и один поток, использующий
// для одновременной работы с несколькими сетевыми соединениями системный
// вызов (select или) poll. Прокси должен обеспечивать одновременную работу
// нескольких клиентов (один клиент не должен ждать завершения запроса или
// этапа обработки запроса другого клиента).

// 1. HTTP 1.0
// 2. GET, POST, PUT
// 3. [200 OK] - кэшировать
// и т.д.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <locale.h>
#include <signal.h>
#include <string.h>
// Posix thread check
#define PCH(a) pthreadFailureCheck(__LINE__, a, __FUNCTION__, __FILE__)

void pthreadFailureCheck(const int line,\
                         const int code,\
                         const char function[],\
                         const char programName[]) {
    if (code) {
        fprintf(stderr, "%s::%s()::%d pthread function: %s\n",\
         programName, function, line, strerror_l(code, LC_CTYPE));
        exit(EXIT_FAILURE);
    }
}
// Argc param check
void ACH(int argc) {
    if (argc != 3) {
        printf("Invalid number of arguments");
        exit(EXIT_FAILURE);
    }
}
// Threads count check
void TCH() {
    
}

#define CACHE_SIZE 3

typedef struct cache_t {
    int pageSize;
    char* url;
    char* page;
    pthread_mutex_t mutex;
    long lastTime;
} cache_t;

int listenFd;
pthread_mutex_t cacheIndexMutex;
pthread_mutex_t reallocMutex;
cache_t* cache;

void getSignal(int signalNumber) {
    close(listenFd);
    exit(EXIT_FAILURE);
}

void initMutexFunction() {
    pthread_mutex_init(&cacheIndexMutex, NULL);
    pthread_mutex_init(&reallocMutex, NULL);

    cache = (cache_t*)calloc(CACHE_SIZE, sizeof(cache_t));

    for(int i = 0; i < CACHE_SIZE; i++) {
        pthread_mutex_init(&(cache[i].mutex), NULL);
        cache[i].lastTime = time(NULL);
    }
}

void init (int argc, char* argv[]) {
    ACH(argc);
    signal(SIGINT, getSignal);
    initMutexFunction();
    int serverPort = atoi(argv[1]);
    int countOfThreads = atoi(argv[2]);
    TCH(countOfThreads);
}

int main(int argc, char* argv[]) {    
    init();

    pthread_exit(NULL);
    exit(EXIT_SUCCESS);
}
