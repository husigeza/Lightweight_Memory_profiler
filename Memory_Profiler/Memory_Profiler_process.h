/*
 * Process.h
 *
 *  Created on: Nov 2, 2015
 *      Author: egezhus
 */

#ifndef MEMORY_PROFILER_PROCESS_H_
#define MEMORY_PROFILER_PROCESS_H_

#ifdef bfd_need_config_h
#include "config.h"
#endif

#include <stdint.h>
#include <semaphore.h>
#include <vector>
#include <map>
#include <time.h>
#include <fstream>

#include "bfd.h"


#include "Memory_Profiler_memory_map.h"
#include "Memory_Profiler_symbol_table.h"

#include <sstream>
#define SSTR( x ) dynamic_cast< std::ostringstream & >( std::ostringstream() << std::dec << x ).str()


enum {
	malloc_func = 1,
	free_func = 2
};

using namespace std;

#define max_call_stack_depth 100

class Process_handler;


class memory_profiler_sm_object_log_entry_class{

public:
	memory_profiler_sm_object_log_entry_class() {
		thread_id = 0;
		tval_before.tv_sec = 0;
		tval_before.tv_usec = 0;
		tval_after.tv_sec = 0;
		tval_after.tv_usec = 0;
		type = 0;
		size = 0;
		backtrace_length = 0;
		//call_stack = nullptr;
		address = 0;
		valid = false;
	}

	pthread_t thread_id;
	struct timeval tval_before;
	struct timeval tval_after;
	int type; //malloc = 1, free = 2
	size_t  size; // in case of malloc
	int backtrace_length;
	void *call_stack[max_call_stack_depth];
	uint64_t address;
	bool valid;

	void Print(Process_handler *process, ofstream &log_file) const;
	void Print(Process_handler *process) const;

};

class memory_profiler_sm_object_class {

public:

	long unsigned int log_count; // Always has a bigger value with 1 than the real element number
	bool profiled;
	memory_profiler_sm_object_log_entry_class log_entry[1];
};

struct memory_map_table_entry_class_comp {
  bool operator() (const memory_map_table_entry_class & memory_map_A, const memory_map_table_entry_class & memory_map_B) const
  {return memory_map_A.end_address < memory_map_B.end_address;}
};

class Process_handler {

    pid_t PID;
    string PID_mapping;

    bool profiled;
    bool alive;

    int shared_memory;
    string shared_memory_name;
    bool shared_memory_initialized;
    memory_profiler_sm_object_class *memory_profiler_struct;
    unsigned long mapped_size_of_shared_memory;

    int start_stop_semaphore_shared_memory;
    string start_stop_semaphore_name;
    sem_t* start_stop_semaphore;

    string elf_path;

    // This container stores the local symbols and symbols from shared libraries
    map<memory_map_table_entry_class,vector<symbol_table_entry_class>, memory_map_table_entry_class_comp> all_function_symbol_table;

    bfd* Open_ELF() const;
    bfd* Open_ELF(string ELF_path) const;

    long Parse_dynamic_symbol_table_from_ELF(bfd* bfd_ptr,asymbol ***symbol_table);
    long Parse_symbol_table_from_ELF(bfd* bfd_ptr,asymbol ***symbol_table);

    void Get_defined_symbols(asymbol ***symbol_table_param,long number_of_symbols,vector<symbol_table_entry_class> &tmp_function_symbol_table,unsigned long VMA_start_address);
    bool Create_symbol_table();
    bool Read_virtual_memory_mapping();

    map<memory_map_table_entry_class,vector<symbol_table_entry_class>,memory_map_table_entry_class_comp >::const_iterator Find_function_VMA (const uint64_t address) const;

    bool Init_start_stop_semaphore();

    ofstream symbol_file;
    string symbol_file_name;
    ofstream memory_map_file;
	string memory_map_file_name;
    ofstream shared_memory_file;
	string shared_memory_file_name;

    public:

    	Process_handler();
        Process_handler(pid_t PID);
        ~Process_handler();

        string PID_string;

        Process_handler(Process_handler &&obj);
        Process_handler& operator=(Process_handler &&obj);

        Process_handler(const Process_handler &obj);
        Process_handler& operator=(const Process_handler &obj);

        const pid_t GetPID() const {return PID;};

        void Set_profiled(bool value){this->profiled = value;};
        bool Get_profiled() const {return profiled;};

        void Set_alive(bool value){this->alive = value;};
		bool Get_alive() const {return alive;};

        void Start_Stop_profiling() const;

        bool Init_shared_memory();
        bool Remap_shared_memory();

        const memory_profiler_sm_object_class* Get_shared_memory() const;
        const bool Is_shared_memory_initialized() const {return shared_memory_initialized;} ;

        const string Find_function_name(uint64_t const address) const;

        void Print_shared_memory() const;
        void Print_backtrace(unsigned int entry_num,ofstream &log_file) const;

        void Save_symbol_table_to_file();
        void Save_memory_mappings_to_file();
        void Save_shared_memory_to_file();

};



#endif /* MEMORY_PROFILER_PROCESS_H_ */
