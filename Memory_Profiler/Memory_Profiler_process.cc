#include "Memory_Profiler_process.h"

#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fstream>
#include <sstream>
#include <regex.h>
#include <stdlib.h>
#include <unistd.h>
#include <algorithm>
#include <string>

#include <inttypes.h>





using namespace std;

//#define Memory_profiler_shared_library "libMemory_Profiler_shared_library.so"
//#define Memory_profiler_IMM_shared_library "libMemory_Profiler_IMM_shared_library.so"


Process_handler::Process_handler() {

	PID = 0;
	PID_string = "";
	alive = false;
	profiled = false;
	memory_profiler_struct = 0;
	shared_memory = 0;
	semaphore_shared_memory = 0;
	semaphore = 0;
	elf_path = "";
	shared_memory_initialized = false;

}

Process_handler::Process_handler(pid_t PID) {

	this->PID = PID;
	//PID_string = to_string(PID);
	PID_string = SSTR(PID);
	alive = true;
	profiled = false;
	memory_profiler_struct = 0;
	shared_memory = 0;
	semaphore_shared_memory = 0;
	semaphore = 0;
	elf_path = "/proc/" + this->PID_string + "/exe";
	shared_memory_initialized = false;

	// If the profiled process compiled with START_PROF_IMM flag, that means it starts putting data into shared memory immediately after startup
	// In this case we have to map the memory area, set the profiled and initialized flags of the process true
	// Check whether the process starts profiling at the beginning with checking the existence of the corresponding shared memory area
	shared_memory = shm_open(("/" + PID_string).c_str(),  O_RDWR , S_IRWXU | S_IRWXG | S_IRWXO);
	if (shared_memory >= 0){
		cout << "Constructor, shared memory exists, shared memory handler: " << shared_memory /*<<" errno: " <<errno*/ << endl;

		// Map the shared memory because it exists
		memory_profiler_struct = (memory_profiler_sm_object_class*) mmap(
								NULL,
								sizeof(memory_profiler_sm_object_class),
								PROT_READ,
								MAP_SHARED,
								shared_memory,
								0);
		if (memory_profiler_struct != MAP_FAILED) {
			// Shared memory initalized by the profiled process
			shared_memory_initialized = true;
			// If shared memory is already initialized until this point it means the process is being profiled at the moment
			profiled = true;
		}
	}
	else {
		cout << "Constructor, shared memory not exist, shared memory handler: " << shared_memory /*<< " errno: " <<errno*/ <<endl;
	}

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
	shared_memory_initialized = obj.shared_memory_initialized;

	semaphore_shared_memory = obj.semaphore_shared_memory;
	semaphore = obj.semaphore;

	elf_path = obj.elf_path;


	memory_map_table = obj.memory_map_table;
	function_symbol_table = obj.function_symbol_table;

}

Process_handler& Process_handler::operator=(const Process_handler &obj){
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
			shared_memory_initialized = obj.shared_memory_initialized;
		}
		return *this;

}
/*
Process_handler::Process_handler(Process_handler &&obj){

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
	shared_memory_initialized = obj.shared_memory_initialized;

	obj.PID = 0;
	obj.PID_string = "";
	obj.profiled = false;
	obj.alive = false;
	obj.memory_profiler_struct = 0;
	obj.shared_memory = 0;
	obj.semaphore_shared_memory = 0;
	obj.semaphore = 0;
	obj.elf_path = "";
	obj.memory_map_table.clear();
	obj.function_symbol_table.clear();
	obj.shared_memory_initialized = false;
}
*/
/*Process_handler& Process_handler::operator=(Process_handler&& obj){

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
		shared_memory_initialized = obj.shared_memory_initialized;

		obj.PID = 0;
		obj.PID_string = "";
		obj.profiled = false;
		obj.alive = false;
		obj.memory_profiler_struct = 0;
		obj.shared_memory = 0;
		obj.semaphore_shared_memory = 0;
		obj.semaphore = 0;
		obj.elf_path = "";
		obj.function_symbol_table.clear();
		obj.memory_map_table.clear();
		obj.shared_memory_initialized = false;
	}
	return *this;
}
*/

Process_handler::~Process_handler() {

	memory_map_table.clear();
	function_symbol_table.clear();

	//munmap(semaphore, sizeof(sem_t));
	//munmap(memory_profiler_struct, sizeof(memory_profiler_sm_object_log_entry_class));

	//shm_unlink(("/" + PID_string).c_str());
}

void Process_handler::Process_delete(){

	memory_map_table.clear();
	function_symbol_table.clear();

	munmap(semaphore, sizeof(sem_t));
	munmap(memory_profiler_struct, sizeof(memory_profiler_sm_object_log_entry_class));

	shm_unlink(("/" + PID_string).c_str());
}


/**
 * Returns with the index of function in vector
 */
vector<symbol_table_entry_class>::iterator Process_handler::Find_function(uint64_t address){

	vector<symbol_table_entry_class>::iterator it = lower_bound(function_symbol_table.begin(),function_symbol_table.end(),address);

	if(!(it == function_symbol_table.begin())) it = it-1;

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
			cout << "Error opening Process's ELF file: " << ELF_path << endl;
			return NULL;
		}

		//check if the file is in format
		if (!bfd_check_format(tmp_bfd, bfd_object)) {
			if (bfd_get_error() != bfd_error_file_ambiguously_recognized) {
				cout << "Incompatible FILE format, not an ELF: " << ELF_path<< endl;
				return NULL;
			}
		}

		return tmp_bfd;
}


long Process_handler::Parse_symbol_table_from_ELF(bfd* bfd_ptr,asymbol ***symbol_table){

	long storage_needed;
	long number_of_symbols;

	storage_needed = bfd_get_symtab_upper_bound(bfd_ptr);
	*symbol_table = (asymbol**) malloc(storage_needed);
	number_of_symbols = bfd_canonicalize_symtab(bfd_ptr, *symbol_table);

	return number_of_symbols;
}

/*
 * Need to free symbol_table manually after this function !!!
 */
long Process_handler::Parse_dynamic_symbol_table_from_ELF(bfd* bfd_ptr,asymbol ***symbol_table){

	long storage_needed;
	long number_of_symbols;

	storage_needed = bfd_get_dynamic_symtab_upper_bound(bfd_ptr);
	*symbol_table = (asymbol**) malloc(storage_needed);
	number_of_symbols = bfd_canonicalize_dynamic_symtab(bfd_ptr, *symbol_table);

	return number_of_symbols;
}

string Process_handler::Find_memory_profiler_library_name(){

	/*for(auto elem : memory_map_table){
		if(elem.shared_lib_path.find(Memory_profiler_shared_library) != string::npos){
			return Memory_profiler_shared_library;
		}
		else if(elem.shared_lib_path.find(Memory_profiler_IMM_shared_library) != string::npos){
			 return Memory_profiler_IMM_shared_library;
		}
	}
	return "";*/

	vector<memory_map_table_entry_class>::iterator it;
	string Memory_profiler_shared_library = "libMemory_Profiler_shared_library.so";
	string Memory_profiler_IMM_shared_library = "libMemory_Profiler_IMM_shared_library.so";

	for (it = memory_map_table.begin(); it != memory_map_table.end(); it++) {
		if(it->shared_lib_path.find(Memory_profiler_shared_library) != string::npos){
			return Memory_profiler_shared_library;
		}
		else if(it->shared_lib_path.find(Memory_profiler_IMM_shared_library) != string::npos){
			 return Memory_profiler_IMM_shared_library;
		}
	}
	return "";
}



bool Process_handler::Create_symbol_table() {

	bfd* tmp_bfd;
	asymbol **symbol_table = 0;
	long number_of_symbols;

	symbol_table_entry_class symbol_entry;

	// TODO This needs to be rethinked, define a const (linux MAX_PATH?) for it or a find dynamic way
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


	string memory_profiler_library = Find_memory_profiler_library_name();

	// /proc/PID/exe is a sym link to the executable, read it with readlink because we need the full path of the executable
	if ((len = readlink(elf_path.c_str(), program_path, sizeof(program_path)-1)) != -1)
		program_path[len] = '\0';


	// Get malloc from memory_profiler_shared_library
	// Initialize name and address
	symbol_entry.name = "malloc";
	symbol_entry.address = 0;
	for(unsigned int i = 0; i < memory_map_table.size(); i++){

		//Find the entry of the memory_profiler_shared_library, read its VM base address, and find malloc/free address offset from it
		if(memory_map_table[i].shared_lib_path.find(memory_profiler_library) !=  string::npos){
			symbol_entry.address = (uint64_t)(memory_map_table[i].start_address + Get_symbol_address_from_ELF(memory_map_table[i].shared_lib_path,symbol_entry.name));
			break;
		}
	}
	// Save the symbol with its absolute address in the vector
	//function_symbol_table.emplace_back(symbol_entry);
	function_symbol_table.push_back(symbol_entry);

	// Get free from memory_profiler_shared_library
	// Initialize name and address
	symbol_entry.name = "free";
	symbol_entry.address = 0;
	for(unsigned int i = 0; i < memory_map_table.size(); i++){
			//Find the entry of the memory_profiler_shared_library, read its VM base address, and find malloc/free address offset from it
			if(memory_map_table[i].shared_lib_path.find(memory_profiler_library) !=  string::npos){
				symbol_entry.address = (uint64_t)(memory_map_table[i].start_address + Get_symbol_address_from_ELF(memory_map_table[i].shared_lib_path,symbol_entry.name));
				break;
			}
	}
	// Save the symbol with its absolute address in the vector
	//function_symbol_table.emplace_back(symbol_entry);
	function_symbol_table.push_back(symbol_entry);


	for (int i = 0; i < number_of_symbols; i++) {

		if (symbol_table[i]->flags & BSF_FUNCTION) {

			// Initialize name and address
			symbol_entry.name = symbol_table[i]->name;
			symbol_entry.address = 0;

			if (symbol_table[i]->value != 0 && symbol_table[i]->section->vma != 0) {
				// Symbol is defined in the process, address is known from ELF
				symbol_entry.address = (uint64_t)(symbol_table[i]->section->vma + symbol_table[i]->value);
				// Save the symbol with its absolute address in the vector
				//function_symbol_table.emplace_back(symbol_entry);
				function_symbol_table.push_back(symbol_entry);
			} else {
				// Malloc and free has been already saved
				if (symbol_entry.name == "malloc" || symbol_entry.name == "free") {

					continue;

				} else {
					// Find the symbol from one of the dynamically linked shared library
					for(unsigned int i = 0; i < memory_map_table.size(); i++){
						// Do not check the program's own symbol table
						if(memory_map_table[i].shared_lib_path.compare(program_path) !=0){

							// Reading the symbols from .symtab contains @@LIB string in their name referring to the shared library
							// Reading symbols from .dynsym only contains symbol name without @@LIB substring, need to strip it
							// TODO: readelf -s somehow reads dynamic section with symbols containing the @@LIB substring
							size_t posisition = symbol_entry.name.find_first_of("@");
							string stripped_func_name = symbol_entry.name.substr(0,posisition);

							if(Find_symbol_in_ELF(memory_map_table[i].shared_lib_path,stripped_func_name)){
									symbol_entry.address = (uint64_t)(memory_map_table[i].start_address + Get_symbol_address_from_ELF(memory_map_table[i].shared_lib_path,stripped_func_name));
									// Save the symbol with its absolute address in the vector
									//function_symbol_table.emplace_back(symbol_entry);
									function_symbol_table.push_back(symbol_entry);
									break;
							}
						}
					}
				}
			}
		}
	}

	//Sort the symbols based on address
	sort(function_symbol_table.begin(),function_symbol_table.end());

	vector<symbol_table_entry_class>::iterator it;

		for (it = function_symbol_table.begin(); it != function_symbol_table.end(); it++){

		cout << std::hex << it->address <<"  " << it->name << endl;
	}

	// Free the symbol table allocated in
	bfd_close(tmp_bfd);
	free(symbol_table);


	return true;
}

bool Process_handler::Read_virtual_memory_mapping() {

	ifstream mapping(("/proc/" + this->PID_string + "/maps").c_str(), std::ifstream::in);
	string line;

	size_t pos_x;
	size_t pos_shared_lib;
	size_t pos_end_address_start;
	size_t pos_end_address_stop;


	memory_map_table_entry_class entry;


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
			char * endptr;
			entry.start_address = strtoumax(line.substr(0,pos_end_address_start).c_str(),&endptr,16);
			entry.end_address = strtoumax(line.substr(pos_end_address_start+1,pos_end_address_stop-pos_end_address_start).c_str(),&endptr,16);

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

	number_of_symbols = Parse_dynamic_symbol_table_from_ELF(tmp_bfd,&symbol_table);
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

	number_of_symbols = Parse_dynamic_symbol_table_from_ELF(tmp_bfd,&symbol_table);
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

	semaphore_shared_memory = shm_open(
			("/" + PID_string + "_start_sem").c_str(), O_RDWR,
			S_IRWXU | S_IRWXG | S_IRWXO);
	if (semaphore_shared_memory < 0) cout << "Error while opening semaphore shared memory: " << errno << endl;

	semaphore = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_WRITE, MAP_SHARED,
			semaphore_shared_memory, 0);
	if (semaphore == MAP_FAILED) cout << "Failed mapping the semaphore shared memory: " << errno << endl;

}

bool Process_handler::Init_shared_memory() {

	// If this is the first profiling of a process create the shared memory, if the shared memory already exists just open it
	shared_memory = shm_open(("/" + PID_string).c_str(), O_CREAT | O_RDWR | O_EXCL, S_IRWXU | S_IRWXG | S_IRWXO);

	if (shared_memory < 0) {
		cout << "Error while creating shared memory! errno: " << errno << endl;
		if (errno == EEXIST){
			cout << "Shared memory already exists, do not re-create it!" << errno << endl;
		}
		return false;
	}

	// Truncate and map the shared memory if it has not existed before
	cout << "Creating shared memory, ID: " << "/" + PID_string << endl;
	int err = ftruncate(shared_memory, sizeof(memory_profiler_sm_object_class));
	if (err < 0){
		cout << "Error while truncating shared memory: " << errno << endl;
		return false;
	}

	memory_profiler_struct = (memory_profiler_sm_object_class*) mmap(
				NULL,
				sizeof(memory_profiler_sm_object_class),
				PROT_READ,
				MAP_SHARED,
				shared_memory,
				0);
	if (memory_profiler_struct == MAP_FAILED) {
		cout << "Failed mapping the shared memory: " << errno << endl;
		return false;
	}

	shared_memory_initialized = true;
	return true;
}

bool Process_handler::Remap_shared_memory(){

	if(shared_memory_initialized == false){
		cout<<"Shared memory has not been initialized yet, cannot remap it!" << endl;
		return false;
	}

	// Unmap it first to prevent too much mapping
	/*int err = munmap(memory_profiler_struct, sizeof(memory_profiler_sm_object_class)+(memory_profiler_struct->log_count) * sizeof(memory_profiler_sm_object_log_entry_class));
	if(err < 0){
		cout << "Failed unmapping it first, errno: " << errno << endl;
		return false;
	}*/


	unsigned long log_count = memory_profiler_struct->log_count;
	unsigned long new_size = sizeof(memory_profiler_sm_object_class) + log_count*sizeof(memory_profiler_sm_object_log_entry_class);
	cout << "size of shared memory: "<< new_size << endl;


	this->memory_profiler_struct = (memory_profiler_sm_object_class*) mmap(
							NULL,
							new_size,
							PROT_READ,
							MAP_SHARED,
							shared_memory,
							0);
	if (memory_profiler_struct == MAP_FAILED) {
		cout << "Failed re-mapping the shared memory: " << errno << endl;
		return false;
	}

	return true;

}

void Process_handler::Start_Stop_profiling() {

	sem_post(semaphore);
}

memory_profiler_sm_object_class* Process_handler::Get_shared_memory() {

	return memory_profiler_struct;
}
