// Задача 31(прокси)

// Реализуйте простой кэширующий HTTP-proxy с кэшем в оперативной памяти.

// Прокси должен быть реализован как один процесс и один поток, использующий
// для одновременной работы с несколькими сетевыми соединениями системный
// вызов (select или) poll. Прокси должен обеспечивать одновременную работу
// нескольких клиентов (один клиент не должен ждать завершения запроса или
// этапа обработки запроса другого клиента).

// 1. HTTP 1.0
// 2. GET, POST, PUT
// 3. [200 OK] - кэшировать
// и т.д.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <locale.h>
#include <string.h>

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

int main(int argc, char* argv[]) {
    
    pthread_exit(NULL);

    exit(EXIT_SUCCESS);
}
