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

#define PCH(a) pthreadFailureCheck(__LINE__, a, __FUNCTION__, __FILE__)
#define MAX_SRING_SIZE 80
#define DEFAULT_TIME_WAIT 5

typedef struct linkedList {
    char* str;
    struct linkedList* next;
} linkedList;

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

linkedList* addStringNode(linkedList * head, char * string) {
    linkedList* curHead = head;
    size_t strSize = strlen(string);

    if (strSize > MAX_SRING_SIZE)
        curHead = addStringNode(curHead, string + MAX_SRING_SIZE);

    linkedList* newHead = (linkedList*) malloc(sizeof(linkedList));
    newHead -> str = (char*)calloc(MAX_SRING_SIZE + 1, sizeof(char));
    strncpy(newHead -> str, string, strSize % (MAX_SRING_SIZE + 1));
    newHead -> next = curHead;

    return newHead;
}

void printList(linkedList* head) {
    linkedList* cur = head;

    while (cur != NULL) {
        printf("%s", cur->str);
        cur = cur->next;
    }
}

void swap(linkedList* left, linkedList* right) {
    char* temp = right -> str;
    right -> str = left -> str;
    left -> str = temp;
}

void sort(linkedList* head) {
    if (head == NULL)
        return;

    linkedList * left = head;                 
    linkedList * right = head -> next;          
    linkedList * temp = (linkedList *)malloc(sizeof(linkedList));

    temp -> str = (char *)calloc(MAX_SRING_SIZE + 1, sizeof(char));       

    for (; left -> next; left = left -> next, right = left -> next)               
        for (; right; right = right -> next)
            if (strncmp(left->str, right->str, MAX_SRING_SIZE) > 0)
                swap(left, right);

    free(temp -> str);
    free(temp);
}


linkedList* list;
pthread_mutex_t listMutex;
pthread_mutex_t sortFlagMutex;
pthread_cond_t condVar;
int sortFlag = 0;

void* sortThreadFunc(void * param) {
    while (true) {
        pthread_mutex_lock(&sortFlagMutex);

        while (!sortFlag)
            pthread_cond_wait(&condVar, &sortFlagMutex);

        pthread_mutex_lock(&listMutex);
        sort(list);
        pthread_mutex_unlock(&listMutex);
        sortFlag = 0;
        pthread_mutex_unlock(&sortFlagMutex);
    }
    
}

void* alarmThreadFunc(void * param) {
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

int main(int argc, char ** argv) {
    pthread_t sortThread;
    pthread_t alarmThread;

    PCH(pthread_mutex_init(&listMutex, NULL));
    PCH(pthread_mutex_init(&sortFlagMutex, NULL));
    PCH(pthread_cond_init(&condVar, NULL));
    PCH(pthread_create(&sortThread, NULL, sortThreadFunc, NULL));
    PCH(pthread_create(&alarmThread, NULL, alarmThreadFunc, NULL));

    parentThreadFunc(&alarmThread, &sortThread);

    printf("-----------------FINISH----------------\n");
    pthread_exit(NULL);
    exit(EXIT_SUCCESS);
}
