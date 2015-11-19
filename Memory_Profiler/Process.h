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

#define max_log_entry 10000
#define max_call_stack_depth 100

class memory_profiler_log_entry_t{

public:
	pthread_t thread_id;
	int type; //malloc = 1, free = 2
	size_t  size; // in case of malloc
	int backtrace_length;
	void *call_stack[max_call_stack_depth];
	uint64_t address;
	bool valid;
};

//TODO Rename the classes from struct to class
class memory_profiler_struct_t {

public:
	int log_count;
	memory_profiler_log_entry_t log_entry[max_log_entry];
};

class symbol_table_entry_class {

public:
	string name;
	uint64_t address;

	symbol_table_entry_class() : name{""},address{0} {};
	symbol_table_entry_class(string name_p, uint64_t address_p): name{name_p},address{address_p} {};


	symbol_table_entry_class(const symbol_table_entry_class &obj){
		address = obj.address;
		name = obj.name;
	}

	symbol_table_entry_class& operator=(const symbol_table_entry_class &obj)noexcept {
		address = obj.address;
		name = obj.name;
		return *this;
	}

	symbol_table_entry_class(symbol_table_entry_class &&obj)noexcept{

		if(this != &obj){
			address = obj.address;
			name = obj.name;

			obj.address = 0;
			obj.name = "";
		}
	}

	symbol_table_entry_class& operator=(symbol_table_entry_class &&obj)noexcept{

		if (this != &obj) {
			name = obj.name;
			address = obj.address;

			obj.name = "";
			obj.address = 0;
		}
		return *this;
	}




	bool operator==(const string& symbol_name) const {
		    if(this->name.compare(symbol_name) == 0)return true;
		    else return false;
	}
	bool operator!=(const string& symbol_name) const {return(!(this->name == symbol_name));}
	bool operator<(const symbol_table_entry_class& entry) const {return (address < entry.address);}
	bool operator>(const symbol_table_entry_class& entry) const {return (!(address < entry.address));}
	bool operator<=(const symbol_table_entry_class& entry) const {return (address < entry.address);}
	bool operator>=(const symbol_table_entry_class& entry) const {return (!(address > entry.address));}


};


class memory_map_table_entry_class {

public:
	uint64_t start_address;
	uint64_t end_address;
	string shared_lib_path;

	memory_map_table_entry_class() : start_address{0},end_address{0},shared_lib_path{""} {};
	memory_map_table_entry_class(const memory_map_table_entry_class &obj) {
		start_address = obj.start_address;
		end_address = obj.end_address;
		shared_lib_path = obj.shared_lib_path;
	}

	memory_map_table_entry_class& operator=(const memory_map_table_entry_class &obj)noexcept{

			start_address = obj.start_address;
			end_address = obj.end_address;
			shared_lib_path = obj.shared_lib_path;

			return *this;
	}

	memory_map_table_entry_class(memory_map_table_entry_class &&obj)noexcept{
		if(this != &obj) {
			start_address = obj.start_address;
			end_address = obj.end_address;
			shared_lib_path = obj.shared_lib_path;

			obj.start_address = 0;
			obj.end_address = 0;
			obj.shared_lib_path = "";
		}
	}
	memory_map_table_entry_class& operator=(memory_map_table_entry_class &&obj)noexcept{
		if(this != &obj) {
			start_address = obj.start_address;
			end_address = obj.end_address;
			shared_lib_path = obj.shared_lib_path;

			obj.start_address = 0;
			obj.end_address = 0;
			obj.shared_lib_path = "";
		}
		return *this;
	}

	bool operator==(const string& shared_lib_path) const {
	    if(this->shared_lib_path.compare(shared_lib_path) == 0)return true;
	    else return false;
	}
	bool operator!=(const string& shared_lib_path) const {return(!(this->shared_lib_path == shared_lib_path));}
	bool operator<(const memory_map_table_entry_class& entry) const {return (start_address < entry.start_address);}
	bool operator>(const memory_map_table_entry_class& entry) const {return (!(start_address < entry.start_address));}
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
    vector<memory_map_table_entry_class> memory_map_table;
    // Storing the symbols from the ELF with its proper virtual address
	vector<symbol_table_entry_class> function_symbol_table;

    void Init_semaphore();

    bfd* Open_ELF();
    bfd* Open_ELF(string ELF_path);

    long Parse_symbol_table_from_ELF(bfd* bfd_ptr,asymbol ***symbol_table);

    bool Create_symbol_table();
    bool Read_virtual_memory_mapping();

    uint64_t Get_symbol_address_from_ELF(string ELF_path, string symbol_name);
    bool Find_symbol_in_ELF(string ELF_path, string symbol_name);


    public:

    	Process_handler();
        Process_handler(pid_t PID);
        ~Process_handler();
        Process_handler(const Process_handler &obj);
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

        vector<symbol_table_entry_class>::iterator Find_function(uint64_t &address);
};



#endif /* PROCESS_H_ */
