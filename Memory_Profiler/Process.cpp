#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <bfd.h>
#include <fstream>
#include <sstream>
#include <regex.h>
#include <stdlib.h>
#include <unistd.h>

#include "Process.h"

using namespace std;


Process_handler::Process_handler() {

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
	PID_string = to_string(PID);
	profiled = false;
	alive = true;
	memory_profiler_struct = NULL;
	shared_memory = 0;
	semaphore_shared_memory = 0;
	semaphore = NULL;
	elf_path = "/proc/" + this->PID_string + "/exe";

	if (!Create_symbol_table()) {
		cout << "Error creating the symbol table" << endl;
	}
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
	elf_path = obj.elf_path;

	obj.PID = 0;
	obj.PID_string = "";
	obj.profiled = false;
	obj.alive = false;
	obj.memory_profiler_struct = NULL;
	obj.shared_memory = 0;
	obj.semaphore_shared_memory = 0;
	obj.semaphore = NULL;
	obj.elf_path = "";
}

Process_handler& Process_handler::operator=(Process_handler&& obj) {

	if (this != &obj) {

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
		elf_path = obj.elf_path;

		obj.PID = 0;
		obj.PID_string = "";
		obj.profiled = false;
		obj.alive = false;
		obj.memory_profiler_struct = NULL;
		obj.shared_memory = 0;
		obj.semaphore_shared_memory = 0;
		obj.semaphore = NULL;
		obj.elf_path = "";
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

bfd* Process_handler::Open_ELF() {

	return Open_ELF(elf_path);

}

bfd* Process_handler::Open_ELF(string ELF_path){

	bfd* tmp_bfd = bfd_openr(ELF_path.c_str(),NULL);
		if (tmp_bfd == NULL) {
			printf("Error opening Process ELF file");
			return NULL;
		}
		//check if the file is in format
		if (!bfd_check_format(tmp_bfd, bfd_object)) {
			if (bfd_get_error() != bfd_error_file_ambiguously_recognized) {
				//printf("Incompatible format\n");
				return NULL;
			}
		}

		return tmp_bfd;

}

bool Process_handler::Create_symbol_table() {

	bfd* tmp_bfd;
	long storage_needed;
	asymbol **symbol_table;
	long number_of_symbols;
	symbol_table_entry_struct_t symbol_entry;

	char program_path[1024];
	int len;

	int counter=0;

	tmp_bfd = Open_ELF();
	if (!tmp_bfd) {
		return false;
	}

	storage_needed = bfd_get_symtab_upper_bound(tmp_bfd);
	symbol_table = (asymbol**) malloc(storage_needed);
	number_of_symbols = bfd_canonicalize_symtab(tmp_bfd, symbol_table);

	if (number_of_symbols < 0) {
		return false;
	}

	if(Read_virtual_memory_mapping() == false){
		return false;
	}

	if ((len = readlink(elf_path.c_str(), program_path, sizeof(program_path)-1)) != -1)
		program_path[len] = '\0';

	for (int i = 0; i < number_of_symbols; i++) {
		if (symbol_table[i]->flags & BSF_FUNCTION) {
			string name(symbol_table[i]->name);

			symbol_entry.name = name;
			symbol_entry.address = 0;

			if (symbol_table[i]->value != 0) {
				// Symbol is defined in the process, address is known from ELF
				symbol_entry.address = symbol_table[i]->section->vma + symbol_table[i]->value;
			} else {
				// Symbol is not defined in the process, address is not known from ELF, need to get it from shared library
				if (name == "malloc" || name == "free") {
					counter++;
					// Get the address from memory_profiler_shared_library
					string memory_profiler_library = "libMemory_profiler_shared_library.so";
					for(unsigned int i = 0; i < memory_map_table.size(); i++){
						if(memory_map_table[i].shared_lib_path.find(memory_profiler_library) !=  string::npos){
							symbol_entry.address = memory_map_table[i].start_address + Get_symbol_address_from_ELF(memory_map_table[i].shared_lib_path,symbol_entry.name);
							break;
						}
					}

				} else {
					// Find the symbol from one of the dynamically linked shared library
					for(unsigned int i = 0; i < memory_map_table.size(); i++){

						if(memory_map_table[i].shared_lib_path.compare(program_path) !=0){
							if(Find_symbol_in_ELF(memory_map_table[i].shared_lib_path,symbol_entry.name)){
									symbol_entry.address = memory_map_table[i].start_address + Get_symbol_address_from_ELF(memory_map_table[i].shared_lib_path,symbol_entry.name);
									break;
							}
						}
					}
				}
			}


			this->function_symbol_table.push_back(symbol_entry);
			cout << "symbol name: " << symbol_entry.name << " symbol address: " << std::hex << symbol_entry.address << endl;
		}
	}
	cout << "counter: " << std::dec << counter << endl;
	// Free the symbol table allocated in
	free(symbol_table);
	return true;

}

bool Process_handler::Read_virtual_memory_mapping() {


	ifstream mapping("/proc/" + this->PID_string + "/maps");
	string line;

	size_t pos_shared_lib;
	size_t pos_end_address_start;
	size_t pos_end_address_stop;


	memory_map_table_entry_struct_t entry;

	while (getline(mapping, line)) {

			try{

				pos_shared_lib = line.find("/");

				entry.shared_lib_path = line.substr(pos_shared_lib);

				pos_end_address_start = line.find_first_of("-");
				pos_end_address_stop = line.find_first_of(" ");

				entry.start_address = stoul(line.substr(0,pos_end_address_start).c_str(),NULL,16);
				entry.end_address = stoul(line.substr(pos_end_address_start+1,pos_end_address_stop-pos_end_address_start).c_str(),NULL,16);

				memory_map_table.push_back(entry);


				/*cout << "start_address number: " << entry.start_address << endl;
				cout << "end_address: " << entry.end_address << endl;
				cout << "Shared library location: " << entry.shared_lib_path << endl << endl;*/
			}
			catch(exception &e){
					//cout << "No shared lib" << endl;
				// examine exception and return with false if necessary
			}
	}


	return true;
}

uint64_t Process_handler::Get_symbol_address_from_ELF(string ELF_path,string symbol_name){

	bfd* tmp_bfd;
	long storage_needed;
	asymbol **symbol_table;
	long number_of_symbols;

	uint64_t address;

	tmp_bfd = Open_ELF(ELF_path);

	storage_needed = bfd_get_symtab_upper_bound(tmp_bfd);
	symbol_table = (asymbol**) malloc(storage_needed);
	number_of_symbols = bfd_canonicalize_symtab(tmp_bfd, symbol_table);


	for (int i = 0; i < number_of_symbols; i++) {
			if (symbol_table[i]->flags & BSF_FUNCTION) {
				if (symbol_table[i]->value != 0) {
					string name(symbol_table[i]->name);
					if(name == symbol_name){
						address = symbol_table[i]->section->vma + symbol_table[i]->value;
						break;
					}
				}
			}
		}

	free(symbol_table);
	return address;

}

bool Process_handler::Find_symbol_in_ELF(string ELF_path, string symbol_name){

	bfd* tmp_bfd;
	long storage_needed;
	asymbol **symbol_table;
	long number_of_symbols;

	tmp_bfd = Open_ELF(ELF_path);
	if(tmp_bfd == NULL){
		return false;
	}

	storage_needed = bfd_get_symtab_upper_bound(tmp_bfd);
	symbol_table = (asymbol**) malloc(storage_needed);
	number_of_symbols = bfd_canonicalize_symtab(tmp_bfd, symbol_table);

	for (int i = 0; i < number_of_symbols; i++) {
		if (symbol_table[i]->flags & BSF_FUNCTION) {
			string name(symbol_table[i]->name);
			// If a symbol with the same name is found we has to be sure it is not a symbol from a shared library linked to this shared lib
			if(symbol_name.compare(name)==0 && symbol_table[i]->value > 0){
				free(symbol_table);
				return true;
			}
		}
	}
	free(symbol_table);
	return false;
}

void Process_handler::Init_semaphore() {

	this->semaphore_shared_memory = shm_open(
			("/" + PID_string + "_start_sem").c_str(), O_RDWR,
			S_IRWXU | S_IRWXG | S_IRWXO);
	if (semaphore_shared_memory < 0)
		printf("Error while opening semaphore shared memory:%d \n", errno);

	this->semaphore = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_WRITE, MAP_SHARED,
			semaphore_shared_memory, 0);
	if (semaphore == MAP_FAILED)
		printf("Failed mapping the semaphore shared memory: %d \n", errno);
}

void Process_handler::Init_shared_memory() {

	// If this is the first profiling of a rpocess create the shared memory, if the shared memory already exists (e.g: second profiling for a process), use it
	this->shared_memory = shm_open(("/" + PID_string).c_str(),
	O_CREAT | O_RDWR | O_EXCL, S_IRWXU | S_IRWXG | S_IRWXO);

	if (shared_memory < 0) {
		if (errno == EEXIST) {
			return;
		} else
			printf("Error while creating shared memory:%d \n", errno);
	} else {
		// Map the shared memory if it has not existed before
		int err = ftruncate(shared_memory, sizeof(memory_profiler_struct_t));
		if (err < 0)
			printf("Error while truncating shared memory: %d \n", errno);

		this->memory_profiler_struct = (memory_profiler_struct_t*) mmap(NULL,
				sizeof(memory_profiler_struct_t), PROT_READ, MAP_SHARED,
				this->shared_memory, 0);
		if (memory_profiler_struct == MAP_FAILED)
			printf("Failed mapping the shared memory: %d \n", errno);

	}

}

void Process_handler::Send_signal() {

	kill(this->PID, SIGUSR1);

}

void Process_handler::Start_Stop_profiling() {

	sem_post(semaphore);

}

memory_profiler_struct_t* Process_handler::Get_shared_memory() {

	return memory_profiler_struct;
}
