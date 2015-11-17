#include "Memory_profiler_shared_library.h"

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
#include <semaphore.h>
#include <execinfo.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>


#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <dlfcn.h>

#define fifo_path "/home/egezhus/mem_prof_fifo"
#define max_log_entry 1000
#define max_call_stack_depth 15

extern void *__libc_malloc(size_t size);
extern void *__libc_free(void *);

static pthread_t hearthbeat_thread_id;
static pthread_t memory_profiler_start_thread_id;

static bool enable = false;

static char PID_string[6];
static char PID_string_shared_mem[7];
static char PID_string_sem[16];

static int shared_memory;

static int trace_size = 0;

static int mem_prof_fifo;

sem_t enable_semaphore;
sem_t thread_semaphore;

static int semaphore_shared_memory;
static sem_t* memory_profiler_start_semaphore;



typedef struct memory_profiler_log_entry_s{
	pthread_t thread_id;
	int type; //malloc = 1, free = 2
	size_t size; //allocated bytes in case of malloc
	void *call_stack[max_call_stack_depth];
	uint64_t address;
	bool valid;
}memory_profiler_log_entry_t;


typedef struct memory_profiler_struct_s {
	int log_count;
	memory_profiler_log_entry_t log_entry[max_log_entry];
} memory_profiler_struct_t;

static memory_profiler_struct_t *memory_profiler_struct;


void
signal_callback_handler(int signum)
{

	sem_destroy(&thread_semaphore);
	sem_destroy(memory_profiler_start_semaphore);

	munmap(memory_profiler_struct, sizeof(memory_profiler_struct_t));
	munmap(memory_profiler_start_semaphore, sizeof(sem_t));

	shm_unlink(PID_string_sem);
	//shm_unlink(PID_string_shared_mem);
	printf("Caught signal %d\n",signum);

	// Terminate program
	exit(signum);
}


typedef struct Dl_info_s{
    const char *dli_fname;  /* Pathname of shared object that
                               contains address */
    void       *dli_fbase;  /* Address at which shared object
                               is loaded */
    const char *dli_sname;  /* Name of nearest symbol with address
                               lower than addr */
    void       *dli_saddr;  /* Exact address of symbol named
                               in dli_sname */
} Dl_info_t;

void __attribute__ ((constructor)) init() {



	printf("Init\n");

	signal(SIGINT, signal_callback_handler);

	sprintf(PID_string, "%d", getpid());

	printf("current process is: %s\n", PID_string);



	if(sem_init(&enable_semaphore,0,1) == -1) printf("Error in enable_semaphore init, errno: %d\n",errno);
	if(sem_init(&thread_semaphore,0,1) == -1) printf("Error in thread_semaphore init, errno: %d\n",errno);

	sprintf(PID_string_sem, "%d_start_sem", getpid());
	semaphore_shared_memory = shm_open(PID_string_sem, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
	if (semaphore_shared_memory < 0) printf("Error while creating semaphore shared memory:%d \n", errno);

	int err = ftruncate(semaphore_shared_memory, sizeof(sem_t));
	if (err < 0) printf("semaphore_shared_memory while truncating shared memory: %d \n", errno);

	memory_profiler_start_semaphore = (sem_t*)mmap(NULL, sizeof(sem_t), PROT_WRITE,MAP_SHARED, semaphore_shared_memory, 0);
	if (memory_profiler_start_semaphore == MAP_FAILED) printf("Failed mapping the shared memory: %d \n", errno);

	if(sem_init(memory_profiler_start_semaphore,1,0) == -1) printf("Error in memory_profiler_start_semaphore init, errno: %d\n",errno);

	err = pthread_create(&memory_profiler_start_thread_id, NULL, &Memory_profiler_start_thread, NULL);
	if (err) printf("Memory_profiler_start thread creation failed error:%d \n", err);

	err = pthread_create(&hearthbeat_thread_id, NULL, &Hearthbeat, NULL);
	if (err) printf("Hearthbeat thread creation failed error:%d \n", err);


}

void __attribute__ ((destructor)) finit(){

	printf("Closing shared lib\n");
	munmap(memory_profiler_struct, sizeof(memory_profiler_struct_t));
	sem_destroy(&thread_semaphore);
	shm_unlink(PID_string_sem);
	close(mem_prof_fifo);


}

void free(void* pointer) {

	if (profiling_allowed()) {
		printf("This is from my free!\n");
	}
	__libc_free(pointer);
	return;
}


void* malloc(size_t size) {

	Dl_info_t info;
	int i;

	if (profiling_allowed()) {

		set_profiling(false);

		printf("This is from my malloc!\n");

		void* pointer = __libc_malloc(size);

		//TODO: Make this faster for multiple thread, don't defend the whole structure
		sem_wait(&thread_semaphore);

		printf("log_count %d\n",memory_profiler_struct->log_count);

		trace_size = backtrace(memory_profiler_struct->log_entry[memory_profiler_struct->log_count].call_stack,max_call_stack_depth);

		for(i= 0; i< trace_size; i++){
		dladdr(memory_profiler_struct->log_entry[memory_profiler_struct->log_count].call_stack[i],&info);
		printf("Name: %s, address: %lx\n",info.dli_sname,(uint64_t)info.dli_saddr);
		}


		memory_profiler_struct->log_entry[memory_profiler_struct->log_count].thread_id = pthread_self();
		memory_profiler_struct->log_entry[memory_profiler_struct->log_count].type = 1;
		memory_profiler_struct->log_entry[memory_profiler_struct->log_count].size = size;
	    memory_profiler_struct->log_entry[memory_profiler_struct->log_count].address = (uint64_t*)pointer;
	    printf("address: %lu\n",(uint64_t*)pointer);
	    memory_profiler_struct->log_entry[memory_profiler_struct->log_count].valid = true;
	    memory_profiler_struct->log_count++;




		printf("Shared memory has been written\n");

		if(sem_post(&thread_semaphore) == -1){
			printf("Error in sem_post, errno: %d\n",errno);
		}

		set_profiling(true);

		return pointer;
	}

	return __libc_malloc(size);
}

void* Memory_profiler_start_thread(void *arg){


	while(true){

		sem_wait(memory_profiler_start_semaphore);

		if (profiling_allowed() == false) {
			printf("opening shared memory for profiling\n");

			sprintf(PID_string_shared_mem, "/%d", getpid());
			shared_memory = shm_open(PID_string_shared_mem, /*O_CREAT | */O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
			if (shared_memory < 0) printf("Error while opening shared memory:%d \n", errno);

		   printf("memory_profiler_struct_t size %lu \n",sizeof(memory_profiler_struct_t));
		   printf("log_entry size %lu \n",sizeof(memory_profiler_log_entry_t));

			memory_profiler_struct = (memory_profiler_struct_t*)mmap(NULL, sizeof(memory_profiler_struct_t), PROT_WRITE, MAP_SHARED , shared_memory, 0);
			if (memory_profiler_struct == MAP_FAILED) {
				printf("Failed mapping the shared memory: %d \n", errno);
			}

			set_profiling(true);

		} else {
			printf("closing shared memory for profiling\n");
			munmap(memory_profiler_struct, sizeof(memory_profiler_struct_t));
			set_profiling(false);
		}
	}
}

void* Hearthbeat(void *arg) {


	while (true) {

		mem_prof_fifo = open(fifo_path, O_WRONLY);

		if (mem_prof_fifo != -1) {

			if (write(mem_prof_fifo, &PID_string, sizeof(PID_string)) == -1) {

				printf("Failed writing the FIFO\n");
			}

			close(mem_prof_fifo);
		} else {
			printf("Failed opening the FIFO, errno: %d\n", errno);
		}


		sleep(1);
	}

}

bool profiling_allowed(void) {

	sem_wait(&enable_semaphore);
		bool enable_local = enable;
	sem_post(&enable_semaphore);

	return enable_local;
}

void set_profiling(bool value){

	sem_wait(&enable_semaphore);
		enable = value;
	sem_post(&enable_semaphore);
}

