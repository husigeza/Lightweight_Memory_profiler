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
#include <algorithm>

#include "Process.h"

using namespace std;

#define memory_profiler_library "libMemory_profiler_shared_library.so"

bool operator == (const uint64_t &address, const symbol_table_entry_struct_t& entry){ return (address == entry.address);}
bool operator != (const uint64_t &address, const symbol_table_entry_struct_t& entry){ return !(address == entry.address);}
bool operator <  (const uint64_t &address, const symbol_table_entry_struct_t& entry){ return (address < entry.address);}
bool operator >  (const uint64_t &address, const symbol_table_entry_struct_t& entry){ return !(address < entry.address);}
bool operator <= (const uint64_t &address, const symbol_table_entry_struct_t& entry){ return (address < entry.address);}
bool operator >= (const uint64_t &address, const symbol_table_entry_struct_t& entry){ return !(address < entry.address);}

bool operator == (const symbol_table_entry_struct_t& entry, const uint64_t &address){return (entry.address == address);}
bool operator != (const symbol_table_entry_struct_t& entry, const uint64_t &address){return !(entry.address == address);}
bool operator <  (const symbol_table_entry_struct_t& entry, const uint64_t &address){return (entry.address < address);}
bool operator >  (const symbol_table_entry_struct_t& entry, const uint64_t &address){return !(entry.address < address);}
bool operator <= (const symbol_table_entry_struct_t& entry, const uint64_t &address){return (entry.address < address);}
bool operator >= (const symbol_table_entry_struct_t& entry, const uint64_t &address){return (entry.address > address);}




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

Process_handler::Process_handler(const Process_handler &obj){

	PID = obj.PID;
	PID_string = obj.PID_string;
	profiled = obj.profiled;
	alive = obj.alive;
	memory_profiler_struct = obj.memory_profiler_struct;
	shared_memory = obj.shared_memory;
	semaphore_shared_memory = obj.semaphore_shared_memory;
	semaphore = obj.semaphore;
	elf_path = obj.elf_path;
	memory_map_table = obj.memory_map_table;
	function_symbol_table = obj.function_symbol_table;

}

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
	memory_map_table = obj.memory_map_table;
	function_symbol_table = obj.function_symbol_table;

	obj.PID = 0;
	obj.PID_string = "";
	obj.profiled = false;
	obj.alive = false;
	obj.memory_profiler_struct = NULL;
	obj.shared_memory = 0;
	obj.semaphore_shared_memory = 0;
	obj.semaphore = NULL;
	obj.elf_path = "";
	obj.memory_map_table.clear();
	obj.function_symbol_table.clear();
}

Process_handler& Process_handler::operator=(Process_handler&& obj) {

	if (this != &obj) {

		PID = obj.PID;
		PID_string = obj.PID_string;
		profiled = obj.profiled;
		alive = obj.alive;
		memory_profiler_struct = obj.memory_profiler_struct;
		shared_memory = obj.shared_memory;
		semaphore_shared_memory = obj.semaphore_shared_memory;
		semaphore = obj.semaphore;
		elf_path = obj.elf_path;
		memory_map_table = obj.memory_map_table;
		function_symbol_table = obj.function_symbol_table;

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

	memory_map_table.clear();
	function_symbol_table.clear();

	munmap(semaphore, sizeof(sem_t));
	munmap(memory_profiler_struct, sizeof(memory_profiler_log_entry_t));
	shm_unlink(("/" + PID_string).c_str());

}

/*
 * Returns with the index of function in vector
 */
vector<symbol_table_entry_struct_t>::iterator Process_handler::Find_function(uint64_t &address){

	vector<symbol_table_entry_struct_t>::iterator it = lower_bound(function_symbol_table.begin(),function_symbol_table.end(),address);
	//vector<symbol_table_entry_struct_t>::iterator it = upper_bound(function_symbol_table.begin(),function_symbol_table.end(),address);
	if(it == function_symbol_table.end()) it = it-1;

	return it;
}


/*
 * Need to free bfd manually after this function !!!
 */
bfd* Process_handler::Open_ELF() {

	return Open_ELF(elf_path);
}

/*
 * Need to close bfd manually after this function !!!
 */
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

/*
 * Need to free symbol_table manually after this function !!!
 */
long Process_handler::Parse_symbol_table_from_ELF(bfd* bfd_ptr,asymbol ***symbol_table){

	long storage_needed;
	long number_of_symbols;

	storage_needed = bfd_get_dynamic_symtab_upper_bound(bfd_ptr);
	*symbol_table = (asymbol**) malloc(storage_needed);
	number_of_symbols = bfd_canonicalize_dynamic_symtab(bfd_ptr, *symbol_table);
	return number_of_symbols;
}

bool Process_handler::Create_symbol_table() {

	bfd* tmp_bfd;
	asymbol **symbol_table = 0;
	long number_of_symbols;

	symbol_table_entry_struct_t symbol_entry;

	// TODO This needs to be rethinked, define a const for it or a find dynamic way
	char program_path[1024];
	int len;

	tmp_bfd = Open_ELF();
	if (!tmp_bfd) {
		return false;
	}

	number_of_symbols = Parse_symbol_table_from_ELF(tmp_bfd,&symbol_table);
	if (number_of_symbols < 0) {
		return false;
	}

	// Get the virtual addresses of the shared libs linked to the program
	if(Read_virtual_memory_mapping() == false){
		return false;
	}

	// /proc/PID/exe is a sym link to the executable, read it with readlink because we need the full path of the executable
	if ((len = readlink(elf_path.c_str(), program_path, sizeof(program_path)-1)) != -1)
		program_path[len] = '\0';

	for (int i = 0; i < number_of_symbols; i++) {
		if (symbol_table[i]->flags & BSF_FUNCTION) {

			// Initialize name and address
			symbol_entry.name = symbol_table[i]->name;
			symbol_entry.address = 0;

			if (symbol_table[i]->value != 0) {
				// Symbol is defined in the process, address is known from ELF
				symbol_entry.address = (uint64_t)(symbol_table[i]->section->vma + symbol_table[i]->value);
			} else {
				// Symbol is not defined, address is not known from ELF, need to get it from shared library
				if (symbol_entry.name == "malloc" || symbol_entry.name == "free") {

					// Get the address from memory_profiler_shared_library
					for(unsigned int i = 0; i < memory_map_table.size(); i++){
						//Find the entry of the memory_profiler_shared_library, read its VM base address, and find malloc/free address offset from it
						if(memory_map_table[i].shared_lib_path.find(memory_profiler_library) !=  string::npos){
							symbol_entry.address = (uint64_t)(memory_map_table[i].start_address + Get_symbol_address_from_ELF(memory_map_table[i].shared_lib_path,symbol_entry.name));
							break;
						}
					}

				} else {
					// Find the symbol from one of the dynamically linked shared library
					for(unsigned int i = 0; i < memory_map_table.size(); i++){
						// Do not check the program's own symbol table
						if(memory_map_table[i].shared_lib_path.compare(program_path) !=0){
							if(Find_symbol_in_ELF(memory_map_table[i].shared_lib_path,symbol_entry.name)){
									symbol_entry.address = (uint64_t)(memory_map_table[i].start_address + Get_symbol_address_from_ELF(memory_map_table[i].shared_lib_path,symbol_entry.name));
									break;
							}
						}
					}
				}
			}

			// Save the symbol with its absolute address in the vector
			function_symbol_table.emplace_back(symbol_entry);

			//cout << "symbol name: " << symbol_entry.name << " symbol address: " << std::hex << symbol_entry.address << endl;
		}
	}

	//cout << "address: " << &function_symbol_table << endl;
	/*cout << "this: " << this << endl;
	cout << "this->PID: " << std::dec<<this->PID << endl;*/
	//Sort the symbols based on address
	sort(function_symbol_table.begin(),function_symbol_table.end());

	for(auto element : function_symbol_table){

		cout << std::hex << element.address <<"  " << element.name << endl;
	}

	// Free the symbol table allocated in
	bfd_close(tmp_bfd);
	free(symbol_table);


	return true;
}

bool Process_handler::Read_virtual_memory_mapping() {


	ifstream mapping("/proc/" + this->PID_string + "/maps");
	string line;

	size_t pos_x;
	size_t pos_shared_lib;
	size_t pos_end_address_start;
	size_t pos_end_address_stop;


	memory_map_table_entry_struct_t entry;

	while (getline(mapping, line)) {
		try{

			// Check /proc/PID/maps for the line formats
			// Example of a line: 7f375e459000-7f375e614000 r-xp 00000000 08:01 398095                     /lib/x86_64-linux-gnu/libc-2.19.so

			// Absolute path of the shared lib starts with /
			pos_shared_lib = line.find("/");

			// Read lines only with x (executable) permission, I assume symbols are only put here
			// If permission x is not enabled, skip this line (until the path of shared lib, x could be appear only 1 place: at permissions))
			pos_x = line.find_first_of("x");
			if(pos_shared_lib < pos_x) {
				continue;
			}

			entry.shared_lib_path = line.substr(pos_shared_lib);

			// First "string" is the starting address, after '-' char the closing address starts
			pos_end_address_start = line.find_first_of("-");
			// After closing address an empty character can be found
			pos_end_address_stop = line.find_first_of(" ");

			// Convert the strings to numbers
			entry.start_address = stoul(line.substr(0,pos_end_address_start).c_str(),NULL,16);
			entry.end_address = stoul(line.substr(pos_end_address_start+1,pos_end_address_stop-pos_end_address_start).c_str(),NULL,16);

			memory_map_table.push_back(entry);

		}
		catch(exception &e){
			// TODO Analyze exception and return with false if necessary
		}
	}

	return true;
}

uint64_t Process_handler::Get_symbol_address_from_ELF(string ELF_path,string symbol_name){

	bfd* tmp_bfd;
	asymbol **symbol_table;
	long number_of_symbols;

	uint64_t address = 0;

	tmp_bfd = Open_ELF(ELF_path);
	if (!tmp_bfd) {
		cout<< "Error parsing ELF in Get_symbol_address_from_ELF"<<endl;
		return 0;
	}

	number_of_symbols = Parse_symbol_table_from_ELF(tmp_bfd,&symbol_table);
	if (number_of_symbols < 0) {
		cout<< "Error reading symbols from ELF in Parse_symbol_table_from_ELF"<<endl;
		return 0;
	}

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

	bfd_close(tmp_bfd);
	free(symbol_table);

	return address;
}

bool Process_handler::Find_symbol_in_ELF(string ELF_path, string symbol_name){

	bfd* tmp_bfd;
	asymbol **symbol_table;
	long number_of_symbols;

	tmp_bfd = Open_ELF(ELF_path);
	if(tmp_bfd == NULL){
		return false;
	}

	number_of_symbols = Parse_symbol_table_from_ELF(tmp_bfd,&symbol_table);
	if (number_of_symbols < 0) {
		return false;
	}

	for (int i = 0; i < number_of_symbols; i++) {
		if (symbol_table[i]->flags & BSF_FUNCTION) {
			string name(symbol_table[i]->name);
			// If a symbol with the same name is found we has to be sure it is not a symbol from a shared library linked to this shared lib
			// In this case value == 0
			if(symbol_name.compare(name) == 0 && symbol_table[i]->value > 0){
				free(symbol_table);
				bfd_close(tmp_bfd);
				return true;
			}
		}
	}

	bfd_close(tmp_bfd);
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

	// If this is the first profiling of a process create the shared memory, if the shared memory already exists (e.g: second profiling for a process), use it
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
