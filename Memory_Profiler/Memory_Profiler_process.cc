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
#include <iterator>

#include <inttypes.h>

using namespace std;

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
		cout << "Constructor, shared memory exists, shared memory handler: " << shared_memory <<" errno: " << errno << endl;

		// Map the shared memory because it exists
		// TODO: this mapping is unnecessary, will remap the shared memory again when needed
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
		cout << "Constructor, shared memory not exist, shared memory handler: " << shared_memory << " errno: " <<errno <<endl;
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
	all_function_symbol_table = obj.all_function_symbol_table;

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
			all_function_symbol_table = obj.all_function_symbol_table;
		}
		return *this;

}

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
 * Returns with the iterator containing the corresponding function name
 */
vector<symbol_table_entry_class>::iterator Process_handler::Find_function(uint64_t address){

	map<memory_map_table_entry_class const,vector<symbol_table_entry_class>,memory_map_table_entry_class_comp >::iterator it_VMA = Find_function_VMA(address);
	vector<symbol_table_entry_class>::iterator it = lower_bound(it_VMA->second.begin(),it_VMA->second.end(),address);

	if(!(it == function_symbol_table.begin())) it = it-1;

	//return it-1;
	return it;
}

/**
 * Returns with iterator to corresponding shared library where the address is located
 */
map<memory_map_table_entry_class const,vector<symbol_table_entry_class>,memory_map_table_entry_class_comp >::iterator Process_handler::Find_function_VMA(uint64_t address){

	map<memory_map_table_entry_class const,vector<symbol_table_entry_class>,memory_map_table_entry_class_comp >::iterator it;
	memory_map_table_entry_class tmp(0,address,"");

	it = all_function_symbol_table.upper_bound(tmp);
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


/**
 * Put local (defined) functions into tmp_function_symbol_table and calculates their absolute address
 */
bool Process_handler::Get_defined_symbols(asymbol ***symbol_table_param,long number_of_symbols,vector<symbol_table_entry_class> &tmp_function_symbol_table,unsigned long VMA_start_address){

	symbol_info *sym_inf = (symbol_info*)malloc(sizeof(symbol_info));
	symbol_table_entry_class symbol_entry;
	asymbol **symbol_table = *symbol_table_param;

	for (int i = 0; i < number_of_symbols; i++) {

		if (symbol_table[i]->flags & BSF_FUNCTION) {

			bfd_symbol_info(symbol_table[i],sym_inf);

			// Initialize name and address
			symbol_entry.name = symbol_table[i]->name;
			symbol_entry.address = 0;




			//if (symbol_table[i]->value != 0 && symbol_table[i]->section->vma != 0) {
			if(sym_inf->type == 't' || sym_inf->type == 'T'){
				cout << endl <<sym_inf->name<< ": " << std::hex <<sym_inf->value << endl;
				cout << "VMA: " << std::hex <<symbol_table[i]->section->vma << "  value: " << symbol_table[i]->value << "  START ADDR: " << VMA_start_address << endl;
				// Symbol is defined in the process, address is known from ELF
				symbol_entry.address = (uint64_t)(symbol_table[i]->section->vma + symbol_table[i]->value + VMA_start_address);
				// Save the symbol with its absolute address in the vector
				//function_symbol_table.emplace_back(symbol_entry);
				tmp_function_symbol_table.push_back(symbol_entry);
			}
		}
	}
	free(sym_inf);
	return true;
}

bool Process_handler::Create_symbol_table(){

	bfd* tmp_bfd;
	asymbol **symbol_table = 0;
	long number_of_symbols;

	// TODO This needs to be rethinked, define a const (linux MAX_PATH?) for it or a find dynamic way
	char program_path[1024];
	int len;

	vector<symbol_table_entry_class> tmp_function_symbol_table;
	map<memory_map_table_entry_class const,vector<symbol_table_entry_class>,memory_map_table_entry_class_comp >::iterator it;


	// Memory mappings are stored in memory_map_table after this
	if(Read_virtual_memory_mapping() == false){
		return false;
	}

	ofstream memory_map;
	memory_map.open(("Memory_map_"+ PID_string + ".txt").c_str(), ios::app);

	memory_map << "MEMORY MAP from /proc/ID/maps:" << endl;

	for(it = all_function_symbol_table.begin(); it != all_function_symbol_table.end(); it++){
		memory_map << "0x" << std::hex << it->first.start_address << "--" << "0x" << std::hex << it->first.end_address << "   " << it->first.path <<endl;
	}

	memory_map << endl << "Parsing symbols from:" << endl;
	// /proc/PID/exe is a sym link to the executable, read it with readlink because we need the full path of the executable
	if ((len = readlink(elf_path.c_str(), program_path, sizeof(program_path)-1)) != -1)
		program_path[len] = '\0';

	for (it = all_function_symbol_table.begin(); it != all_function_symbol_table.end(); it++) {

		// Read symbol table of the user application
		if(it->first.path == program_path){
			memory_map << it->first.path << endl;

			tmp_bfd = Open_ELF();
			if (!tmp_bfd) {
				return false;
			}

			number_of_symbols = Parse_symbol_table_from_ELF(tmp_bfd,&symbol_table);
			if (number_of_symbols < 0) {
				return false;
			}

			Get_defined_symbols(&symbol_table,number_of_symbols,tmp_function_symbol_table,0);

			//Sort the symbols based on address
			sort(tmp_function_symbol_table.begin(),tmp_function_symbol_table.end());

			it->second = tmp_function_symbol_table;
			tmp_function_symbol_table.clear();
		}
		else{
			memory_map << it->first.path << endl;
			tmp_bfd = Open_ELF(it->first.path);
			if (!tmp_bfd) {
				return false;
			}

			number_of_symbols = Parse_symbol_table_from_ELF(tmp_bfd,&symbol_table);
			if (number_of_symbols == 0) {
				number_of_symbols = Parse_dynamic_symbol_table_from_ELF(tmp_bfd,&symbol_table);
						if (number_of_symbols < 0) {
							return false;
						}
			}


			number_of_symbols = Parse_dynamic_symbol_table_from_ELF(tmp_bfd,&symbol_table);
			if (number_of_symbols < 0) {
				return false;
			}

			Get_defined_symbols(&symbol_table,number_of_symbols,tmp_function_symbol_table,it->first.start_address);

			//Sort the symbols based on address
			sort(tmp_function_symbol_table.begin(),tmp_function_symbol_table.end());

			it->second = tmp_function_symbol_table;
			tmp_function_symbol_table.clear();
		}
	}
	memory_map.close();

	bfd_close(tmp_bfd);
	free(symbol_table);

	vector<symbol_table_entry_class>::iterator it2;

	ofstream symbol_file;
	symbol_file.open(("Symbol_table_"+ PID_string + ".txt").c_str(), ios::out);

	for (it = all_function_symbol_table.begin(); it != all_function_symbol_table.end(); it++) {
		symbol_file << endl << endl << it->first.path << endl;
 		for(it2 = it->second.begin(); it2 != it->second.end(); it2++){
			symbol_file << it2->name << " ---- " << "0x" << std::hex << it2->address << endl;
		}
	}
	symbol_file.close();


	return true;
}

bool Process_handler::Read_virtual_memory_mapping() {

	ifstream mapping(("/proc/" + this->PID_string + "/maps").c_str(), std::ifstream::in);
	string line;

	string first_path;
	string current_path;

	map<memory_map_table_entry_class,vector<symbol_table_entry_class>,memory_map_table_entry_class_comp >::iterator it;

	size_t pos_x;
	size_t pos_shared_lib;
	size_t pos_end_address_start;
	size_t pos_end_address_stop;

	bool first_found = false;

	memory_map_table_entry_class entry;

	while (getline(mapping, line)) {
		try{

			// Check /proc/PID/maps for the line formats
			// Example of a line: 7f375e459000-7f375e614000 r-xp 00000000 08:01 398095                     /lib/x86_64-linux-gnu/libc-2.19.so

			// Absolute path of the shared lib starts with /
			pos_shared_lib = line.find("/");
			current_path = line.substr(pos_shared_lib);
			if(first_path != current_path){
				first_found = false;
			}

			// Read lines only with x (executable) permission, I assume symbols are only put here
			// If permission x is not enabled, skip this line (until the path of shared lib, x could be appear only 1 place: at permissions))
			pos_x = line.find_first_of("x");

			if(first_found == false){
				if(pos_shared_lib <= pos_x) {
					continue;
				}
				first_path = line.substr(pos_shared_lib);
				entry.path = first_path;
				// Convert the strings to numbers
				char * endptr;
				entry.start_address = strtoumax(line.substr(0,pos_end_address_start).c_str(),&endptr,16);
				entry.end_address = strtoumax(line.substr(pos_end_address_start+1,pos_end_address_stop-pos_end_address_start).c_str(),&endptr,16);

				//memory_map_table.push_back(entry);
				vector<symbol_table_entry_class> symbol_table;
				all_function_symbol_table.insert(pair< const memory_map_table_entry_class, vector<symbol_table_entry_class> >(memory_map_table_entry_class(entry.start_address,entry.end_address,entry.path),symbol_table));
				first_found = true;
			}
			else{

				// First "string" is the starting address, after '-' char the closing address starts
				pos_end_address_start = line.find_first_of("-");
				// After closing address an empty character can be found
				pos_end_address_stop = line.find_first_of(" ");

				if(first_path == current_path){
					for (it = all_function_symbol_table.begin(); it != all_function_symbol_table.end(); it++) {
						if(it->first.path == current_path){
							char * endptr;
							unsigned long start = it->first.start_address;
							unsigned long end = strtoumax(line.substr(pos_end_address_start+1,pos_end_address_stop-pos_end_address_start).c_str(),&endptr,16);
							string path = it->first.path;
							all_function_symbol_table.erase(it);

							vector<symbol_table_entry_class> symbol_table;
							all_function_symbol_table.insert(pair< const memory_map_table_entry_class, vector<symbol_table_entry_class> >(memory_map_table_entry_class(start,end,path),symbol_table));

						}
					}
				}
			}
		}
		catch(exception &e){
			// TODO Analyze exception and return with false if necessary
		}
	}

	return true;
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
	//cout << "size of shared memory: "<< new_size << endl;


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