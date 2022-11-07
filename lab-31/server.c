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
#include <locale.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>


#define CACHE_SIZE 3
#define COUNT_OF_USERS 10

#define PCH(a) pthreadFailureCheck(__LINE__, a, __FUNCTION__, __FILE__)
/* Posix thread check */
void pthreadFailureCheck(const int line,\
                         const int code,\
                         const char function[],\
                         const char programName[]);
/* Argc param check */
void ACH(int argc);
/* Threads count check */
void TCH(int countOfThreads);
/* Listener's check */
void FCH(int fd);
/* Cache for each page */
typedef struct cache_t {
    int pageSize;
    char* url;
    char* page;
    pthread_mutex_t mutex;
    long lastTime;
} cache_t;
/* Configuration which set by user */
typedef struct config_t {
    int serverPort;
    int countOfThreads
} config_t;
/* Global params */
int listenFd; // Listener's socket fd
pthread_mutex_t cacheIndexMutex; // Mutexes for each cache
pthread_mutex_t reallocMutex; // Mutex for pages realloc moment
cache_t* cache; // Cache which stores in RAM 
config_t config; // Servers configuration set by user
int* clients;
int* sentBytes;
int* cacheToClient;
int* clientsHttpSockets;
/* Sets listener close on exit */
void getSignal(int signalNumber) {
    close(listenFd);
    exit(EXIT_FAILURE);
}
/* Save all that we got from console */
void getAllFromArgc(char* argv[]) {    
    config.serverPort = atoi(argv[1]);
    config.countOfThreads = atoi(argv[2]);
    TCH(config.countOfThreads);
}
/* Mutexes initialiser */
void initMutexFunction() {
    pthread_mutex_init(&cacheIndexMutex, NULL);
    pthread_mutex_init(&reallocMutex, NULL);

    cache = (cache_t*)calloc(CACHE_SIZE, sizeof(cache_t));

    for (int i = 0; i < CACHE_SIZE; i++) {
        pthread_mutex_init(&(cache[i].mutex), NULL);
        cache[i].lastTime = time(NULL);
    }
}
/* Sets net connection params */
void setServAddr(struct sockaddr_in* servAddr) {
    memset(servAddr, NULL, sizeof(struct sockaddr_in)); // clean it
    /* set it fields: */
    servAddr -> sin_family = AF_INET; // IP protocol family
    servAddr -> sin_addr.s_addr = htonl(INADDR_ANY); // Set address to accept any incoming messages
    servAddr -> sin_port = htons(config.serverPort); // Set port
}
/* Handle creation with listenFd check*/
void creatingNewHandle() {
    struct sockaddr_in servAddr; 
    setServAddr(&servAddr);
    FCH(listenFd);
}
/* Listener initialiser */
void initListenFd() {
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    shutdown(listenFd, 2);
    creatingNewHandle();
}
/* Returns count of users that won't get thread */
int countWithoutThreads() {
    return COUNT_OF_USERS - config.countOfThreads;
}
/* Dynamic memory alloc block*/
void commonClientsEntitiesAlloc() {
    clients = (int*) calloc(countWithoutThreads(), sizeof(int));
    sentBytes = (int*) calloc(countWithoutThreads(), sizeof(int));
    cacheToClient = (int*) calloc(countWithoutThreads(), sizeof(int));
    clientsHttpSockets = (int*) calloc(countWithoutThreads(), sizeof(int));
}
/* Init clients, clientsHttpSockets, cachToClient and sentBytes*/
void clientsEntitiesInit() {
    commonClientsEntitiesAlloc();

    for (int i = 0; i < countWithoutThreads(); i++) {
        clients[i] = -1;
        send
        clientsHttpSockets[i] = -1;
        
    }
}
/* Common init */
void init (int argc, char* argv[]) {
    ACH(argc);
    signal(SIGINT, getSignal);
    getConfigAllFromArgc(argv);
    initMutexFunction();
    initListenFd();
    clientsEntitiesInit();
}

int main(int argc, char* argv[]) {    
    init(argc, argv);

    pthread_exit(NULL);
    exit(EXIT_SUCCESS);
}














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

void ACH(int argc) {
    if (argc != 3) {
        printf("Invalid number of arguments\n");
        exit(EXIT_FAILURE);
    }
}

void TCH(int countOfThreads) {
    if (countOfThreads > COUNT_OF_USERS) {
        printf("ERROR: Count of users must be less than %d\n", COUNT_OF_USERS);
        exit(EXIT_FAILURE);
    }
}

void FCH(int fd) {
    if (fd == -1)  {
        printf("ERROR: Problem while socket opening\n");
        exit(EXIT_FAILURE);
    }
}

