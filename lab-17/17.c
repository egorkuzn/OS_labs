// Parent thread reads user input and adds to the list start.
// Strings, thats longer than 80 symbols cut to pieces. When
// empty string input - output list head. Child thread wakes 
// up every 5 sec and sorts list by using bubble sort. All
// operations with list should be syncronized by mutex.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <locale.h>
#include <fcntl.h>
#include <sys/mman.h>

#define pthread_check(a) pthreadFailureCheck(__LINE__, a, __FUNCTION__, __FILE__)
#define LENGTH 80

typedef struct linked_list {
    char buffer[LENGTH];
    linked_list* next;
} linked_list;

linked_list* mainList;
pthread_mutex_t mutex;

void initListToAdd(linked_list* listToAdd) {
    listToAdd -> next = NULL;
    read(STDIN_FILENO, listToAdd -> buffer, LENGTH);
}

linked_list* getSharedPointer() {
    int fd;

    if ((fd = open("/dev/zero", O_RDWR)) < 0)
        fprintf(stderr, "Linked list init failed\n");

    linked_list* sharedPointer = (linked_list*)mmap(NULL, sizeof(linked_list), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    close(fd);

    return sharedPointer;
}

void pthreadFailureCheck(const int line,\
                         const int code,\
                         const char function[],\
                         const char programName[]) {
    if(code){
        fprintf(stderr, "%s::%s()::%d pthread function: %s\n",\
         programName, function, line, strerror_l(code, LC_CTYPE));
        exit(EXIT_FAILURE);
    }
}

void terminalReader(char buffer[LENGTH]) {
    read(STDIN_FILENO, buffer, LENGTH);
}

bool isEmpty(linked_list* list) {
    return list -> next == NULL;
}

void insertToTheHead(linked_list** mainList, linked_list* listToAdd)  {
    listToAdd -> next = *mainList;
    *mainList = listToAdd;
}

void insertToTheMiddle(linked_list* left, linked_list* newNode, linked_list* right) {
    left -> next = newNode;
    newNode -> next = right;
}

bool compare(linked_list* left, linked_list* right) {
    strcmp(left -> buffer, right -> buffer);
}

// 1->[2]->3->[4]->5
// 1->[4]->3->[2]->5

void swap(linked_list* left, linked_list* right) {
    char tmp_buffer[LENGTH];

    memcpy(tmp_buffer     , right -> buffer, LENGTH);
    memcpy(right -> buffer, left -> buffer , LENGTH);
    memcpy(left -> buffer , tmp_buffer     , LENGTH);
}

void linkedListBubbleSort(linked_list* list) {
    for(linked_list* i = list; !i; i = i -> next)
        for(linked_list* j = i; !j; j = j -> next)
            if(compare(i, j))
                swap(i, j);
}

void* sorterThreadRoutine(void* param) {
    while (true){
        pthread_mutex_lock(&mutex);
        linkedListBubbleSort(mainList);
        pthread_mutex_lock(&mutex);
        sleep(5);
    }    
}

void readerFunction() {
    while (true) {
        linked_list* listToAdd = getSharedPointer();
        initListToAdd(listToAdd);

        pthread_mutex_lock(&mutex);

        if(!isEmpty(listToAdd))
            insertToTheHead(&mainList, listToAdd);

        pthread_mutex_unlock(&mutex);
    }    
}

int main(int argc, char* argv[]) {
    pthread_t sorterThread;
    pthread_check(pthread_create(&sorterThread, NULL, sorterThreadRoutine, NULL));
    readerFunction();
    pthread_exit(NULL);
    exit(EXIT_SUCCESS);
}
