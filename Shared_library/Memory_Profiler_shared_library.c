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

static long unsigned int shared_memory_size;

/**
 * Semaphore for defending the global variable starting/stopping the profiling
 */
sem_t enable_semaphore;

/**
 * Semaphore for defending the shared memory writing between threads
 */
sem_t thread_semaphore;

/**
 * File handler of shared semaphore for starting/stopping the profiling
 */
static int semaphore_shared_memory;

/**
 * Starting/stopping profiling
 */
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
	shm_unlink(PID_string_shared_mem);
	printf("Caught signal %d\n",signum);

	// Terminate program
	exit(signum);
}

FILE *pfile;
char s[1000];
char file[50];

void print_to_log(char *text){
	pfile = fopen(file,"a+");
	fputs(text,pfile);
	memset(text,0,sizeof(text));
	fclose(pfile);
}



void __attribute__ ((constructor)) Memory_profiler_shared_library_init() {

	printf("Init\n");
	sprintf(file,"shared_lib_log_%d.txt",getpid());


	sprintf(s,"Starting to initialize shared memory\n");
	print_to_log(s);
	
	if(sem_init(&enable_semaphore,0,1) == -1) {
		printf("Error in enable_semaphore init, errno: %d\n",errno);
		sprintf(s,"Error in enable_semaphore init, errno: %d\n",errno);
		print_to_log(s);
	}
	else{
		sprintf(s,"Initializing enable_semaphore was successful\n");
		print_to_log(s);
	}

	if(sem_init(&thread_semaphore,0,1) == -1) {
		printf("Error in thread_semaphore init, errno: %d\n",errno);
		sprintf(s,"Error in thread_semaphore init, errno: %d\n",errno);
		print_to_log(s);
	}
	else{
		sprintf(s,"Initializing thread_semaphore was successful\n");
		print_to_log(s);
	}

	signal(SIGINT, signal_callback_handler);

	sprintf(PID_string, "%d", getpid());
	printf("current process is: %s \n", PID_string);

	sprintf(s,"current process is: %s\n",PID_string);
	print_to_log(s);

	sprintf(PID_string_shared_mem, "/%d", getpid());

	sprintf(PID_string_sem, "%d_start_sem", getpid());
	semaphore_shared_memory = shm_open(PID_string_sem, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
	if (semaphore_shared_memory < 0) {
		printf("Error while creating semaphore shared memory:%d \n", errno);
		sprintf(s,"Error while creating semaphore shared memory:%d \n", errno);
		print_to_log(s);
	}
	else{
		sprintf(s,"Creating semaphore shared memory was successful\n");
		print_to_log(s);
	}

	int err = ftruncate(semaphore_shared_memory, sizeof(sem_t));
	if (err < 0) {
		printf("semaphore_shared_memory while truncating shared memory: %d \n", errno);
		sprintf(s,"semaphore_shared_memory while truncating shared memory: %d \n", errno);
		print_to_log(s);
		}
	else{
		sprintf(s,"Truncating semaphore_shared_memory was successful\n");
		print_to_log(s);
	}

	memory_profiler_start_semaphore = (sem_t*)mmap(NULL, sizeof(sem_t), PROT_WRITE,MAP_SHARED, semaphore_shared_memory, 0);
	if (memory_profiler_start_semaphore == MAP_FAILED) {
		printf("Failed mapping the shared memory: %d \n", errno);
		sprintf(s,"Failed mapping the shared memory: %d \n", errno);
		print_to_log(s);
	}
	else{
		sprintf(s,"Mapping shared memory was successful\n");
		print_to_log(s);
	}

	if(sem_init(memory_profiler_start_semaphore,1,0) == -1) {
		printf("Error in memory_profiler_start_semaphore init, errno: %d\n",errno);
		sprintf(s,"Error in memory_profiler_start_semaphore init, errno: %d\n",errno);
		print_to_log(s);
	}
	else{
		sprintf(s,"Initializing memory_profiler_start_semaphore was successful\n");
		print_to_log(s);
	}

	//Calling a dummy backtrace because it calls malloc at its first run, avoid recursion
	void *dummy_call_stack[1];
	backtrace(dummy_call_stack,1);

#ifdef START_PROF_IMM

	if(Create_shared_memory() == false ) {
		printf("Error in creating shared memory for profiling!\n");
		sprintf(s,"Error in creating shared memory for profiling!\n");
		print_to_log(s);
	}
	else{
		sprintf(s,"Creating shared memory for profiling was successful!\n");
		print_to_log(s);
		set_profiling(true);
	}
#endif

	err = pthread_create(&memory_profiler_start_thread_id, NULL, &Memory_profiler_start_thread, NULL);
	if (err) {
		printf("Memory_profiler_start thread creation failed error:%d \n", err);
		sprintf(s,"Memory_profiler_start thread creation failed error:%d \n", err);
		print_to_log(s);
	}
	else{
		sprintf(s,"Memory_profiler_start thread creation was successful\n", err);
		print_to_log(s);
	}

	err = pthread_create(&hearthbeat_thread_id, NULL, &Hearthbeat, NULL);
	if (err) {
		printf("Hearthbeat thread creation failed error:%d \n", err);
		sprintf(s,"Hearthbeat thread creation failed error:%d \n", err);
		print_to_log(s);
	}
	else{
		sprintf(s,"Hearthbeat thread creation was successful\n", err);
		print_to_log(s);
	}

	sprintf(s,"Shared library init finished!\n");
	print_to_log(s);

	init_done = true;

	printf("INIT done\n");

}

void __attribute__ ((destructor)) Memory_profiler_shared_library_finit(){

	printf("Closing shared lib\n");
	shm_unlink(PID_string_shared_mem);
	munmap(memory_profiler_struct, sizeof(memory_profiler_struct_t));
	sem_destroy(&thread_semaphore);
	shm_unlink(PID_string_sem);
	close(mem_prof_fifo);
}

void free(void* pointer) {

	if(init_done){
	sem_wait(&thread_semaphore);
	if (profiling_allowed()) {

			printf("This is from my free!\n");

			long unsigned int new_shared_memory_size = sizeof(memory_profiler_struct_t) + (memory_profiler_struct->log_count) * sizeof(memory_profiler_log_entry_t);

			munmap(memory_profiler_struct, shared_memory_size);

			shared_memory_size = new_shared_memory_size;

			int err = ftruncate(shared_memory, shared_memory_size);
			if (err < 0) {
				printf("Error while truncating shared memory: %d\n", errno);
			}
			memory_profiler_struct = (memory_profiler_struct_t*)mmap(NULL, shared_memory_size, PROT_WRITE, MAP_SHARED , shared_memory, 0);
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
			printf("Free end\n");
			return;
		}
	if(sem_post(&thread_semaphore) == -1){
		printf("Error in sem_post, errno: %d\n",errno);
	}
	}

	__libc_free(pointer);
	return;
}


void* malloc(size_t size) {


	if(init_done){

		sem_wait(&thread_semaphore);
		if (profiling_allowed()) {

			printf("This is from my malloc!\n");

			void* pointer = __libc_malloc(size);

			unsigned long new_shared_memory_size = sizeof(memory_profiler_struct_t) + (memory_profiler_struct->log_count) * sizeof(memory_profiler_log_entry_t);

			munmap(memory_profiler_struct, shared_memory_size);

			shared_memory_size = new_shared_memory_size;

			int err = ftruncate(shared_memory, shared_memory_size);
			if (err < 0) {
				printf("Error while truncating shared memory: %d\n", errno);
			}
			memory_profiler_struct = (memory_profiler_struct_t*)mmap(NULL, shared_memory_size, PROT_WRITE, MAP_SHARED , shared_memory, 0);
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
			printf("Malloc end\n");
			return pointer;
		}
		if(sem_post(&thread_semaphore) == -1){
			printf("Error in sem_post, errno: %d\n",errno);
		}
	}

	return __libc_malloc(size);
}


bool Create_shared_memory(){

	printf("Creating shared memory for profiling\n");
	sprintf(s,"Create_shared_memory: Creating shared memory for profiling\n");
	print_to_log(s);

	shared_memory_size = sizeof(memory_profiler_struct_t);

	shared_memory = shm_open(PID_string_shared_mem, O_CREAT | O_RDWR | O_EXCL, S_IRWXU | S_IRWXG | S_IRWXO);
	if (shared_memory < 0) {
		printf("Error while creating the shared memory:%d \n", errno);
		shared_memory_size = 0;
		sprintf(s,"Create_shared_memory: Error while creating the shared memory:%d \n", errno);
		print_to_log(s);
		return false;
	}

	int err = ftruncate(shared_memory, shared_memory_size);
	if (err < 0) {
		printf("Error while truncating shared memory: %d\n", errno);
		shared_memory_size = 0;
		sprintf(s,"Create_shared_memory: Error while truncating shared memory: %d\n", errno);
		print_to_log(s);
		return false;
	}

	memory_profiler_struct = (memory_profiler_struct_t*)mmap(NULL, shared_memory_size, PROT_WRITE, MAP_SHARED , shared_memory, 0);
	if (memory_profiler_struct == MAP_FAILED) {
		printf("Failed mapping the shared memory: %d \n", errno);
		sprintf(s,"Create_shared_memory: Failed mapping the shared memory: %d\n", errno);
		print_to_log(s);
		return false;
	}

	return true;
}

bool Open_shared_memory(){

	printf("Opening shared memory for profiling\n");
	sprintf(s,"Open_shared_memory: Opening shared memory for profiling\n");
	print_to_log(s);

	if(shared_memory_size == 0){
		shared_memory_size = sizeof(memory_profiler_struct_t);
	}
	printf("SIZE in open: %lu\n",shared_memory_size);

	shared_memory = shm_open(PID_string_shared_mem, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
	if (shared_memory < 0) {
		printf("Error while opening shared memory:%d \n", errno);
		sprintf(s,"Open_shared_memory: Error while opening shared memory:%d \n", errno);
		print_to_log(s);
		return false;
	}

	memory_profiler_struct = (memory_profiler_struct_t*)mmap(NULL, /*sizeof(memory_profiler_struct_t)*/shared_memory_size, PROT_WRITE, MAP_SHARED , shared_memory, 0);
	if (memory_profiler_struct == MAP_FAILED) {
		printf("Failed mapping the shared memory: %d \n", errno);
		sprintf(s,"Open_shared_memory: Failed mapping the shared memory: %d \n", errno);
		print_to_log(s);
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
				sprintf(s,"Memory_profiler_start_thread: Failed to open shared memory!\n");
				print_to_log(s);
			}
			else{
				set_profiling(true);
			}

		} else {
			printf("closing shared memory for profiling\n");
			// Need to check thread_semaphore because if a thread is executing malloc/free we cannot set profiling to false,
			// because when we unmap the shared memory in the next instruction, and the thread may need it from malloc/free, we have to wait for it to finish
			sem_wait(&thread_semaphore);
				set_profiling(false);
			sem_post(&thread_semaphore);
			munmap(memory_profiler_struct, shared_memory_size);

			sprintf(s,"Memory_profiler_start_thread: closing shared memory for profiling\n");
			print_to_log(s);
		}
	}
}

void* Hearthbeat(void *arg) {

	while (true) {

		mem_prof_fifo = open(fifo_path, O_WRONLY);

		if (mem_prof_fifo != -1) {

			if (write(mem_prof_fifo, &PID_string, sizeof(PID_string)) == -1) {

				printf("Failed writing the FIFO\n");
				sprintf(s,"Hearthbeat: Failed writing the FIFO\n");
				print_to_log(s);
			}

			close(mem_prof_fifo);
		} else {
			printf("Failed opening the FIFO, errno: %d\n", errno);
			sprintf(s,"Hearthbeat: Failed opening the FIFO, errno: %d\n", errno);
			print_to_log(s);
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

