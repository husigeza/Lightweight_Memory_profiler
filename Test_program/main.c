#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>
#include <execinfo.h>
#include <string.h>


pthread_t tid[3];

#define max_call_stack_depth 100

void dummy_malloc_1(int *pointer, int *i){

	*i += 1;

	pointer = (int *)malloc(sizeof(int));
	printf("dummy_malloc_1 pointer: 0x%lx \n",(unsigned long)pointer);
}

void dummy_malloc_2(int *pointer, int *i){

	*i += 1;

	pointer = (int *)malloc(sizeof(int));
	printf("dummy_malloc_2 pointer: 0x%lx \n",(unsigned long)pointer);
}

void* Thread_1(void *arg)
{
	int *pointer = NULL;
    int i;

    while(1) {
        printf("Thread %d \n",(int*)arg);
        //dummy_malloc_1(pointer,&i);
        pointer = (int*)malloc(sizeof(int));
        printf("pointer: 0x%lx \n",(unsigned long)pointer);
        free(pointer);
        sleep(1);
    }

    return NULL;
}


void* Thread_2(void *arg)
{
    int *pointer = NULL;
    int i;

    while(1) {
        printf("Thread %d \n",(int*)arg);
        pointer = (int*)malloc(sizeof(int));
        //printf("pointer after dummy_malloc_2: 0x%lx \n",(unsigned long)pointer);
        free(pointer);
        sleep(1);
    }


    return NULL;
}



int main()
{

    pthread_create(&(tid[0]), NULL, &Thread_1, (int*)1);
    printf("\n Created Thread 1\n");

   pthread_create(&(tid[1]), NULL, &Thread_2, (int*)2);
    printf("\n Created Thread 2\n");

    while(1){
    	sleep(1);
   }

    return 0;
}



