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
#include <exception>
#include <stdexcept>
#include <time.h>

// int to string, this is c++98 compliant, other built in methods are not (C++0X is the minimum to use the built in functions)
#include <sstream>
#define SSTR( x ) static_cast< std::ostringstream & >( ( std::ostringstream() << std::dec << x ) ).str()



using namespace std;

string find_alloc_type(int type){
	switch (type) {
	case 1:
		return "malloc";
	case 2:
		return "free";
	case 3:
		return "calloc";
	case 4:
		return "realloc";
	default:
		return "unknown";
	}
}


void memory_profiler_sm_object_class_fix::write_header_to_file(string filename, unsigned long int total_log_count){

	ofstream headerfile;
	headerfile.exceptions ( ofstream::failbit | ofstream::badbit | ofstream::eofbit);

	try{
		headerfile.open(filename.c_str(), ios::binary | ios::trunc);
		headerfile.write((char*)&total_log_count, sizeof(total_log_count));
		headerfile.close();
	}
	catch(ofstream::failure &e) {
		throw e;
	}

}
void memory_profiler_sm_object_class_fix::write_entries_to_file(string filename){

	ofstream entriesfile;
	entriesfile.exceptions( ofstream::failbit | ofstream::badbit | ofstream::eofbit);

	try{
		entriesfile.open(filename.c_str(), ofstream::binary | ios::app);
		for(unsigned long i = 0; i < log_count; i++){
			log_entry[i].write_to_file(entriesfile);
		}
		entriesfile.close();
	}
	catch(ofstream::failure &e) {
		throw e;
	}

}

void memory_profiler_sm_object_class::read_header_from_file(string filename){

	ifstream headerfile;
	headerfile.exceptions( ifstream::failbit | ifstream::badbit | ifstream::eofbit);
	try{
		headerfile.open(filename.c_str(), ifstream::binary);
		headerfile.read((char*)&log_count, sizeof(log_count));
		headerfile.close();
	}
	catch(ifstream::failure &e){
		log_count = 0;
		throw e;
	}


}
void memory_profiler_sm_object_class::read_entries_from_file(string filename){

	ifstream entriesfile;
	entriesfile.exceptions( ifstream::failbit | ifstream::badbit | ifstream::eofbit);
	try{
		entriesfile.open(filename.c_str(), ifstream::binary);
		entriesfile.seekg(0,entriesfile.end);
		unsigned long int  length = entriesfile.tellg();
		entriesfile.seekg(0, entriesfile.beg);
		entriesfile.read((char*)log_entry, length);
		entriesfile.close();
	}
	catch(ifstream::failure &e){
		throw e;
	}
}


void memory_profiler_sm_object_class_fix::write_to_binary_file(string PID,unsigned long int total_log_count){

	try{
		write_header_to_file(PID+"_shm_header.bin",total_log_count);
		write_entries_to_file(PID+"_shm_entries.bin");
	}
	catch(ofstream::failure &e){
		throw e;
	}
}

void memory_profiler_sm_object_class::read_from_binary_file(string PID){

	try{
		read_header_from_file(PID + "_shm_header.bin");
		read_entries_from_file(PID + "_shm_entries.bin");
	}
	catch(ifstream::failure &e){
		throw e;
	}


}

/*
 *
 * entriesfile has to been opened before passing
 */
void memory_profiler_sm_object_log_entry_class::write_to_file(ofstream &entriesfile){

	try{
		entriesfile.write((char*)this,sizeof(*this));
	}
	catch (ofstream::failure &e) {
		cout <<"EXCEPTION write" << endl;
		throw e;
	}
}

void memory_profiler_sm_object_log_entry_class::Print(template_handler<Process_handler> process, ofstream &log_file) const{

	log_file << endl <<"PID: " << process.object->PID_string << endl;
	log_file <<"Thread ID: " << dec << thread_id << endl;
	char buffer[30];
	strftime(buffer,30,"%Y-%m-%d %T.",localtime(&(tval_before.tv_sec)));
	log_file <<"Localtime before: " << buffer << dec << tval_before.tv_usec << endl;
	strftime(buffer,30,"%Y-%m-%d %T.",gmtime(&(tval_before.tv_sec)));
	log_file <<"GMT before: " << buffer << dec << tval_before.tv_usec << endl;
	strftime(buffer,30,"%Y-%m-%d %T.",localtime(&(tval_after.tv_sec)));
	log_file <<"Localtime after: " << buffer << dec << tval_after.tv_usec << endl;
	strftime(buffer,30,"%Y-%m-%d %T.",gmtime(&(tval_after.tv_sec)));
	log_file <<"GMT after: " << buffer << dec << tval_after.tv_usec << endl;
	log_file <<"Call stack type: " << find_alloc_type(type) << endl;
	log_file <<"Address: 0x" << hex << address << endl;
	if(type == realloc_func){
		log_file <<"Realloc parameter address: 0x" << hex << realloc_address << endl;
	}
	log_file <<"Allocation size: " << dec << size << endl;
	log_file <<"Call stack size: " << dec << backtrace_length << endl;
	log_file <<"Call stack: " << endl;
	for(int  k=0; k < backtrace_length;k++){
		log_file << call_stack[k]<< " --- ";
		log_file << process.object->Find_function_name((uint64_t)call_stack[k]) << "  -----  ";
		log_file << process.object->Find_function_VMA((uint64_t)call_stack[k])->first.path << endl;
	}
}

void memory_profiler_sm_object_log_entry_class::Print(template_handler<Process_handler> process) const{

		cout << endl <<"PID: " << process.object->PID_string << endl;
		cout <<"Thread ID: " << dec << thread_id << endl;
		char buffer[30];
		strftime(buffer,30,"%Y-%m-%d %T.",localtime(&(tval_before.tv_sec)));
		cout <<"Localtime before: " << buffer << dec << tval_before.tv_usec << endl;
		strftime(buffer,30,"%Y-%m-%d %T.",gmtime(&(tval_before.tv_sec)));
		cout <<"GMT before: " << buffer << dec << tval_before.tv_usec << endl;
		strftime(buffer,30,"%Y-%m-%d %T.",localtime(&(tval_after.tv_sec)));
		cout <<"Localtime after: " << buffer << dec << tval_after.tv_usec << endl;
		strftime(buffer,30,"%Y-%m-%d %T.",gmtime(&(tval_after.tv_sec)));
		cout <<"GMT after: " << buffer << dec << tval_after.tv_usec << endl;
		cout <<"Call stack type: " << find_alloc_type(type) << endl;
		cout <<"Address: 0x" << hex << address << endl;
		if(type == realloc_func){
			cout <<"Realloc parameter address: 0x" << hex << realloc_address << endl;
		}
		cout <<"Allocation size: " << dec << size << endl;
		cout <<"Call stack size: " << dec << backtrace_length << endl;
		cout <<"Call stack: " << endl;
		for(int  k=0; k < backtrace_length;k++){
			cout << call_stack[k]<< " --- ";
			cout << process.object->Find_function_name((uint64_t)call_stack[k])<< "  -----  ";
			cout << process.object->Find_function_VMA((uint64_t)call_stack[k])->first.path << endl;
		}
}


Process_handler::Process_handler() {

	PID = 0;
	PID_string = "";
	PID_mapping = "";
	alive = false;
	profiled = false;
	memory_profiler_struct = 0;
	start_stop_semaphore_shared_memory = 0;
	start_stop_semaphore_name = "";
	start_stop_semaphore = 0;
	shared_memory_initialized = false;
	symbol_file_name = "";
	memory_map_file_name = "";
	shared_memory_file_name = "";

	shared_memory_A = 0;
	shared_memory_B = 0;
	shared_memory_name_A = "";
	shared_memory_name_B = "";
	memory_profiler_struct_A = 0;
	memory_profiler_struct_B = 0;

	total_entry_number = 0;
	header = "";
	entries = "";
}

Process_handler::Process_handler(pid_t PID) {

	this->PID = PID;
	PID_string = SSTR(PID);
	PID_mapping = "/proc/" + PID_string + "/maps";
	alive = true;
	profiled = false;
	memory_profiler_struct = 0;
	start_stop_semaphore_shared_memory = 0;
	start_stop_semaphore_name = PID_string + "_start_sem";
	start_stop_semaphore = 0;
	shared_memory_initialized = false;
	symbol_file_name = "Symbol_table_"+ PID_string + ".txt";
	memory_map_file_name = "Memory_map_"+ PID_string + ".txt";
	shared_memory_file_name = "Backtrace_"+ PID_string + ".txt";

	shared_memory_name_A = PID_string + "_mem_prof_A";
	shared_memory_name_B = PID_string + "_mem_prof_B";

	total_entry_number = 0;

	header = PID_string + "_header.bin";
	entries = PID_string + "_entries.bin";


	// If the profiled process compiled with START_PROF_IMM flag, that means it starts putting data into shared memory immediately after startup
	// In this case we have to map the memory area, set the profiled and initialized flags of the process true
	// Check whether the process starts profiling at the beginning with checking the existence of the corresponding shared memory area
	shared_memory_A = shm_open(shared_memory_name_A.c_str(),  O_RDWR , S_IRWXU | S_IRWXG | S_IRWXO);
	if (shared_memory_A >= 0){

		// Map the shared memory because it exists
		memory_profiler_struct_A = (memory_profiler_sm_object_class_fix*) mmap(
								NULL,
								sizeof(memory_profiler_sm_object_class_fix),
								PROT_READ,
								MAP_SHARED,
								shared_memory_A,
								0);



		if (memory_profiler_struct_A != MAP_FAILED) {

			shared_memory_B = shm_open(shared_memory_name_B.c_str(),  O_RDWR , S_IRWXU | S_IRWXG | S_IRWXO);
			if (shared_memory_B >= 0){

				// Map the shared memory because it exists
				memory_profiler_struct_B = (memory_profiler_sm_object_class_fix*) mmap(
										NULL,
										sizeof(memory_profiler_sm_object_class_fix),
										PROT_READ | PROT_WRITE,
										MAP_SHARED,
										shared_memory_B,
										0);
			}
			if (memory_profiler_struct_B == MAP_FAILED){
				cout << shared_memory_name_B << " shared memory mapping UNsuccesful, errno: " << dec << errno << endl;
				throw false;
			}

			// Shared memory initalized by the profiled process if both shared memory area can be mapped
			shared_memory_initialized = true;
			// If shared memory is already initialized until this point it means the process is being profiled at the moment
			// Shared memory area A contains correct profiling information
			profiled = memory_profiler_struct_A->profiled;

		}
		else {
			cout << shared_memory_name_A << " shared memory mapping UNsuccesful, errno: " << dec << errno << endl;
			throw false;
		}
	}

	if (!Create_symbol_table()) {
		cout << "Error creating the symbol table!" << endl;
		throw false;
	}

	if(!Init_start_stop_semaphore()){
		cout << "Error initializing the start/stop semaphore!" << endl;
		throw false;
	}

}

Process_handler::Process_handler(const Process_handler &obj){

	PID = obj.PID;
	PID_string = obj.PID_string;
	PID_mapping = obj.PID_mapping;
	profiled = obj.profiled;
	alive = obj.alive;
	memory_profiler_struct = obj.memory_profiler_struct;
	start_stop_semaphore_shared_memory = obj.start_stop_semaphore_shared_memory;
	start_stop_semaphore_name = obj.start_stop_semaphore_name;
	start_stop_semaphore = obj.start_stop_semaphore;
	shared_memory_initialized = obj.shared_memory_initialized;
	all_function_symbol_table = obj.all_function_symbol_table;
	symbol_file_name = obj.symbol_file_name;
	memory_map_file_name = obj.memory_map_file_name;
	shared_memory_file_name = obj.shared_memory_file_name;

    shared_memory_A = obj.shared_memory_A;
    shared_memory_B = obj.shared_memory_B;
    shared_memory_name_A = obj.shared_memory_name_A;
    shared_memory_name_B = obj.shared_memory_name_B;
    memory_profiler_struct_A = obj.memory_profiler_struct_A;
    memory_profiler_struct_B = obj.memory_profiler_struct_B;

    total_entry_number = obj.total_entry_number;
    header = obj.header;
	entries = obj.entries;
}
Process_handler& Process_handler::operator=(const Process_handler &obj){
	if (this != &obj) {

			PID = obj.PID;
			PID_string = obj.PID_string;
			PID_mapping = obj.PID_mapping;
			profiled = obj.profiled;
			alive = obj.alive;
			memory_profiler_struct = obj.memory_profiler_struct;
			start_stop_semaphore_shared_memory = obj.start_stop_semaphore_shared_memory;
			start_stop_semaphore = obj.start_stop_semaphore;
			start_stop_semaphore_name = obj.start_stop_semaphore_name;
			shared_memory_initialized = obj.shared_memory_initialized;
			all_function_symbol_table = obj.all_function_symbol_table;
			symbol_file_name = obj.symbol_file_name;
			memory_map_file_name = obj.memory_map_file_name;
			shared_memory_file_name = obj.shared_memory_file_name;

		    shared_memory_A = obj.shared_memory_A;
		    shared_memory_B = obj.shared_memory_B;
		    shared_memory_name_A = obj.shared_memory_name_A;
		    shared_memory_name_B = obj.shared_memory_name_B;
		    memory_profiler_struct_A = obj.memory_profiler_struct_A;
		    memory_profiler_struct_B = obj.memory_profiler_struct_B;

		    total_entry_number = obj.total_entry_number;

		    header = obj.header;
			entries = obj.entries;
	}
		return *this;
}

Process_handler::~Process_handler() {

	all_function_symbol_table.clear();

	munmap(start_stop_semaphore, sizeof(sem_t));

	if(shared_memory_initialized){
		munmap(memory_profiler_struct_A,sizeof(memory_profiler_sm_object_class_fix));
		munmap(memory_profiler_struct_B,sizeof(memory_profiler_sm_object_class_fix));
	}

	if(alive == false){
		sem_destroy(start_stop_semaphore);
		shm_unlink(start_stop_semaphore_name.c_str());
		shm_unlink(shared_memory_name_A.c_str());
		shm_unlink(shared_memory_name_B.c_str());
	}
}


/**
 * Returns with iterator to corresponding shared library where the address is located
 */
 map<memory_map_table_entry_class,vector<symbol_table_entry_class>,memory_map_table_entry_class_comp >::const_iterator Process_handler::Find_function_VMA (const uint64_t address) const{

	memory_map_table_entry_class tmp(0,address,"");
	map<memory_map_table_entry_class,vector<symbol_table_entry_class>,memory_map_table_entry_class_comp >::const_iterator it = all_function_symbol_table.upper_bound(tmp);

	return it;
}

const string Process_handler::Find_function_name(const uint64_t address) const{

	map<memory_map_table_entry_class,vector<symbol_table_entry_class>,memory_map_table_entry_class_comp >::const_iterator it_VMA = Find_function_VMA(address);

	vector<symbol_table_entry_class>::const_iterator it = upper_bound(it_VMA->second.begin(),it_VMA->second.end(),address);


	//Needed because of upper bound (points to next element)
	if(it != it_VMA->second.begin()){
			it = it-1;

			// Some cases from libc a version with __ name and version without __ exists with the same location
			// With upper bound we will find the one which is later, but better to print out version without __ characters at the beginning
			if(it->address == (it-1)->address){
				if((it)->name.find_first_of("_") == 0){
					it = it-1;
				}
			}
	}

	if(it == it_VMA->second.end()){
		return "Symbol has not been found for this address";
	}
	else{
		return it->name;
	}
}


/*
 * Need to close bfd manually after this function !!!
 */
bfd* Process_handler::Open_ELF(string ELF_path) const{

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
void Process_handler::Get_defined_symbols(asymbol ***symbol_table_param,long number_of_symbols,vector<symbol_table_entry_class> &tmp_function_symbol_table,unsigned long VMA_start_address){

	symbol_info *sym_inf = (symbol_info*)malloc(sizeof(symbol_info));
	symbol_table_entry_class symbol_entry;
	asymbol **symbol_table = *symbol_table_param;

	for (int i = 0; i < number_of_symbols; i++) {

			bfd_symbol_info(symbol_table[i],sym_inf);

			//Skip symbols without names
			if (sym_inf->name == NULL || *sym_inf->name == '\0'){
				continue;
			}
			// Skip Symbols where value is 0 (they are undefined symbols in this ELF)
			if (symbol_table[i]->value == 0){
				    continue;
			}

			// Initialize name and address
			symbol_entry.name = symbol_table[i]->name;
			symbol_entry.address = 0;

			// Symbol is defined in the process, address is known from ELF
			if(symbol_table[i]->section->vma + symbol_table[i]->value < VMA_start_address){
				symbol_entry.address = (uint64_t)(symbol_table[i]->section->vma + symbol_table[i]->value + VMA_start_address);
			}
			else {
			//bfd_values_file << "Interpreting values as absolute..."<< endl;
				symbol_entry.address = (uint64_t)(symbol_table[i]->section->vma + symbol_table[i]->value);
			}
			// Save the symbol with its absolute address in the vector

			tmp_function_symbol_table.push_back(symbol_entry);
	}
	free(sym_inf);
}

bool Process_handler::Create_symbol_table(){

	bfd* tmp_bfd;
	asymbol **symbol_table = 0;
	long number_of_symbols;

	// TODO This needs to be rethinked, define a const (linux MAX_PATH?) for it or a find dynamic way
	char program_path[5000];
	int len;

	vector<symbol_table_entry_class> tmp_function_symbol_table;
	map<memory_map_table_entry_class const,vector<symbol_table_entry_class>,memory_map_table_entry_class_comp >::iterator it;


	// Memory mappings are stored in memory_map_table after this
	if(Read_virtual_memory_mapping() == false){
		return false;
	}

	for (it = all_function_symbol_table.begin(); it != all_function_symbol_table.end(); it++) {

			tmp_bfd = Open_ELF(it->first.path);
			if (!tmp_bfd) {
				return false;
			}

			number_of_symbols = Parse_symbol_table_from_ELF(tmp_bfd,&symbol_table);
			if (number_of_symbols <= 0) {
				//This means the ELF is stripped, reading the dynamic section
				free(symbol_table);
				number_of_symbols = Parse_dynamic_symbol_table_from_ELF(tmp_bfd,&symbol_table);
						if (number_of_symbols < 0) {
							free(symbol_table);
							cout << " Error while parsing symbol and dynamic symbol table from ELF: " << it->first.path << endl;
							return false;
						}
			}

			Get_defined_symbols(&symbol_table,number_of_symbols,tmp_function_symbol_table,it->first.start_address);

			//Sort the symbols based on address
			sort(tmp_function_symbol_table.begin(),tmp_function_symbol_table.end());

			it->second = tmp_function_symbol_table;
			tmp_function_symbol_table.clear();
			free(symbol_table);
			bfd_close(tmp_bfd);
	}

	return true;
}

bool Process_handler::Read_virtual_memory_mapping() {

	ifstream mapping(PID_mapping.c_str(), std::ifstream::in);
	string line = "";

	string first_path = "";
	string current_path = "";

	map<memory_map_table_entry_class,vector<symbol_table_entry_class>,memory_map_table_entry_class_comp >::iterator it;

	size_t pos_x = 0;
	size_t pos_shared_lib = 0;
	size_t pos_end_address_start = 0;
	size_t pos_end_address_stop = 0;

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
		catch(const std::out_of_range& oor){
			// Thrown by substring when no "/" character is in the line, drop that line = do nothing

			}
		catch(const exception & e){
			//Any other exceptions
			cout << "Exception in Read_virtual_memory_mapping function: " << e.what() << endl;
		}
	}

	if(all_function_symbol_table.size() == 0){

		cout << "Failed reading the memory mapping..." << endl;
		return false;
	}

	return true;
}


bool Process_handler::Init_start_stop_semaphore() {

	start_stop_semaphore_shared_memory = shm_open(start_stop_semaphore_name.c_str(), O_CREAT | O_RDWR | O_EXCL, S_IRWXU | S_IRWXG | S_IRWXO);
	if (start_stop_semaphore_shared_memory < 0){
		if (errno == EEXIST){
			start_stop_semaphore_shared_memory = shm_open(start_stop_semaphore_name.c_str(),  O_RDWR , S_IRWXU | S_IRWXG | S_IRWXO);
			if(start_stop_semaphore_shared_memory <= 0){
				cout << "Error while opening start_stop_semaphore shared memory: " << dec << errno << endl;
				return false;
			}
		}
		else {
			cout << "Error while opening start_stop_semaphore shared memory: " << dec << errno << endl;
		return false;
		}
	}

	start_stop_semaphore = (sem_t*) mmap(NULL, sizeof(sem_t), PROT_WRITE, MAP_SHARED,start_stop_semaphore_shared_memory, 0);
	if (start_stop_semaphore == MAP_FAILED){
		cout << "Failed mapping the start_stop_semaphore shared memory: " << dec << errno << endl;
		return false;
	}

	return true;
}

bool Process_handler::Init_shared_memory() {

	// If this is the first profiling of a process create the shared memory, if the shared memory already exists just open it
	shared_memory_A = shm_open(shared_memory_name_A.c_str(), O_CREAT | O_RDWR | O_EXCL, S_IRWXU | S_IRWXG | S_IRWXO);

	if (shared_memory_A < 0) {
		if (errno == EEXIST){
			shared_memory_A = shm_open(shared_memory_name_A.c_str(),  O_RDWR , S_IRWXU | S_IRWXG | S_IRWXO);
			if(shared_memory_A <= 0){
				cout << "Failed opening the shared memory for Process: " << dec << PID << " errno: " << dec << errno << endl;
				return false;
			}
		}
		else {
			cout << "Error while creating shared memory! errno: " << errno << endl;
			return false;
		}
	}

	// Truncate and map the shared memory if it has not existed before
	int err = ftruncate(shared_memory_A, sizeof(memory_profiler_sm_object_class_fix));
	if (err < 0){
		cout << "Error while truncating shared memory: " << errno << endl;
		return false;
	}

	memory_profiler_struct_A = (memory_profiler_sm_object_class_fix*) mmap(
				NULL,
				sizeof(memory_profiler_sm_object_class_fix),
				PROT_READ,
				MAP_SHARED,
				shared_memory_A,
				0);
	if (memory_profiler_struct_A == MAP_FAILED) {
		cout << "Failed mapping the shared memory: " << errno << endl;
		return false;
	}

	// If this is the first profiling of a process create the shared memory, if the shared memory already exists just open it
	shared_memory_B = shm_open(shared_memory_name_B.c_str(), O_CREAT | O_RDWR | O_EXCL, S_IRWXU | S_IRWXG | S_IRWXO);

	if (shared_memory_B < 0) {
		if (errno == EEXIST){
			shared_memory_B = shm_open(shared_memory_name_B.c_str(),  O_RDWR , S_IRWXU | S_IRWXG | S_IRWXO);
			if(shared_memory_B <= 0){
				cout << "Failed opening the shared memory for Process: " << dec << PID << " errno: " << dec << errno << endl;
				return false;
			}
		}
		else {
			cout << "Error while creating shared memory! errno: " << errno << endl;
			return false;
		}
	}

	// Truncate and map the shared memory if it has not existed before
	err = ftruncate(shared_memory_B, sizeof(memory_profiler_sm_object_class_fix));
	if (err < 0){
		cout << "Error while truncating shared memory: " << errno << endl;
		return false;
	}

	memory_profiler_struct_B = (memory_profiler_sm_object_class_fix*) mmap(
				NULL,
				sizeof(memory_profiler_sm_object_class_fix),
				PROT_READ | PROT_WRITE,
				MAP_SHARED,
				shared_memory_B,
				0);
	if (memory_profiler_struct_B == MAP_FAILED) {
		cout << "Failed mapping the shared memory: " << errno << endl;
		return false;
	}



	shared_memory_initialized = true;
	return true;
}


void Process_handler::Start_Stop_profiling() const {

	sem_post(start_stop_semaphore);
}

void Process_handler::Read_shared_memory(){

	ifstream headerfile;
	headerfile.exceptions( ifstream::failbit | ifstream::badbit | ifstream::eofbit);
	unsigned long int count = 0;

	if(!shared_memory_initialized || profiled){
		cout << "Process " << PID_string << " is under profiling and/or shared memory has not been initialized." << endl;
	}
	cout << "Reading binary file..." << endl;
	try{

		headerfile.open((PID_string + "_shm_header.bin").c_str(), ios::binary | ios::in);
		headerfile.read((char*)&count, sizeof(count));
		headerfile.close();

		memory_profiler_struct = new memory_profiler_sm_object_class(count);
		memory_profiler_struct->read_from_binary_file(PID_string);
	}
	catch(ifstream::failure &e){
		delete memory_profiler_struct;
		throw e;
	}
	cout << "Reading binary file finished successfully!" << endl;
}

