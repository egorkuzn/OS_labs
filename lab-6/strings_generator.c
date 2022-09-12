//Yegor Kuznetsov, 2022, NSU
//This program generates 100 random strings and write in file.
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_COUNT 100
#define DEFAULT_FILENAME "string_collection.txt"

int randomSize(int* size){
    *size = rand() % DEFAULT_COUNT;
    return *size;
}

char randChar(){
    return '/' + rand() % (16 * 4);
}

void initBuffer(char* buffer, int size){
    for(int i = 0; i < size; i++)
        buffer[i] = randChar();
}

int main(){
    FILE* out = fopen(DEFAULT_FILENAME, "w");

    if(!out){
        printf("Didn't opened");
        return 1;
    }

    for(int i = 0; i < DEFAULT_COUNT; i++){
        int size = 0;
        char* buffer = (char*)calloc(randomSize(&size), sizeof(char));
        initBuffer(buffer, size);
        fprintf(out, "%s\n", buffer);
    }

    fclose(out);
    return 0;
}