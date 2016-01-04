#include "Memory_Profiler_shared_library.h"

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

//#define fifo_path "/home/egezhus/Memory_profiler/mem_prof_fifo"
#define fifo_path "/dev/mem_prof_fifo"

#define max_call_stack_depth 100

//#define START_PROF_IMM


extern void *__libc_malloc(size_t size);
extern void *__libc_free(void *);

static pthread_t hearthbeat_thread_id;
static pthread_t memory_profiler_start_thread_id;

// If another shared lib during init phase wants to call malloc/free before this shared lib has been initialized
// the execution stops at the semaphore reading, because they have not been initialized.
// init_done variable shows whether the semaphores have been initialized or not
static bool init_done = false;

static bool enable = false;

static char PID_string[6];
static char PID_string_shared_mem[7];
static char PID_string_sem[16];

static int shared_memory;
static int mem_prof_fifo;

sem_t enable_semaphore;
sem_t thread_semaphore;

static int semaphore_shared_memory;
static sem_t* memory_profiler_start_semaphore;



enum {
	malloc_func = 1,
	free_func = 2
};

typedef struct memory_profiler_log_entry_s{
	pthread_t thread_id;
	int type; //malloc = 1, free = 2
	size_t  size; // in case of malloc
	int backtrace_length;
	void *call_stack[max_call_stack_depth];
	uint64_t address;
	bool valid;
}memory_profiler_log_entry_t;


typedef struct memory_profiler_struct_s {
	long unsigned int log_count;
	memory_profiler_log_entry_t log_entry[1];
} memory_profiler_struct_t;

static memory_profiler_struct_t *memory_profiler_struct;


void
signal_callback_handler(int signum)
{

	//sem_destroy(&thread_semaphore);
	sem_destroy(memory_profiler_start_semaphore);

	munmap(memory_profiler_struct, sizeof(memory_profiler_struct_t));
	munmap(memory_profiler_start_semaphore, sizeof(sem_t));

	shm_unlink(PID_string_sem);
	//shm_unlink(PID_string_shared_mem);
	printf("Caught signal %d\n",signum);

	// Terminate program
	exit(signum);
}

void __attribute__ ((constructor)) init() {

	printf("Init\n");
	
	if(sem_init(&enable_semaphore,0,1) == -1) printf("Error in enable_semaphore init, errno: %d\n",errno);
	if(sem_init(&thread_semaphore,0,1) == -1) printf("Error in thread_semaphore init, errno: %d\n",errno);

	init_done = true;

	signal(SIGINT, signal_callback_handler);

	sprintf(PID_string, "%d", getpid());
	sprintf(PID_string_shared_mem, "/%d", getpid());

	printf("current process is: %s\n", PID_string);

	sprintf(PID_string_sem, "%d_start_sem", getpid());
	semaphore_shared_memory = shm_open(PID_string_sem, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
	if (semaphore_shared_memory < 0) printf("Error while creating semaphore shared memory:%d \n", errno);

	int err = ftruncate(semaphore_shared_memory, sizeof(sem_t));
	if (err < 0) printf("semaphore_shared_memory while truncating shared memory: %d \n", errno);

	memory_profiler_start_semaphore = (sem_t*)mmap(NULL, sizeof(sem_t), PROT_WRITE,MAP_SHARED, semaphore_shared_memory, 0);
	if (memory_profiler_start_semaphore == MAP_FAILED) printf("Failed mapping the shared memory: %d \n", errno);

	if(sem_init(memory_profiler_start_semaphore,1,0) == -1) printf("Error in memory_profiler_start_semaphore init, errno: %d\n",errno);

	//Calling a dummy backtrace because it calls malloc at its first run, avoid recursion
	void *dummy_call_stack[1];
	backtrace(dummy_call_stack,1);

#ifdef START_PROF_IMM

	if(Create_shared_memory() == false ) {
		printf("Error in creating shared memory for profiling!\n");
	}
	else{
		set_profiling(true);
	}
#endif

	err = pthread_create(&memory_profiler_start_thread_id, NULL, &Memory_profiler_start_thread, NULL);
	if (err) printf("Memory_profiler_start thread creation failed error:%d \n", err);

	err = pthread_create(&hearthbeat_thread_id, NULL, &Hearthbeat, NULL);
	if (err) printf("Hearthbeat thread creation failed error:%d \n", err);

}

void __attribute__ ((destructor)) finit(){

	printf("Closing shared lib\n");
	//shm_unlink(PID_string_shared_mem);
	//munmap(memory_profiler_struct, sizeof(memory_profiler_struct_t));
	//sem_destroy(&thread_semaphore);
	//shm_unlink(PID_string_sem);
	close(mem_prof_fifo);
}

void free(void* pointer) {

	if(init_done){
	if (profiling_allowed()) {

			printf("This is from my free!\n");

			//TODO: Make this faster for multiple thread, don't defend the whole structure
			if(init_done)
			sem_wait(&thread_semaphore);

			long unsigned int new_size = sizeof(memory_profiler_struct_t) + (memory_profiler_struct->log_count + 1) * sizeof(memory_profiler_log_entry_t);

			munmap(memory_profiler_struct, sizeof(memory_profiler_struct_t)+(memory_profiler_struct->log_count) * sizeof(memory_profiler_log_entry_t));

			int err = ftruncate(shared_memory, new_size);
			if (err < 0) {
				printf("Error while truncating shared memory: %d\n", errno);
			}
			memory_profiler_struct = (memory_profiler_struct_t*)mmap(NULL, new_size, PROT_WRITE, MAP_SHARED , shared_memory, 0);
			if (memory_profiler_struct == MAP_FAILED) {
				printf("Failed mapping the shared memory: %d \n", errno);
			}

			memory_profiler_struct->log_entry[memory_profiler_struct->log_count].backtrace_length = backtrace(memory_profiler_struct->log_entry[memory_profiler_struct->log_count].call_stack,max_call_stack_depth);
			memory_profiler_struct->log_entry[memory_profiler_struct->log_count].thread_id = pthread_self();
			memory_profiler_struct->log_entry[memory_profiler_struct->log_count].type = free_func;
			memory_profiler_struct->log_entry[memory_profiler_struct->log_count].size = 0;
		    memory_profiler_struct->log_entry[memory_profiler_struct->log_count].address = (uint64_t)pointer;
		    //printf("address: %lx\n",memory_profiler_struct->log_entry[memory_profiler_struct->log_count].address);
		    memory_profiler_struct->log_entry[memory_profiler_struct->log_count].valid = true;

		    memory_profiler_struct->log_count++;

		    __libc_free(pointer);

			if(sem_post(&thread_semaphore) == -1){
				printf("Error in sem_post, errno: %d\n",errno);
			}
			return;
		}
	}

	__libc_free(pointer);
	return;
}


void* malloc(size_t size) {

	if(init_done){
	if (profiling_allowed()) {

		printf("This is from my malloc!\n");

		//TODO: Make this faster for multiple thread, don't defend the whole structure
		if(init_done)
		sem_wait(&thread_semaphore);

		void* pointer = __libc_malloc(size);

		long unsigned int new_size = sizeof(memory_profiler_struct_t) + (memory_profiler_struct->log_count + 1) * sizeof(memory_profiler_log_entry_t);

		munmap(memory_profiler_struct, sizeof(memory_profiler_struct_t)+(memory_profiler_struct->log_count) * sizeof(memory_profiler_log_entry_t));


		int err = ftruncate(shared_memory, new_size);
		if (err < 0) {
			printf("Error while truncating shared memory: %d\n", errno);
		}
		memory_profiler_struct = (memory_profiler_struct_t*)mmap(NULL, new_size, PROT_WRITE, MAP_SHARED , shared_memory, 0);
		if (memory_profiler_struct == MAP_FAILED) {
			printf("Failed mapping the shared memory: %d \n", errno);
		}

		memory_profiler_struct->log_entry[memory_profiler_struct->log_count].backtrace_length = backtrace(memory_profiler_struct->log_entry[memory_profiler_struct->log_count].call_stack,max_call_stack_depth);
		memory_profiler_struct->log_entry[memory_profiler_struct->log_count].thread_id = pthread_self();
		memory_profiler_struct->log_entry[memory_profiler_struct->log_count].type = malloc_func;
		memory_profiler_struct->log_entry[memory_profiler_struct->log_count].size = size;
	    memory_profiler_struct->log_entry[memory_profiler_struct->log_count].address = (uint64_t*)pointer;
	    //printf("address: %xl\n",(uint64_t*)pointer);
	    memory_profiler_struct->log_entry[memory_profiler_struct->log_count].valid = true;

	    memory_profiler_struct->log_count++;

		if(sem_post(&thread_semaphore) == -1){
			printf("Error in sem_post, errno: %d\n",errno);
		}

		return pointer;
	}
	}

	return __libc_malloc(size);
}


bool Create_shared_memory(){

	printf("Creating shared memory for profiling\n");

	shared_memory = shm_open(PID_string_shared_mem, O_CREAT | O_RDWR | O_EXCL, S_IRWXU | S_IRWXG | S_IRWXO);
	if (shared_memory < 0) {
		printf("Error while creating the shared memory:%d \n", errno);
		return false;
	}

	int err = ftruncate(shared_memory, sizeof(memory_profiler_struct_t));
	if (err < 0) {
		printf("Error while truncating shared memory: %d\n", errno);
		return false;
	}

	memory_profiler_struct = (memory_profiler_struct_t*)mmap(NULL, sizeof(memory_profiler_struct_t), PROT_WRITE, MAP_SHARED , shared_memory, 0);
	if (memory_profiler_struct == MAP_FAILED) {
		printf("Failed mapping the shared memory: %d \n", errno);
		return false;
	}

	return true;
}

bool Open_shared_memory(){

	printf("Opening shared memory for profiling\n");

	shared_memory = shm_open(PID_string_shared_mem, O_RDWR , S_IRWXU | S_IRWXG | S_IRWXO);
	if (shared_memory < 0) {
		printf("Error while opening shared memory:%d \n", errno);
		return false;
	}

	memory_profiler_struct = (memory_profiler_struct_t*)mmap(NULL, sizeof(memory_profiler_struct_t), PROT_WRITE, MAP_SHARED , shared_memory, 0);
	if (memory_profiler_struct == MAP_FAILED) {
		printf("Failed mapping the shared memory: %d \n", errno);
		return false;
	}
	return true;
}

void* Memory_profiler_start_thread(void *arg){

	while(true){

		sem_wait(memory_profiler_start_semaphore);

		if (profiling_allowed() == false) {

			if(Open_shared_memory() == false){
				printf("Failed to open shared memory!\n");
			}
			else{
				set_profiling(true);
			}

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

