/*
 * Process.h
 *
 *  Created on: Nov 2, 2015
 *      Author: egezhus
 */

#ifndef MEMORY_PROFILER_PROCESS_H_
#define MEMORY_PROFILER_PROCESS_H_

#include <stdint.h>
#include <semaphore.h>
#include <vector>
#include <map>

#include <bfd.h>

#include "Memory_Profiler_memory_map.h"
#include "Memory_Profiler_symbol_table.h"

#include <sstream>
#define SSTR( x ) dynamic_cast< std::ostringstream & >( std::ostringstream() << std::dec << x ).str()


using namespace std;

#define max_call_stack_depth 100

class memory_profiler_sm_object_log_entry_class{

public:
	memory_profiler_sm_object_log_entry_class() {
		thread_id = 0;
		type = 0;
		size = 0;
		backtrace_length = 0;
		//call_stack = nullptr;
		address = 0;
		valid = false;
	}

	pthread_t thread_id;
	int type; //malloc = 1, free = 2
	size_t  size; // in case of malloc
	int backtrace_length;
	void *call_stack[max_call_stack_depth];
	uint64_t address;
	bool valid;
};

class memory_profiler_sm_object_class {

public:

	long unsigned int log_count; // Always has a bigger value with 1 than the real element number
	memory_profiler_sm_object_log_entry_class log_entry[1];
};

struct memory_map_table_entry_class_comp {
  bool operator() (const memory_map_table_entry_class & memory_map_A, const memory_map_table_entry_class & memory_map_B) const
  {return memory_map_A.end_address < memory_map_B.end_address;}
};

class Process_handler {

    pid_t PID;

    bool profiled;
    bool alive;

    int shared_memory;
    bool shared_memory_initialized;
    memory_profiler_sm_object_class *memory_profiler_struct;
    unsigned long mapped_size_of_shared_memory;

    //TODO: right know this semaphore is unused
    int semaphore_shared_memory;
    sem_t* semaphore;

    string elf_path;

    // This container stores the local symbols and symbols from shared libraries
    map<memory_map_table_entry_class,vector<symbol_table_entry_class>, memory_map_table_entry_class_comp> all_function_symbol_table;

    // Storing memory mappings of the shared libraries used by the customer program
    vector<memory_map_table_entry_class> memory_map_table;
    // Storing the symbols from the ELF with its proper virtual address
	vector<symbol_table_entry_class> function_symbol_table;

    bfd* Open_ELF();
    bfd* Open_ELF(string ELF_path);

    long Parse_dynamic_symbol_table_from_ELF(bfd* bfd_ptr,asymbol ***symbol_table);
    long Parse_symbol_table_from_ELF(bfd* bfd_ptr,asymbol ***symbol_table);

    bool Get_defined_symbols(asymbol ***symbol_table_param,long number_of_symbols,vector<symbol_table_entry_class> &tmp_function_symbol_table,unsigned long VMA_start_address);
    bool Create_symbol_table();
    bool Read_virtual_memory_mapping();

    void Init_semaphore();

    public:

    	Process_handler();
        Process_handler(pid_t PID);
        ~Process_handler();

        string PID_string;

        void Process_delete();

        Process_handler(Process_handler &&obj);
        Process_handler& operator=(Process_handler &&obj);

        pid_t GetPID(){return PID;};

        void Set_profiled(bool value){this->profiled = value;};
        bool Get_profiled(){return profiled;};

        void Set_alive(bool value){this->alive = value;};
		bool Get_alive(){return alive;};

        void Start_Stop_profiling();

        bool Init_shared_memory();
        bool Remap_shared_memory();

        memory_profiler_sm_object_class* Get_shared_memory();
        bool Is_shared_memory_initialized(){return shared_memory_initialized;};

        vector<symbol_table_entry_class>::iterator Find_function(uint64_t address);
        map<memory_map_table_entry_class const,vector<symbol_table_entry_class>,memory_map_table_entry_class_comp >::iterator Find_function_VMA(uint64_t address);

};



#endif /* MEMORY_PROFILER_PROCESS_H_ */
