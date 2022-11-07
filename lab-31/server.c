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
#include <sys/socket.h>

#define CACHE_SIZE 3
#define COUNT_OF_USERS 10

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
        printf("Invalid number of arguments\n");
        exit(EXIT_FAILURE);
    }
}
// Threads count check
void TCH(int countOfThreads) {
    if (countOfThreads > COUNT_OF_USERS) {
        printf("ERROR: Count of users must be less than %d\n", COUNT_OF_USERS);
        exit(EXIT_FAILURE);
    }
}
// Cache for each page
typedef struct cache_t {
    int pageSize;
    char* url;
    char* page;
    pthread_mutex_t mutex;
    long lastTime;
} cache_t;
// Configuration which set by user
typedef struct config_t {
    int serverPort;
    int countOfThreads
} config_t;

int listenFd;
pthread_mutex_t cacheIndexMutex;
pthread_mutex_t reallocMutex;
cache_t* cache;
config_t config;

void getSignal(int signalNumber) {
    close(listenFd);
    exit(EXIT_FAILURE);
}

void getAllFromArgc(char* argv[]) {    
    config.serverPort = atoi(argv[1]);
    config.countOfThreads = atoi(argv[2]);
    TCH(config.countOfThreads);
}

void initMutexFunction() {
    pthread_mutex_init(&cacheIndexMutex, NULL);
    pthread_mutex_init(&reallocMutex, NULL);

    cache = (cache_t*)calloc(CACHE_SIZE, sizeof(cache_t));

    for (int i = 0; i < CACHE_SIZE; i++) {
        pthread_mutex_init(&(cache[i].mutex), NULL);
        cache[i].lastTime = time(NULL);
    }
}

void creatingNewHandle() {
    struct sockaddr_t servAddr;    
}

void initListenFd() {
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    shutdown(listenFd, 2);
    creatingNewHandle();
}

void init (int argc, char* argv[]) {
    ACH(argc);
    signal(SIGINT, getSignal);
    getConfigAllFromArgc(argv);
    initMutexFunction();
    initListenFd();
}

int main(int argc, char* argv[]) {    
    init(argc, argv);

    pthread_exit(NULL);
    exit(EXIT_SUCCESS);
}
