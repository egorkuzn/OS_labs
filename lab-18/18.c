// Parent thread reads user input and adds to the list start.
// Strings, thats longer than 80 symbols cut to pieces. When
// empty string input - output list head. Child thread wakes 
// up every 5 sec and sorts list by using bubble sort. All
// operations with list should be syncronized by mutex.

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>    
#include <pthread.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>
#include <signal.h>
#include <stddef.h>

#define PCH(a) pthreadFailureCheck(__LINE__, a, __FUNCTION__, __FILE__)
#define MAX_SRING_SIZE 80
#define DEFAULT_TIME_WAIT 5

typedef struct linkedListM {
    char* str;
    pthread_mutex_t mutex;
    struct linkedListM* next;
} linkedListM;

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

linkedListM* addStringNode(linkedListM* head, char* string) {
    linkedListM* curHead = head;
    size_t strSize = strlen(string);

    if (strSize > MAX_SRING_SIZE)
        curHead = addStringNode(curHead, string + MAX_SRING_SIZE);

    linkedListM* newHead = (linkedListM*) malloc(sizeof(linkedListM));
    newHead -> str = (char*)calloc(MAX_SRING_SIZE + 1, sizeof(char));
    strncpy(newHead -> str, string, strSize % (MAX_SRING_SIZE + 1));
    newHead -> next = curHead;

    return newHead;
}

void printList(linkedListM* head) {
    linkedListM* cur = head;

    while (cur != NULL) {
        printf("%s", cur->str);
        cur = cur->next;
    }
}

void swap(linkedListM* left, linkedListM* right) {
    char* temp = right -> str;
    right -> str = left -> str;
    left -> str = temp;
}

void sort(linkedListM* head) {
    if (head == NULL)
        return;

    linkedListM* left = head;                 
    linkedListM* right = head -> next;          
    linkedListM* temp = (linkedListM*) malloc(sizeof(linkedListM));

    temp -> str = (char*) calloc(MAX_SRING_SIZE + 1, sizeof(char));       

    for (; left -> next; left = left -> next, right = left -> next)               
        for (; right; right = right -> next)
            if (strncmp(left->str, right->str, MAX_SRING_SIZE) > 0)
                swap(left, right);

    free(temp -> str);
    free(temp);
}


linkedListM* list;
int sortFlag = 0;

void* sortThreadFunc(void* param) {
    while (true) {
        if(sortFlag){
            sort(list);
            sortFlag = 0;
        }
    }    
}

void* alarmThreadFunc(void* param) {
    while (true) {
        sleep(DEFAULT_TIME_WAIT);
        pthread_mutex_lock(&sortFlagMutex);
        sortFlag = 1;
        pthread_mutex_unlock(&sortFlagMutex);
        pthread_cond_signal(&condVar);
    }
}

void parentThreadFunc(pthread_t* alarmThread, pthread_t* sortThread) {
    char userString[MAX_SRING_SIZE + 1] = {0};

    while (true) {
        fgets(userString, MAX_SRING_SIZE, stdin);
        pthread_mutex_lock(&listMutex);

        if (!strcmp("exit\n", userString)) {
            pthread_cancel(*sortThread);
            pthread_cancel(*alarmThread);
            break;
        }

        if (!strcmp("\n", userString)) {
            printf("--------------LIST PRINTING------------\n");
            printList(list);
            printf("---------------------------------------\n");
        } else
            list = addStringNode(list, userString);

        pthread_mutex_unlock(&listMutex);
    }
}

void setSortFlag(int signo) {
    sortFlag = 1;
    signal(SIGALRM, setSortFlag);
    alarm(1);
}

int main(int argc, char ** argv) {
    signal(SIGALRM, setSortFlag);

    pthread_t sortThread;
    pthread_t alarmThread;

    PCH(pthread_mutex_init(&listMutex, NULL));
    PCH(pthread_mutex_init(&sortFlagMutex, NULL));
    PCH(pthread_cond_init(&condVar, NULL));
    PCH(pthread_create(&sortThread, NULL, sortThreadFunc, NULL));
    PCH(pthread_create(&alarmThread, NULL, alarmThreadFunc, NULL));

    alarm(1);
    parentThreadFunc(&alarmThread, &sortThread);

    printf("-----------------FINISH----------------\n");
    pthread_exit(NULL);
    exit(EXIT_SUCCESS);
}
