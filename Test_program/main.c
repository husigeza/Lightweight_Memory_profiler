#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>



pthread_t tid[3];

void* Thread(void *arg)
{
    int *pointer;

    while(1) {
        printf("Thread %d \n",(int*)arg);
        pointer = (int *)malloc(sizeof(int));
        free(pointer);
        sleep(1);
    }

    return NULL;
}


int main()
{



    pthread_create(&(tid[0]), NULL, &Thread, (int*)1);
    printf("\n Created Thread 1\n");

    pthread_create(&(tid[1]), NULL, &Thread, (int*)2);
    printf("\n Created Thread 2\n");

    /*result_malloc = (int *)malloc(sizeof(int));
    free(result_malloc);
    usleep(20);
    printf("Set enable to 1\n");
    enable = 1;*/


   /* usleep(20);
    result_malloc = (int *)malloc(sizeof(int));
    free(result_malloc);
    printf("Set enable to 0\n");
    enable = 0;*/


//enable = 1;
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



