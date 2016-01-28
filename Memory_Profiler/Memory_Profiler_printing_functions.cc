#include <iostream>

#include "Memory_Profiler_class.h"

using namespace std;

void Memory_Profiler::Print_all_processes(){

	cout << "Printing all processes" << endl;
	map<const pid_t, Process_handler>::iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {
		cout << "PID: " << std::dec << it->first << endl;
	}
}

void Memory_Profiler::Print_alive_processes() {

	cout << "Printing alive processes" << endl;

	map<const pid_t, Process_handler>::iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {
		if(Get_process_alive_flag(it->first)){
			cout << "PID: " << std::dec << it->first << endl;
		}
	}
}

void Memory_Profiler::Print_profiled_processes() {

	cout << "Printing profiled" << endl;
	map<const pid_t, Process_handler>::iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {
		if(it->second.Get_profiled() == true){
			cout << "PID: " << std::dec << it->first << endl;
		}

	}
}

void Memory_Profiler::Print_profiled_process_shared_memory(const pid_t PID) {

	if (Processes[PID].Get_profiled() == true) {
		memory_profiler_sm_object_class *shared_memory = Processes[PID].Get_shared_memory();

		for (unsigned int j = 0; j < shared_memory->log_count; j++) {

			if(shared_memory->log_entry[j].valid == true){
				cout << endl <<"Shared memory PID: " << PID << endl;
				cout <<"Shared_memory index: " << j << endl;
				cout <<"Thread ID: " << shared_memory->log_entry[j].thread_id << endl;
				cout <<"Call stack type: " << shared_memory->log_entry[j].type << endl;
				cout <<"Address: " << shared_memory->log_entry[j].address << endl;
				cout <<"Call stack size: " << shared_memory->log_entry[j].size << endl;
				cout <<"call stack: " << endl;

				for(uint32_t i=0; i < shared_memory->log_entry[j].size;i++){
					cout << shared_memory->log_entry[j].call_stack[i] << endl;
				}
			}
		}
	}
}

void Memory_Profiler::Print_all_processes_shared_memory() {

	cout << "Printing shared memories" << endl;
	map<const pid_t, Process_handler>::iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {
		if(it->second.Get_shared_memory() != NULL){
			cout << "Process's address: " << &Processes[it->first] << endl;
			cout << "log_count in printing "<< it->second.Get_shared_memory()->log_count << endl;
			for (unsigned int j = 0; j < Processes[it->first].Get_shared_memory()->log_count; j++) {
				if(it->second.Get_shared_memory()->log_entry[j].valid == true){
					cout << endl <<"Shared memory PID: " << std::dec << it->first << endl;
					cout <<"Shared_memory index: " << std::dec << j << endl;
					cout <<"Thread ID: " << it->second.Get_shared_memory()->log_entry[j].thread_id << endl;
					cout <<"Call stack type: " << it->second.Get_shared_memory()->log_entry[j].type << endl;
					cout <<"Address: " << std::hex << it->second.Get_shared_memory()->log_entry[j].address << endl;
					cout <<"Allocated size: " << std::dec << it->second.Get_shared_memory()->log_entry[j].size << endl;
					cout <<"Call stack size: " << std::dec << it->second.Get_shared_memory()->log_entry[j].backtrace_length << endl;
					cout <<"call stack: " << endl;

					for(int  k=0; k < it->second.Get_shared_memory()->log_entry[j].backtrace_length;k++){
						cout << it->second.Get_shared_memory()->log_entry[j].call_stack[k]<< " --- ";
						cout << it->second.Find_function((uint64_t&)it->second.Get_shared_memory()->log_entry[j].call_stack[k])->name<< endl;
					}
				}
			}
		}
	}
}

void Memory_Profiler::Print_process_symbol_table(pid_t PID){

	map<const pid_t, Process_handler>::iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {
		//TODO: complete this
	}

}
