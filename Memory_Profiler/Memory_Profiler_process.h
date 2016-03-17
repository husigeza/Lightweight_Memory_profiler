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
#include <iostream>

#include "bfd.h"


#include "Memory_Profiler_memory_map.h"
#include "Memory_Profiler_symbol_table.h"

#include "Memory_Profiler_handler_template.h"

#include <sstream>
#define SSTR( x ) dynamic_cast< std::ostringstream & >( std::ostringstream() << std::dec << x ).str()


enum {
	malloc_func = 1,
	free_func = 2,
	calloc_func = 3,
	realloc_func = 4
};

using namespace std;


#define shared_memory_MAX_ENTRY 100000
#define max_call_stack_depth 100

class Process_handler;


class memory_profiler_sm_object_log_entry_class{

public:
	memory_profiler_sm_object_log_entry_class() {

		valid = false;
		thread_id = 0;
		type = 0;
		size = 0;
		backtrace_length = 0;
		address = 0;
	}

	pthread_t thread_id;
	struct timeval tval_before;
	struct timeval tval_after;
	unsigned int type;
	unsigned long int size;
	unsigned int backtrace_length;
	void *call_stack[max_call_stack_depth];
	unsigned long int address;
	bool valid;

	void wite_to_file(ofstream &entriesfile);
	void read_from_file(ofstream &entriesfile);

	void Print(template_handler<Process_handler> process, ofstream &log_file) const;
	void Print(template_handler<Process_handler> process) const;

};

class memory_profiler_sm_object_class_base {

protected:
	memory_profiler_sm_object_class_base(){
		log_count = 0;
		profiled = false;
		active = false;
	}

	memory_profiler_sm_object_class_base(unsigned long int entry_count){
		log_count = entry_count;
		profiled = false;
		active = false;
	}

public:
	~memory_profiler_sm_object_class_base(){};

	unsigned long int log_count; // Always has a bigger value with 1 than the real element number
	bool profiled;
	bool active;

};

class memory_profiler_sm_object_class_fix : public memory_profiler_sm_object_class_base{

	void write_header_to_file(string filename, unsigned long int total_log_count);
	void write_entries_to_file(string filename);

public:
	memory_profiler_sm_object_class_fix() : memory_profiler_sm_object_class_base(){}
	~memory_profiler_sm_object_class_fix(){}

	memory_profiler_sm_object_log_entry_class log_entry[shared_memory_MAX_ENTRY];

	void write_to_binary_file(string PID,unsigned long int total_log_count);
};


class memory_profiler_sm_object_class : public memory_profiler_sm_object_class_base{

	void read_header_from_file(string filename);
	void read_entries_from_file(string filename);

public:
	memory_profiler_sm_object_class(unsigned long int entry_count) : memory_profiler_sm_object_class_base(entry_count){
		log_entry = new memory_profiler_sm_object_log_entry_class[entry_count];
	}

	~memory_profiler_sm_object_class(){
		cout << " log_entry destructor" << endl;
		delete[] log_entry;
	}

	memory_profiler_sm_object_log_entry_class *log_entry;

	void read_from_binary_file(string PID);
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

    int shared_memory_A;
    int shared_memory_B;
    string shared_memory_name_A;
    string shared_memory_name_B;

    string header;
    string entries;


    unsigned long mapped_size_of_shared_memory;

    int start_stop_semaphore_shared_memory;
    string start_stop_semaphore_name;
    sem_t* start_stop_semaphore;

    string elf_path;

    bfd* Open_ELF() const;
    bfd* Open_ELF(string ELF_path) const;

    long Parse_dynamic_symbol_table_from_ELF(bfd* bfd_ptr,asymbol ***symbol_table);
    long Parse_symbol_table_from_ELF(bfd* bfd_ptr,asymbol ***symbol_table);

    void Get_defined_symbols(asymbol ***symbol_table_param,long number_of_symbols,vector<symbol_table_entry_class> &tmp_function_symbol_table,unsigned long VMA_start_address);
    bool Create_symbol_table();
    bool Read_virtual_memory_mapping();

    map<memory_map_table_entry_class,vector<symbol_table_entry_class>,memory_map_table_entry_class_comp >::const_iterator Find_function_VMA (const uint64_t address) const;

    bool Init_start_stop_semaphore();

    public:

    	Process_handler();
        Process_handler(pid_t PID);
        ~Process_handler();

        string symbol_file_name;
    	string memory_map_file_name;
    	string shared_memory_file_name;

        // This container stores the local symbols and symbols from shared libraries
        map<memory_map_table_entry_class,vector<symbol_table_entry_class>, memory_map_table_entry_class_comp> all_function_symbol_table;

        memory_profiler_sm_object_class_fix *memory_profiler_struct_A;
        memory_profiler_sm_object_class_fix *memory_profiler_struct_B;

        memory_profiler_sm_object_class *memory_profiler_struct;

        unsigned long int total_entry_number;

        string PID_string;

        Process_handler(const Process_handler &obj);
        Process_handler& operator=(const Process_handler &obj);


        void Set_profiled(bool value){this->profiled = value;};
        bool Get_profiled() const {return profiled;};

        void Set_alive(bool value){this->alive = value;};
		bool Get_alive() const {return alive;};

        void Start_Stop_profiling() const;

        bool Init_shared_memory();

        void Read_shared_memory();

        const bool Is_shared_memory_initialized() const {return shared_memory_initialized;} ;

        const string Find_function_name(uint64_t const address) const;

};



#endif /* MEMORY_PROFILER_PROCESS_H_ */
