#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <bfd.h>

#include "Process.h"

using namespace std;

Process_handler::Process_handler(){

	PID = 0;
	PID_string = "";
	profiled = false;
	alive = false;
	memory_profiler_struct = NULL;
	shared_memory = 0;
    semaphore_shared_memory = 0;
    semaphore = NULL;

}

Process_handler::Process_handler(pid_t PID) {



	this->PID = PID;
	PID_string=to_string(PID);
	profiled = false;
	alive = true;
	memory_profiler_struct = NULL;
	shared_memory = 0;
    semaphore_shared_memory = 0;
    semaphore = NULL;

    Parse_ELF();
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
	alive = obj.alive;
	memory_profiler_struct = obj.memory_profiler_struct;
	shared_memory = obj.shared_memory;
    semaphore_shared_memory = obj.semaphore_shared_memory;
    semaphore = obj.semaphore;

	obj.PID = 0;
	obj.PID_string = "";
	obj.profiled = false;
	obj.alive = false;
	obj.memory_profiler_struct = NULL;
	obj.shared_memory = 0;
	obj.semaphore_shared_memory = 0;
	obj.semaphore = NULL;
}

Process_handler& Process_handler::operator=(Process_handler&& obj){

	if (this!=&obj){

		PID = 0;
		PID_string = "";
		profiled = false;
		alive = false;
		delete memory_profiler_struct;
		shared_memory = 0;
	    semaphore_shared_memory = 0;
	    delete semaphore;

		PID = obj.PID;
		PID_string = obj.PID_string;
		profiled = obj.profiled;
		alive = obj.alive;
		memory_profiler_struct = obj.memory_profiler_struct;
		shared_memory = obj.shared_memory;
	    semaphore_shared_memory = obj.semaphore_shared_memory;
	    semaphore = obj.semaphore;


		obj.PID = 0;
		obj.PID_string = "";
		obj.profiled = false;
		obj.alive = false;
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
	munmap(memory_profiler_struct, sizeof(memory_profiler_log_entry_t));
	shm_unlink(("/" + PID_string).c_str());

}

void Process_handler::Parse_ELF(){

	bfd *tmp_bfd = NULL;
    long storage_needed;
    asymbol **symbol_table;
    long number_of_symbols;
    long i;

	tmp_bfd = bfd_openr(("/proc/" +this->PID_string+ "/exe").c_str(), NULL);
	if (tmp_bfd == NULL) {
	    printf ("Error openning file");
		exit(-1);
	}
	//check if the file is in format
	if (!bfd_check_format (tmp_bfd, bfd_object)) {
		if (bfd_get_error () != bfd_error_file_ambiguously_recognized) {
			printf("Incompatible format\n");
			exit(-1);
		}
	}

	cout << endl << "Process "<< std::dec << this->PID <<" symbol map"<< endl <<endl;

	cout << "exec file format is" << tmp_bfd->xvec->name << endl;


	storage_needed = bfd_get_symtab_upper_bound (tmp_bfd);

	if (storage_needed < 0)
	  return;

	if (storage_needed == 0)
	  return;

	cout << "storage_needed: " << storage_needed << endl;
	symbol_table = (asymbol**)malloc(storage_needed);
	number_of_symbols = bfd_canonicalize_symtab (tmp_bfd, symbol_table);

	if (number_of_symbols < 0)
	  return;

	cout << "start address: "<< hex <<tmp_bfd->start_address << endl;

	for (i = 0; i < number_of_symbols; i++) {
		if(symbol_table[i]->flags & BSF_FUNCTION){
		cout << "name: " << symbol_table[i]->name << "  value: " << std::hex << symbol_table[i]->value << "  type: ";
		if(symbol_table[i]->flags & BSF_LOCAL) cout << "BSF_LOCAL" << endl;
		else if(symbol_table[i]->flags & BSF_GLOBAL) cout << "BSF_GLOBAL" << endl;
		else cout<< endl;
		cout << "section VMA: " << symbol_table[i]->section->vma << endl;
		}
	}

	free(symbol_table);

}

void Process_handler::Init_semaphore() {

	this->semaphore_shared_memory = shm_open(("/"+PID_string +"_start_sem").c_str(), O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
	if (semaphore_shared_memory < 0) printf("Error while opening semaphore shared memory:%d \n", errno);

	this->semaphore = (sem_t*)mmap(NULL, sizeof(sem_t), PROT_WRITE,MAP_SHARED, semaphore_shared_memory, 0);
	if (semaphore == MAP_FAILED) printf("Failed mapping the semaphore shared memory: %d \n", errno);
}

void Process_handler::Init_shared_memory() {

	// If this is the first profiling of a rpocess create the shared memory, if the shared memory already exists (e.g: second profiling for a process), use it
	this->shared_memory = shm_open(("/"+PID_string).c_str(), O_CREAT | O_RDWR | O_EXCL, S_IRWXU | S_IRWXG | S_IRWXO);

	if (shared_memory < 0) {
		if(errno == EEXIST) {
			return;
		}
		else printf("Error while creating shared memory:%d \n", errno);
	}
	else{
		// Map the shared memory if it has not existed before
		int err = ftruncate(shared_memory, sizeof(memory_profiler_struct_t));
		if (err < 0) printf("Error while truncating shared memory: %d \n", errno);

		this->memory_profiler_struct = (memory_profiler_struct_t*) mmap(NULL, sizeof(memory_profiler_struct_t),PROT_READ, MAP_SHARED, this->shared_memory, 0);
		if (memory_profiler_struct == MAP_FAILED) printf("Failed mapping the shared memory: %d \n", errno);

	}


}

void Process_handler::Send_signal() {

	kill(this->PID, SIGUSR1);

}

void Process_handler::Start_Stop_profiling(){

	sem_post(semaphore);

}

memory_profiler_struct_t* Process_handler::Get_shared_memory() {

	return memory_profiler_struct;
}
