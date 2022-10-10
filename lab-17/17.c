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

#define pthread_check(a) pthreadFailureCheck(__LINE__, a, __FUNCTION__, __FILE__)
#define LENGTH 80

typedef struct linked_list{
    char buffer[LENGTH];
    linked_list* next;
} linked_list;

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

void getListToAdd(linked_list* listToAdd){

}

bool isEmpty(linked_list* list){
    return list -> next == NULL;
}

void insertToTheHead(linked_list** mainList, linked_list* listToAdd){
    listToAdd -> next = *mainList;
    *mainList = listToAdd;
}

void insertToTheMiddle(linked_list* left, linked_list* newNode, linked_list* right){
    left -> next = newNode;
    newNode -> next = right;
}

bool compare(linked_list* left, linked_list* right){
    strcmp(left -> buffer, right -> buffer);
}

// 1->[2]->3->[4]->5
// 1->[4]->3->[2]->5

void swap(linked_list* left, linked_list* right){
    char tmp_buffer[LENGTH];

    memcpy(tmp_buffer     , right -> buffer, LENGTH);
    memcpy(right -> buffer, left -> buffer , LENGTH);
    memcpy(left -> buffer , tmp_buffer     , LENGTH);
}

void linkedListBubbleSort(linked_list* list){
    for(linked_list* i; !i; i = i -> next)
        for(linked_list* j = i; !j; j = j -> next)
            if(compare(i, j))
                swap(i, j);
}

void* sorterThreadRoutine(void* param){
    while (true){
        pthread_mutex_lock(&mutex);
        linkedListBubbleSort(list);
        pthread_mutex_lock(&mutex);
        sleep(5);
    }    
}

void readerFunction(){
    linked_list* mainList;
    linked_list* listToAdd;
    getListToAdd(listToAdd);
    pthread_mutex_lock(&mutex);

    if(!isEmpty(listToAdd))
        insertToTheHead(&mainList, listToAdd);

    pthread_mutex_unlock(&mutex);
}

int main(int argc, char* argv[]) {
    pthread_t sorterThread;
    pthread_check(pthread_create(&sorterThread, NULL, sorterThreadRoutine, NULL));

    exit(EXIT_SUCCESS);
}