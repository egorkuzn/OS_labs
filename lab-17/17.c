// Parent thread reads user input and adds to the list start.
// Strings, thats longer than 80 symbols cut to pieces. When
// empty string input - output list head. Child thread wakes 
// up every 5 sec and sorts list by using bubble sort. All
// operations with list should be syncronized by mutex.

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <stddef.h>
#include <pthread.h>
#include <stdbool.h>

#define pthread_check(a) pthreadFailureCheck(__LINE__, a, __FUNCTION__, __FILE__)
#define pthread_check_with_list(a, b) stop(a, b, __FUNCTION__, __LINE__)

typedef struct linkedList{
    struct linkedList* next;
    char* val;
} linkedList;

int returnFromThreads = 0;
pthread_t threadSorter;
pthread_mutex_t mutex;

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

void freeList(linkedList* head) {
    linkedList* next;

    for (linkedList* node = head; node; node = next) {
        next = node -> next;
        free(node);
    }
}

void stop(linkedList* head, int code, const char* function, const int line) {
    if(code) {
        pthread_check(pthread_mutex_destroy(&mutex));
        freeList(head);
        pthreadFailureCheck(line, code, function, __FILE__);
    }
}

void lockMutex(linkedList* head) {
    pthread_check_with_list(head, pthread_mutex_lock(&mutex));
}

void unlockMutex(linkedList* head) {
    pthread_check_with_list(head, pthread_mutex_unlock(&mutex));
}

size_t listSize(linkedList** head) {
    size_t size = 0;

    for (linkedList* node = (*head); node && (size < SIZE_MAX); node = node -> next, size++);

    return size;
}

linkedList* nodeValueSet(linkedList* node, const char* str){
    if (!strcpy(node -> val, str)) {
        free(node -> val);
        free(node);
        return NULL;
    }

    return node;
}

linkedList* createNode(const char* str) {
    linkedList* node = (linkedList*) malloc(sizeof(linkedList));

    if (!node) {
        free(node);
        printf("malloc\n");
        return NULL;
    }

    node -> next = NULL;
    node -> val  = (char*) malloc(strlen(str) + 1);

    return nodeValueSet(node, str);
}

void pushToList(linkedList** head, const char* sentence) {
    lockMutex(*head);

    linkedList *oldHead = (*head);
    linkedList *node = createNode(sentence);

    if (!node)
        return;

    *head = node;
    (*head) -> next = oldHead;

    unlockMutex(*head);
}
//Pointer swap - the most effective swap
void swap(linkedList* ptr1, linkedList* ptr2) {
    linkedList* tmp = ptr2 -> next;
    ptr2 -> next = ptr1;
    ptr1 -> next = tmp;
}

void bubbleSort(linkedList** head) {
    linkedList** prevNode;
    int swapped;
    size_t size = listSize(head);

    for (size_t i = 0; i <= size; i++) {
        prevNode = head;
        swapped = 0;

        for (size_t j = 0; j < size - i - 1; j++) {
            linkedList* nextNode = (*prevNode) -> next;

            if (strcmp((*prevNode) -> val, nextNode -> val) > 0) {
                swap(*prevNode, nextNode);
                swapped = 1;
            }

            prevNode = &(nextNode -> next);
        }

        if (!swapped)
            break;
    }
}

void initMutex() {
    pthread_check(pthread_mutex_init(&mutex, NULL));
}

void printList(linkedList** head) {
    lockMutex(*head);

    printf("==================LIST====================\n");

    for (linkedList* node = (*head); node; node = node -> next)
        printf("%s\n", node -> val);

    printf("=================END-LIST=================\n");

    unlockMutex(*head);
}

void sortStop(linkedList* head, int res) {
    returnFromThreads = 1;

    if (!res) {
        unlockMutex(head);
        pthread_cancel(threadSorter);
    }
}

void* threadHandler(void* data) {
    linkedList** head = (linkedList**) data;

    while (!returnFromThreads) {
        pthread_check_with_list(*head, sleep(5));

        lockMutex(*head);
        printf("sorting...\n");

        if (!(*head)) {
            printf("empty list, skip...\n");
            unlockMutex(*head);
            continue;
        }

        bubbleSort(head);
        printf("end of sorting...\n");
        unlockMutex(*head);
    }

    pthread_exit(data);
}

void createSorterThread(linkedList** head) {
    initMutex();
    pthread_check_with_list((*head), pthread_create(&threadSorter, NULL, threadHandler, (void *) head));
}

ssize_t promptLine(char* line, const int sizeLine) {
    ssize_t n = 0;

    while (true) {
        n += read(0, (line + n), (size_t) (sizeLine - n));
        *(line + n) = '\0';

        return n;
    }
}

void getStrings(linkedList** head) {
    char buf[81];
    ssize_t size;

    while (true) {
        size = promptLine(buf, 80);

        if (!strcmp(buf, "exit\n") || !size) {
            sortStop(*head, pthread_mutex_trylock(&mutex));
            return;
        } else if (!strcmp(buf, "\n"))
            printList(head);
        else {
            for (int i = 0; i < strlen(buf); i++)
                if (buf[i] == '\n')
                    buf[i] = '\0';

            pushToList(head, buf);
        }
    }
}

int main() {
    linkedList* head = NULL;
    createSorterThread(&head);
    getStrings(&head);
    pthread_check_with_list(head, pthread_join(threadSorter, NULL));
    pthread_check_with_list(head, pthread_mutex_destroy(&mutex));
    freeList(head);
    pthread_exit(NULL);
    exit(EXIT_SUCCESS);
}
