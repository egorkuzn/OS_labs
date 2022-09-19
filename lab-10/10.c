//Yegor Kuznetsov, 2022, NSU
//Dining philosophers. The problem of "deadlocks" fixing.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <locale.h>

#define PHILO 5
#define DELAY 30000
#define FOOD 50
#define PROGRAM_NAME "10"

pthread_t phils[PHILO];
pthread_mutex_t forks[PHILO];
pthread_mutex_t foodlock;
pthread_mutex_t getting_forks_mx;
pthread_cond_t getting_forks_cond;

void pthreadFailureCheck(const int code,
                         const char problem[],
                         const char programName[]){
    if(code){
        fprintf(stderr, "%s: %s thread: %s\n", \
                programName, problem, strerror_l(code, LC_CTYPE));
        exit(EXIT_FAILURE);
    }
}

int food_on_table(){
    static int food = FOOD;
    int myfood;

    pthread_mutex_lock(&foodlock);

    if(food > 0)
        food--;
    
    myfood = food;
    pthread_mutex_unlock(&foodlock);
    return myfood;
}

void get_forks(int right_fork, int left_fork, int phil){
    int res = pthread_mutex_lock(&getting_forks_mx);

    do{
        res = pthread_mutex_trylock(&forks[right_fork]);

        if(!res){ // if right fork is taken by phil
            res = pthread_mutex_trylock(&forks[left_fork]);

            if(res) // if couldn't take left put aside right
                pthread_mutex_unlock(&forks[right_fork]);
            else 
                printf("Philosopher %d: got left fork %d\n", phil, left_fork);
        } else
            printf("Philosopher %d: got right fork %d\n", phil, right_fork);

        if(res)
            pthread_cond_wait(&getting_forks_cond, &getting_forks_mx);
    } while(res);    

    pthread_mutex_unlock(&getting_forks_mx);
}

void down_forks(int f1, int f2){
    pthread_mutex_lock(&getting_forks_mx);
    pthread_mutex_unlock(&forks[f1]);
    pthread_mutex_unlock(&forks[f2]);
    pthread_cond_broadcast(&getting_forks_cond);
    pthread_mutex_unlock(&getting_forks_mx);
}

void* philosopher (void* num){
    int id;
    int left_fork, right_fork, f;

    id = (int)num;
    printf("Philosopher %d sitting down to dinner.\n", id);
    right_fork = id;
    left_fork  = id + 1;
 
    if(left_fork == PHILO)
        left_fork = 0;
 
    while((f = food_on_table()) > 0){
        printf("Philosopher %d: get dish %d.\n", id, f);
        get_forks(right_fork, left_fork, id);
        printf("Philosopher %d: eating.\n", id);
        usleep(DELAY * (FOOD - f + 1));
        down_forks(left_fork, right_fork);
    }

    printf("Philosopher %d is done eating.\n", id);
    return(NULL);
}

int main(int argn, char **argv){
    pthread_mutex_init(&foodlock, NULL);
    pthread_cond_init(&getting_forks_cond, NULL);

    for(int i = 0; i < PHILO; i++)
        pthread_mutex_init(&forks[i], NULL);

    for(int i = 0; i < PHILO; i++)
        pthread_create(&phils[i], NULL, philosopher, (void*)i);

    for(int i = 0; i < PHILO; i++)
        pthread_join(phils[i], NULL);

    pthread_cond_destroy(&getting_forks_cond);
    pthread_exit(NULL);
    exit(EXIT_SUCCESS);
}
