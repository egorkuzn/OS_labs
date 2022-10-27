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
    *prevLeft = *left;
    *left = (*left) -> next;
    *right = (*left) -> next;
    *nextLeft = (*left) -> next; 

    return *nextLeft;
}


void sort(linkedList* head){
    if (head == NULL)
        return;

    pthread_mutex_lock(&(head->mutex));
    linkedList* left = head;
    linkedList* right = head->next;
    linkedList* nextLeft = left->next;
    linkedList* prevLeft = left;
    linkedList* prevRight = right;
    pthread_mutex_unlock(&(head->mutex));
    
    int flag = 0;

    do {
        
        while (right) {
            pthread_mutex_lock(&(left->mutex));
            pthread_mutex_lock(&(right->mutex));

            if (strncmp(left->str, right->str, MAX_SRING_SIZE) > 0) {
                swap(left, right);
                flag = 1;
            }

            prevRight = right;
            right = right->next;
            pthread_mutex_unlock(&(left->mutex));
            pthread_mutex_unlock(&(prevRight->mutex));
        }

   } while (flag && sortIterator(&flag, &prevLeft, &left, &right, &nextLeft));
}



void setSortFlag(int signo){
    sortFlag = 1;
    signal(SIGALRM, setSortFlag);
    alarm(1);
}

void* sorterFunc(void* param){
    while (true) {
        if (sortFlag) {
            sort(list);
            sortFlag = 0;
        }
    }

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
