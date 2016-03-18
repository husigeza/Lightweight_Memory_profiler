#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>
#include <execinfo.h>
#include <string.h>
#include <stdbool.h>


pthread_t tid[3];

volatile static int *ptr1;
static int *ptr2;

static unsigned int size = 0;


void dummy_malloc_1(int *pointer, int *i){

	*i += 1;
	size++;

	printf("before ptr1: %lx\n",ptr1);
	//printf("new size: %d",size*sizeof(int));
	if(size < 6){
	ptr1 = (int *)realloc(ptr1,size*sizeof(int));
	}
	else if(size == 6){
		free(ptr1);
	}

}

void dummy_malloc_2(int *pointer, int *i){

	*i += 1;

	printf("ptr2: %lx\n",ptr2);
	ptr2 = (int *)realloc(ptr2,*i*sizeof(int));
	//free(pointer);
}

void* Thread_1(void *arg)
{
	volatile int *pointer = NULL;
    int i;

    while(1) {
       printf("Thread %d \n",(int*)arg);
        dummy_malloc_1(pointer,&i);
        //usleep(30);
        sleep(1);
    }

    return NULL;
}


void* Thread_2(void *arg)
{
    volatile int *pointer = NULL;
    int i;

    while(1) {
        printf("Thread %d \n",(int*)arg);
        dummy_malloc_2(pointer,&i);
       usleep(30);
        //sleep(1);
    }


    return NULL;
}



int main()
{

    pthread_create(&(tid[0]), NULL, &Thread_1, (int*)1);
    printf("\nCreated Thread 1\n");

   /*pthread_create(&(tid[1]), NULL, &Thread_2, (int*)2);
    printf("\nCreated Thread 2\n");*/

    //volatile int *pointer = realloc(NULL,sizeof(int));
    while(1){
    	sleep(1);

    	//realloc(pointer,sizeof(int));

    	//realloc(pointer,0);

    	//set_user_profiling_flag(true);
    	//func1();
    	//set_user_profiling_flag(false);
   }

    return 0;
}



