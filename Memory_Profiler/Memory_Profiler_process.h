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

#include <bfd.h>

#include "Memory_Profiler_memory_map.h"
#include "Memory_Profiler_symbol_table.h"

#include <sstream>

#define SSTR( x ) dynamic_cast< std::ostringstream & >( std::ostringstream() << std::dec << x ).str()


using namespace std;

#define max_call_stack_depth 100

class memory_profiler_sm_object_log_entry_class{

public:
	pthread_t thread_id;
	int type; //malloc = 1, free = 2
	size_t  size; // in case of malloc
	int backtrace_length;
	void *call_stack[max_call_stack_depth];
	uint64_t address;
	bool valid;

	memory_profiler_sm_object_log_entry_class(){
		thread_id = 0;
		type = 0;
		size = 0 ;
		backtrace_length = 0;
		//call_stack = (void *)0;
		address = 0;
		valid = false;
	}
};

class memory_profiler_sm_object_class {

public:

	/*memory_profiler_sm_object_class(const memory_profiler_sm_object_class &obj);
	memory_profiler_sm_object_class& operator=(const memory_profiler_sm_object_class &obj);

	memory_profiler_sm_object_class(memory_profiler_sm_object_class &&obj);
	memory_profiler_sm_object_class& operator=(memory_profiler_sm_object_class &&obj);*/

	long unsigned int log_count;
	memory_profiler_sm_object_log_entry_class log_entry[1];
};



class Process_handler {

    pid_t PID;
    string PID_string;

    bool profiled;
    bool alive;

    int shared_memory;
    bool shared_memory_initialized;
    memory_profiler_sm_object_class *memory_profiler_struct;

    int semaphore_shared_memory;
    sem_t* semaphore;

    string elf_path;

    // Storing memory mappings of the shared libraries used by the customer program
    vector<memory_map_table_entry_class> memory_map_table;
    // Storing the symbols from the ELF with its proper virtual address
	vector<symbol_table_entry_class> function_symbol_table;



    bfd* Open_ELF();
    bfd* Open_ELF(string ELF_path);

    long Parse_dynamic_symbol_table_from_ELF(bfd* bfd_ptr,asymbol ***symbol_table);
    long Parse_symbol_table_from_ELF(bfd* bfd_ptr,asymbol ***symbol_table);

    bool Create_symbol_table();
    bool Read_virtual_memory_mapping();
    string Find_memory_profiler_library_name();

    uint64_t Get_symbol_address_from_ELF(string ELF_path, string symbol_name);
    bool Find_symbol_in_ELF(string ELF_path, string symbol_name);

    void Init_semaphore();


    public:

    	Process_handler();
        Process_handler(pid_t PID);
        ~Process_handler();

        void Process_delete();

        Process_handler(const Process_handler &obj);
        Process_handler& operator=(const Process_handler &obj);

        /*Process_handler(Process_handler &&obj);
        Process_handler& operator=(Process_handler &&obj);*/

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

};



#endif /* MEMORY_PROFILER_PROCESS_H_ */
