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



using namespace std;

#define max_log_entry 1000
#define max_call_stack_depth 15

class memory_profiler_log_entry_t{

public:
	pthread_t thread_id;
	int type; //malloc = 1, free = 2
	size_t size; //allocated bytes in case of malloc
	void *call_stack[max_call_stack_depth];
	uint64_t address;
	bool valid;
};


class memory_profiler_struct_t {

public:
	int log_count;
	memory_profiler_log_entry_t log_entry[max_log_entry];
};

class symbol_table_entry_struct_t {

public:
	string name;
	uint64_t address;

	bool operator==(const string& symbol_name) const
		{
		    if(this->name.compare(symbol_name) == 0){
		    	return true;
		    }
		    else {
		    	return false;
		    }

		}
};

class memory_map_table_entry_struct_t {

public:
	uint64_t start_address;
	uint64_t end_address;
	string shared_lib_path;

	bool operator==(const string& shared_lib_path) const
	{
	    if(this->shared_lib_path.compare(shared_lib_path) == 0){
	    	return true;
	    }
	    else {
	    	return false;
	    }

	}

	uint64_t is_symbol_in_shared_lib(string &symbol_name){


		return false;
	}

};


class Process_handler {

    pid_t PID;
    string PID_string;

    bool profiled;
    bool alive;

    int shared_memory;
    memory_profiler_struct_t *memory_profiler_struct;

    int semaphore_shared_memory;
    sem_t* semaphore;

    string elf_path;

    // Storing memory mappings of the shared libraries used by the customer program
    vector<memory_map_table_entry_struct_t> memory_map_table;
    // Storing the symbols from the ELF with its proper virtual address
    vector<symbol_table_entry_struct_t> function_symbol_table;



    void Init_semaphore();

    bfd* Open_ELF();
    bfd* Open_ELF(string ELF_path);
    bool Create_symbol_table();
    bool Read_virtual_memory_mapping();

    uint64_t Get_symbol_address_from_ELF(string ELF_path, string symbol_name);
    bool Find_symbol_in_ELF(string ELF_path, string symbol_name);


    public:

    	Process_handler();
        Process_handler(pid_t PID);
        ~Process_handler();
        Process_handler(const Process_handler &obj) = delete;
        Process_handler(Process_handler &&obj);
        Process_handler& operator=(Process_handler &&obj);

        void Init_shared_memory();

        pid_t GetPID(){return PID;};

        void Set_profiled(bool value){this->profiled = value;};
        bool Get_profiled(){return profiled;};

        void Set_alive(bool value){this->alive = value;};
		bool Get_alive(){return alive;};

        void Send_signal();
        void Start_Stop_profiling();

        memory_profiler_struct_t* Get_shared_memory();
        vector<asymbol>& Get_function_symbol_table();
};



#endif /* PROCESS_H_ */
