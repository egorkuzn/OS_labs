//Yegor Kuznetsov, 2022, NSU
//One thread creation. Default attributes.
//Parent and child printing 10 strings.
//Syncronized modification with semaphores.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <semaphore.h>

#define PCH(a) pthreadFailureCheck(__LINE__, a, __FUNCTION__, __FILE__)
#define memory_check(a) memoryFailureCheck(__LINE__, a, __FUNCTION__, __FILE__)
#define COUNT_OF_STRINGS 5

sem_t* printerSemArray[2];

typedef enum enum_actor{
    PARENT,
    CHILD
} enum_actor;

const char* enumActor2Str[2] = {"Parent", "Child"};

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

void segfaultSigaction(int signal, siginfo_t *si, void *arg) {
    printf("Caught segfault at address %p :: mmap \n", si->si_addr);
    exit(0);
}

void classic(int signal, siginfo_t *si, void *arg) {
    printf("Sigmentation fault!\n");
    exit(0);
}

void setSa(struct sigaction* sa, void* fun) { 
    memset(sa, 0, sizeof(struct sigaction));
    sigemptyset(&(sa -> sa_mask));
    sa -> sa_sigaction = fun;
    sa -> sa_flags = SA_SIGINFO;

    sigaction(SIGSEGV, sa, NULL);
}

void memoryFailureCheck(const int line,\
                         const int code,\
                         const char function[],\
                         const char programName[]){
    if(code == -1){
        fprintf(stderr, "%s::%s()::%d mmap function: %s\n",\
         programName, function, line, strerror_l(code, LC_CTYPE));
        exit(EXIT_FAILURE);
    }
}

enum_actor otherName(enum_actor name){
    return (enum_actor)((name + 1) % 2);
}

void syncPrint(enum_actor name, const int i){
    sem_wait(printerSemArray[name]);
    printf("|%s\t|%4d|\n", enumActor2Str[name], i);
    sem_post(printerSemArray[otherName(name)]);
}


void printer(enum_actor name){
    for(int i = 0; i < COUNT_OF_STRINGS; ++i)
        syncPrint(name, i);    
}

void* childPrint(void* param){
    printer(CHILD);
    printf("+------------+\n");
}

void parentPrint(){
    printf("+------------+\n");
    printer(PARENT);
} 

void printerSemInit(struct sigaction* sa){
    //setSa(sa, segfaultSigaction);

    int fd;

    if ((fd = open("/dev/zero", O_RDWR)) < 0)
        fprintf(stderr, "Sem init failed\n");

    for(u_char i = 0; i < 2; i++){ 
        printerSemArray[i] = NULL;//(sem_t*)mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        printf("HERE I AM!!!");
        /*PCH(*/sem_init(printerSemArray[i], 1, 1 - i);//);
        printf("%s semaphore inited\n", enumActor2Str[i]);
    }

    close(fd);
}

void printerSemDestroy(int pid){
    int status;

    waitpid(pid, &status, 0);

    if(WIFEXITED(status)){
        printf("\nSemaphore destruction as soon as child finished.\n");

        PCH(sem_destroy(printerSemArray[0]));
        PCH(sem_destroy(printerSemArray[1]));

        memory_check(munmap(printerSemArray[0], sizeof(sem_t)));
        memory_check(munmap(printerSemArray[1], sizeof(sem_t)));
    }
}

int main(int argc, char *argv[]){
    int pid;
    struct sigaction sa;
    pthread_t newThread;
    // Semaphores initialization:
    printerSemInit(&sa);
    setSa(&sa, classic);
    
    if((pid = fork()) < 0){
        // New proc creation fail exception:
        fprintf(stderr, "fork() exception\n");
    } else if(pid == 0){
        // Child process work:
        childPrint(NULL);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process work: 
        parentPrint();
        printerSemDestroy(pid);
    }

    exit(EXIT_SUCCESS);
}
