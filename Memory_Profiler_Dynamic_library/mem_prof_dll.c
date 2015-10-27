#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/mman.h>


#include "mem_prof_dll.h"


#define fifo_path "/home/egezhus/mem_prof_fifo"
#define max_length 10000


extern void *__libc_malloc(size_t size);
extern void *__libc_free(void *);

static pthread_t hearthbeat_thread_id;
static bool enable;
static char PID_string[5];
static int shared_memory;

typedef struct malloc_struct_s {

    int len;
    int buf[max_length];
}malloc_struct_t;

static malloc_struct_t *malloc_struct;

void _init() {

        signal(SIGUSR1, signal_callback_handler);

        printf("current process: %d\n",getpid());

        sprintf(PID_string,"%d",getpid());
        shared_memory = shm_open(PID_string, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
        if(shared_memory < 0){

            printf("Error while creating shared memory:%d \n",errno);

        }

        int err = ftruncate(shared_memory,sizeof(malloc_struct_t));
        if(err < 0){

            printf("Error while truncating shared memory: %d \n",errno);

        }

        malloc_struct = mmap(NULL,sizeof(malloc_struct_t),PROT_WRITE,MAP_SHARED,shared_memory,0);
        if(malloc_struct == MAP_FAILED) {

            printf("Failed mapping the shared memory: %d \n",errno);

        }

        err = pthread_create(&hearthbeat_thread_id, NULL, &Hearthbeat, NULL);
        if(err){
            printf("Thread creation failed error:%d \n",err);
        }
        else {
            printf("Thread created\n");
        }


    }

void* malloc(size_t size) {
        //char *error;
        //void *caller = __builtin_return_address(0);

        if(profiling_allowed()) {
            printf("This is from my malloc!\n");

            if(malloc_struct->len != max_length){
                malloc_struct->buf[malloc_struct->len] = malloc_struct->len;
                malloc_struct->len++;
            }

        }

        //printf("Caller: %s\n",(char*)caller);

        return __libc_malloc(size);
    }

void free(void* pointer) {

        if(profiling_allowed()) {
            printf("This is from my free!\n");
        }
        __libc_free(pointer);
        return;
    }

void* Hearthbeat(void *arg) {

    int mem_prof_fifo;

        while(true){
            mem_prof_fifo = open(fifo_path, O_WRONLY);

            if(mem_prof_fifo != -1) {


                if(write(mem_prof_fifo,&PID_string, sizeof(PID_string)) != -1){

                    printf("Writing the FIFO was succesfull\n");

                }
                else {
                    printf("Failed writing the FIFO\n");
                }
            }
            else {
                printf("Failed opening the FIFO, errno: %d\n",errno);
            }
            close(mem_prof_fifo);
            sleep(1);
        }

}

bool profiling_allowed(void) {

    return enable;
}


void signal_callback_handler(int signum) {

    enable = ~enable;
}




