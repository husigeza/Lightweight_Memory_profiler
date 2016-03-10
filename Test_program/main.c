#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>
#include <execinfo.h>
#include <string.h>


pthread_t tid[3];


void dummy_malloc_1(int *pointer, int *i){

	*i += 1;

	pointer = (int *)calloc(3,sizeof(int));

}

void dummy_malloc_2(int *pointer, int *i){

	*i += 1;

	pointer = (int *)malloc(sizeof(int));
	free(pointer);
}

void* Thread_1(void *arg)
{
	int *pointer = NULL;
    int i;

    while(1) {
       printf("Thread %d \n",(int*)arg);
        dummy_malloc_1(pointer,&i);
        //usleep(3000);
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
        dummy_malloc_2(pointer,&i);
        //usleep(3000);
        sleep(1);
    }


    return NULL;
}



int main()
{

    pthread_create(&(tid[0]), NULL, &Thread_1, (int*)1);
    printf("\nCreated Thread 1\n");

   pthread_create(&(tid[1]), NULL, &Thread_2, (int*)2);
    printf("\nCreated Thread 2\n");

    int *pointer = malloc(sizeof(int));
    while(1){
    	sleep(1);
    	pointer = realloc(pointer,sizeof(int));
    	func1();
   }

    return 0;
}



