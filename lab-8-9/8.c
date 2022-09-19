//Yegor Kuznetsov, 2022, NSU
//Calculation of π, while user don't stop our program. 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <locale.h>
#include <stddef.h>
#include <stdatomic.h>

#define pthread_check(a) pthreadFailureCheck(__LINE__, a, __FUNCTION__, __FILE__)
#define DEFAULT_COUNT 8

static atomic_bool commandToStop = false;
atomic_int countOfFinishedThreads = 0;

typedef struct{
    int threadNum;
    int countOfThreads;
    int stepsNum;
}threadInfo_t;

void threadCalculationsInit(int* threadNum,
                            int* countOfThreads,
                            int* stepsNum,
                            double* result,
                            threadInfo_t* info){
    *threadNum = info->threadNum;
    *countOfThreads = info->countOfThreads;
    *stepsNum = info->stepsNum;
    *result = 0;
}

void myPthreadBarrier(int countOfThreads){
    countOfFinishedThreads++;

    if(countOfFinishedThreads > countOfThreads){
        fprintf(stderr, "Problem in myPthreadBarrier() code.\n");
        exit(EXIT_FAILURE);
    }


    while(countOfFinishedThreads != countOfThreads && !commandToStop)
        usleep(100);

    countOfFinishedThreads--;
}

void* threadCalculations(void* param){
    int threadNum, countOfThreads, stepsNum;
    double* result = (double*)calloc(1, sizeof(double));

    threadCalculationsInit(&threadNum,
                           &countOfThreads,
                           &stepsNum,
                           result,
                           (threadInfo_t*)param);

    for(u_long start = threadNum * stepsNum; \
                    !commandToStop && start < __LONG_LONG_MAX__; \
                                        start += countOfThreads * stepsNum){
        for (u_long i = start; i < start + stepsNum; i++)        
            *result += 1.0 / (i * 4.0 + 1.0)  -  1.0 / (i * 4.0 + 3.0);
    
        myPthreadBarrier(countOfThreads);
    }
    
    printf("%d\n", countOfFinishedThreads);
    pthread_exit(result);
}

void sigStop(int signum){
    commandToStop = true;
}

void setSignalHandler(int signal, void(*handler)(int)){
    struct sigaction action;

    action.sa_handler = handler;
    action.sa_flags = 0;

    if(signal == SIGCHLD)
        action.sa_flags |= SA_RESTART;

    sigemptyset(&action.sa_mask);
    sigaction(signal, &action, NULL);
}

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

double getPi(const int countOfThreads,
             const int numOfSteps,
             const char programName[]){
    int code;

    pthread_t threadPool[countOfThreads];
    threadInfo_t infoPool[countOfThreads];

    for(int i = 0; i < countOfThreads; i++){
        threadInfo_t newInfo = {i, countOfThreads, numOfSteps};
        infoPool[i] = newInfo;
        pthread_check(pthread_create(&threadPool[i], NULL, threadCalculations,\
                                                        (void*)(infoPool + i)));
    }

    double pi = 0;

    for(int i = 0; i < countOfThreads; i++){
        double* result;
        pthread_check(pthread_join(threadPool[i], (void**)&result));
        pi += *result;
        free(result);
    }  

    pi *= 4.0;
    return pi;
}

int main(int argc, char* argv[]){
    setSignalHandler(SIGINT, sigStop);
    setSignalHandler(SIGTERM, sigStop);
    printf("\nπ done - %.15g \n", getPi(DEFAULT_COUNT, 10000, argv[0])); 
    pthread_exit(NULL);
    exit(EXIT_SUCCESS);
}
