#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "Process.h"

using namespace std;

Process_handler::Process_handler(){

	PID = 0;
	PID_string = "";
	profiled = false;
	memory_profiler_struct = NULL;
	shared_memory = 0;
    semaphore_shared_memory = 0;
    semaphore = NULL;

}

Process_handler::Process_handler(pid_t PID) {



	this->PID = PID;
	PID_string=to_string(PID);
	profiled = false;
	memory_profiler_struct = NULL;
	shared_memory = 0;
    semaphore_shared_memory = 0;
    semaphore = NULL;

    Init_semaphore();
}

/*Process_handler::Process_handler(const Process_handler &obj) {

	cout << "copy constructor" << endl;

	this->PID = obj.PID;
	this->profiled = obj.profiled;
	this->memory_profiler_struct = NULL;
	this->shared_memory = 0;

	//this->Init_shared_memory();

}*/

Process_handler::Process_handler(Process_handler &&obj) {

	cout << "move constructor" << endl;

	PID = obj.PID;
	PID_string = obj.PID_string;
	profiled = obj.profiled;
	memory_profiler_struct = obj.memory_profiler_struct;
	shared_memory = obj.shared_memory;
    semaphore_shared_memory = obj.semaphore_shared_memory;
    semaphore = obj.semaphore;

	obj.PID = 0;
	obj.PID_string = "";
	obj.profiled = false;
	obj.memory_profiler_struct = NULL;
	obj.shared_memory = 0;
	obj.semaphore_shared_memory = 0;
	obj.semaphore = NULL;
}

Process_handler& Process_handler::operator=(Process_handler&& obj){

	if (this!=&obj){

		PID = 0;
		PID_string = "";
		profiled = 0;
		delete memory_profiler_struct;
		shared_memory = 0;
	    semaphore_shared_memory = 0;
	    delete semaphore;

		PID = obj.PID;
		PID_string = obj.PID_string;
		profiled = obj.profiled;
		memory_profiler_struct = obj.memory_profiler_struct;
		shared_memory = obj.shared_memory;
	    semaphore_shared_memory = obj.semaphore_shared_memory;
	    semaphore = obj.semaphore;


		obj.PID = 0;
		obj.PID_string = "";
		obj.profiled = false;
		obj.memory_profiler_struct = NULL;
		obj.shared_memory = 0;
		obj.semaphore_shared_memory = 0;
		obj.semaphore = NULL;
	}
return *this;
}

Process_handler::~Process_handler() {

	cout << "Process destructor" << endl;

	//delete memory_profiler_struct;

	munmap(semaphore, sizeof(sem_t));
	shm_unlink(("/" + PID_string +"_start_sem").c_str());

	munmap(memory_profiler_struct, sizeof(memory_profiler_log_entry_t));
	shm_unlink(("/" + PID_string).c_str());

}

void Process_handler::Init_semaphore() {

	this->semaphore_shared_memory = shm_open(("/"+PID_string +"_start_sem").c_str(), O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
	if (semaphore_shared_memory < 0) printf("Error while creating semaphore shared memory:%d \n", errno);

	this->semaphore = (sem_t*)mmap(NULL, sizeof(sem_t), PROT_WRITE,MAP_SHARED, semaphore_shared_memory, 0);
	if (semaphore == MAP_FAILED) {
		printf("Failed mapping the semaphore shared memory: %d \n", errno);
	}
}

void Process_handler::Init_shared_memory() {

	this->shared_memory = shm_open(("/"+PID_string).c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
	if (shared_memory < 0) printf("Error while creating shared memory:%d \n", errno);

	int err = ftruncate(shared_memory, sizeof(memory_profiler_struct_t));
	if (err < 0) printf("Error while truncating shared memory: %d \n", errno);

	this->memory_profiler_struct = (memory_profiler_struct_t*) mmap(NULL, sizeof(memory_profiler_struct_t),PROT_READ, MAP_SHARED, this->shared_memory, 0);
	if (memory_profiler_struct == MAP_FAILED) printf("Failed mapping the shared memory: %d \n", errno);

}

void Process_handler::Send_signal() {

	kill(this->PID, SIGUSR1);

}

void Process_handler::Start_profiling(){

	sem_post(semaphore);

}

memory_profiler_struct_t* Process_handler::Get_shared_memory() {

	return memory_profiler_struct;
}
