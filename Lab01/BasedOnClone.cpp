#include <iostream>
#include <cstdio>
#include <cstring>
#include <sched.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/wait.h>
using namespace std;

#define Test

int producer(void* args);
int consumer(void* args);

pthread_mutex_t mutex;
sem_t product;
sem_t warehouse;

char buffer[8][4];
int bp = 0;


int main(){
   

    pthread_mutex_init(&mutex, NULL);
    sem_init(&product,   0, 0);
    sem_init(&warehouse, 0, 8);

    const int clone_flag = CLONE_VM | CLONE_SIGHAND | CLONE_FS | CLONE_FILES | SIGCHLD;
    const int STACK_SIZE = 1024*1024;

    for(int i = 0; i < 2; i++){
        int *arg_p = (int*)malloc(sizeof(int));
        *arg_p = i;
        char *stack_p = (char*)malloc(STACK_SIZE);
        if(clone(producer, stack_p + STACK_SIZE, clone_flag, arg_p) < 0){
            perror("clone producer");
            exit(EXIT_FAILURE);
        }

        int *arg_c = (int*)malloc(sizeof(int));
        *arg_c = i;
        char *stack_c = (char*)malloc(STACK_SIZE);
        if(clone(consumer, stack_c + STACK_SIZE, clone_flag, arg_c) < 0){
            perror("clone consumer");
            exit(EXIT_FAILURE);
        }
    }

    // 等待四个子“线程”退出
    for(int j = 0; j < 4; j++){
        waitpid(-1, NULL, 0);
    }
    return 0;
}

int producer(void* args)
{
    int id = *((int*)args);
    for(int i = 0; i < 10; i++)
    {
        sleep(i+1);
        sem_wait(&warehouse);
        pthread_mutex_lock(&mutex);

        if(id == 0)
            strcpy(buffer[bp], "aaa");
        else
            strcpy(buffer[bp], "bbb");
        printf("producer%d produce %s in %d\n", id, buffer[bp], bp);
        bp++;

        pthread_mutex_unlock(&mutex);
        sem_post(&product);
    }
    printf("producer%d is over!\n", id);
    return 0;
}

int consumer(void *args)
{
    int id = *((int*)args);
    for(int i = 0; i < 10; i++)
    {
        sleep(10-i);
        sem_wait(&product);
        pthread_mutex_lock(&mutex);
        bp--;
        printf("consumer%d get %s in %d\n", id, buffer[bp], bp);
        strcpy(buffer[bp], "zzz");

        pthread_mutex_unlock(&mutex);
        sem_post(&warehouse);
    }
    printf("consumer%d is over!\n", id);
    return 0;
}
