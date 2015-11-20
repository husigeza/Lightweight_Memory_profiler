/*
 * Process.h
 *
 *  Created on: Nov 2, 2015
 *      Author: egezhus
 */

#ifndef PROCESS_H_
#define PROCESS_H_

#include <stdint.h>
#include <semaphore.h>
#include <vector>
#include <bfd.h>
#include "memory_map.h"
#include "symbol_table.h"


using namespace std;

#define max_log_entry 10000
#define max_call_stack_depth 100

class memory_profiler_sm_object_log_entry_class{

public:
	memory_profiler_sm_object_log_entry_class() : thread_id{0},type{0}, size{0},backtrace_length{0},call_stack{nullptr},address{0},valid{false}{};

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
	int log_count;
	memory_profiler_sm_object_log_entry_class log_entry[max_log_entry];
};



class Process_handler {

    pid_t PID;
    string PID_string;

    bool profiled;
    bool alive;

    int shared_memory;
    memory_profiler_sm_object_class *memory_profiler_struct;

    int semaphore_shared_memory;
    sem_t* semaphore;

    string elf_path;

    // Storing memory mappings of the shared libraries used by the customer program
    vector<memory_map_table_entry_class> memory_map_table;
    // Storing the symbols from the ELF with its proper virtual address
	vector<symbol_table_entry_class> function_symbol_table;

    void Init_semaphore();

    bfd* Open_ELF();
    bfd* Open_ELF(string ELF_path);

    long Parse_dynamic_symbol_table_from_ELF(bfd* bfd_ptr,asymbol ***symbol_table);
    long Parse_symbol_table_from_ELF(bfd* bfd_ptr,asymbol ***symbol_table);

    bool Create_symbol_table();
    bool Read_virtual_memory_mapping();

    uint64_t Get_symbol_address_from_ELF(string ELF_path, string symbol_name);
    bool Find_symbol_in_ELF(string ELF_path, string symbol_name);


    public:

    	Process_handler();
        Process_handler(pid_t PID);
        ~Process_handler();
        Process_handler(const Process_handler &obj)noexcept;
        Process_handler& operator=(const Process_handler &obj)noexcept;
        Process_handler(Process_handler &&obj)noexcept;
        Process_handler& operator=(Process_handler &&obj)noexcept;

        void Init_shared_memory();

        pid_t GetPID(){return PID;};

        void Set_profiled(bool value){this->profiled = value;};
        bool Get_profiled(){return profiled;};

        void Set_alive(bool value){this->alive = value;};
		bool Get_alive(){return alive;};

        void Start_Stop_profiling();

        memory_profiler_sm_object_class* Get_shared_memory();

        vector<symbol_table_entry_class>::iterator Find_function(uint64_t &address);
};



#endif /* PROCESS_H_ */
