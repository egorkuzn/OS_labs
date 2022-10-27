#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <locale.h>

#define MAX_SRING_SIZE 80
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

pthread_mutex_t sortFlagMutex;
int sortFlag = 0;

typedef struct linkedList{
    char* str;
    pthread_mutex_t mutex;
    struct linkedList* next;
} linkedList;

linkedList* list;

linkedList* createNode(linkedList* curHead, const char* string){
    linkedList* node = (linkedList*)malloc(sizeof(linkedList));
    size_t strSize = strlen(string);
    PCH(pthread_mutex_init(&(node->mutex), NULL));
    node->str = (char*)calloc(MAX_SRING_SIZE + 1, sizeof(char));
    strncpy(node->str, string, strSize);
    node->next = curHead;
    return node;
}

void swap(linkedList* left, linkedList* right){
    char* temp = right -> str;
    right -> str = left -> str;
    left -> str = temp;
}

linkedList* addStringNode(linkedList* head, const char* string){
    linkedList* curHead = head;
    linkedList* newHead = createNode(curHead, string);
    return newHead;
}

void printList(linkedList* head){
    linkedList* cur = head;
    linkedList* prev;

    while (cur != NULL) {
        printf("%s", cur->str);
        prev = cur;
        cur = cur->next;
    }
}

linkedList* sortIterator(int* flag,
                  linkedList** prevLeft,
                  linkedList** left,
                  linkedList** right,
                  linkedList** nextLeft) {
    *flag = 0;
    *prevLeft =  *left;
    *left     = (*left) -> next;
    *right    = (*left) -> next;
    *nextLeft = (*left) -> next; 

    return *nextLeft;
}


int nodeLock(linkedList* node) {
    return pthread_mutex_lock(&(node -> mutex));
}

int nodeUnLock(linkedList* node) {
    return pthread_mutex_unlock(&(node -> mutex));
}

void nodePairLock(linkedList* left, linkedList* right) {
    nodeLock(left);
    nodeLock(right);
}

void nodePairUnLock(linkedList* left, linkedList* right) {
    nodeUnLock(left);
    nodeUnLock(right);
}

void listIterator(linkedList** prevRight, linkedList** right, linkedList** left) {
    *prevRight = *right;
    *right = (*right) -> next;
}

void sort(linkedList* head) {
    if (head == NULL)
        return;

    nodeLock(head);
    linkedList* left      = head;
    linkedList* right     = head;
    linkedList* nextLeft  = left -> next;
    linkedList* prevLeft  = left;
    linkedList* prevRight = right;
    nodeUnLock(head);
    
    int flag = 0;

    do {
        while (right) {
            nodePairLock(left, right);

            if (strncmp(left -> str, right -> str, MAX_SRING_SIZE) > 0) {
                swap(left, right);
                flag = 1;
            }

            listIterator(&prevRight, &right, &left);
            nodePairUnLock(left, right);
        }
    } while (flag && sortIterator(&flag, &prevLeft, &left, &right, &nextLeft));

    sortFlag = 0;
}



void setSortFlag(int signo){
    sortFlag = 1;
    signal(SIGALRM, setSortFlag);
    alarm(1);
}

void* sorterFunc(void* param){
    while (true)
        if (sortFlag)
            sort(list);
}

void parentThreadFunc(pthread_t* sortThread) {
    char userString[MAX_SRING_SIZE + 1] = {0};

    while (true) {
        fgets(userString, MAX_SRING_SIZE, stdin);

        if (!strcmp("\n", userString)) {
            printf("--------------LIST PRINTING------------\n");
            printList(list);
            printf("---------------------------------------\n");
        } else {
            list = addStringNode(list, userString);
        }
    }
}

int main(int argc, char ** argv){
    signal(SIGALRM, setSortFlag);
    pthread_t sortThread;
    PCH(pthread_create(&sortThread, NULL, sorterFunc, NULL));
    alarm(1);
    parentThreadFunc(&sortThread);
    pthread_exit(NULL);
    exit(EXIT_SUCCESS);
}
