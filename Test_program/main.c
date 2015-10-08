#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <pthread.h>
#include "../Memory_Profiler_Static_library/mem_prof_static.h"


extern void *__libc_malloc(size_t size);
extern void  __libc_free(void *);

pthread_t tid[3];

void* Thread(void *arg)
{
    int *pointer;



    while(1) {
        printf("Thread %d \n",(int)arg);
        pointer = (int *)malloc(sizeof(int));
        printf("After malloc\n");
        free(pointer);
        sleep(1);
    }

    return NULL;
}


int main()
{
    char *error;
    int *result_malloc;
    int err;

    err = pthread_create(&(tid[0]), NULL, &Thread, (int*)1);
    printf("\n Created Thread 1\n");

    err = pthread_create(&(tid[1]), NULL, &Thread, (int*)2);
    printf("\n Created Thread 2\n");


    /*usleep(5);
    printf("Set enable to 1\n");
    enable = 1;

    usleep(5);
    printf("Set enable to 0\n");
    enable = 0;*/

    while(1);


    /*
    printf("Before first malloc\n");
    result_malloc = malloc(sizeof(int));
    free(result_malloc);
    printf("After first malloc\n");


    SampleAddInt(3,4);

    printf("Before second malloc\n");
    printf("Set enable to 1\n");
    enable = 1;
    result_malloc = malloc(sizeof(int));
    free(result_malloc);
    printf("After second malloc\n");



    printf("Before third malloc\n");
    printf("Set enable to 0\n");
    enable = 0;
    result_malloc = malloc(sizeof(int));
    free(result_malloc);
    printf("After third malloc\n");
    */

    return 0;
}



