#include <iostream>

#include "Memory_Profiler_class.h"

using namespace std;


void Memory_Profiler::Print_process(const pid_t PID) const{

	cout << endl << "Process " << dec << PID<< endl;
	map<const pid_t, Process_handler>::const_iterator it = Processes.find(PID);

	if(it == Processes.end()){
		cout << "Process not found!" << endl << endl;
	}
	else {
		cout <<"Alive: " << it->second.Get_alive() << endl;
		cout <<"Profiled: " << it->second.Get_profiled() << endl;
		cout <<"Shared memory initialized: " << it->second.Is_shared_memory_initialized() << endl;
		if(it->second.Is_shared_memory_initialized()){
			cout <<"Shared memory entry count: "<< dec << it->second.Get_shared_memory()->log_count-1  << endl;
		}
		cout << endl;
	}

}

void Memory_Profiler::Print_all_processes() const{

	cout << endl << "All processes:" << endl;
	map<const pid_t, Process_handler>::const_iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {
		Print_process(it->first);
	}
}

void Memory_Profiler::Print_alive_processes() const{

	map<const pid_t, Process_handler>::const_iterator it;

	cout << endl << "Alive processes:" << endl;
	for (it = Processes.begin(); it != Processes.end(); it++) {
		if(it->second.Get_alive()){
			cout << "PID: " << std::dec << it->first << endl;
		}
	}

	cout << endl << "Dead processes:" << endl;
	for (it = Processes.begin(); it != Processes.end(); it++) {
		if(!it->second.Get_alive()){
			cout << "PID: " << std::dec << it->first << endl;
		}
	}
}

void Memory_Profiler::Print_profiled_processes() const{

	cout << endl <<"Profiled processes: " << endl;
	map<const pid_t, Process_handler>::const_iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {
		if(it->second.Get_profiled() == true){
			cout << "PID: " << std::dec << it->first << endl;
		}

	}
}

void Memory_Profiler::Print_process_shared_memory(const pid_t PID) const{

	map<const pid_t, Process_handler>::const_iterator it = Processes.find(PID);
	if (it->second.Get_profiled() == false) {
		it->second.Print_shared_memory();
	}
	else{
		cout << endl << "Process " << dec << PID << " is under profiling! Stop profiling it first!" << endl;
	}
}

void Memory_Profiler::Print_all_processes_shared_memory() const{

	cout << "Shared memories" << endl;
	map<const pid_t, Process_handler>::const_iterator it;


	for (it = Processes.begin(); it != Processes.end(); it++) {
		if (it->second.Get_profiled() == false) {
			it->second.Print_shared_memory();
			}
			else{
				cout << endl << "Process " << dec << it->first << " is under profiling! Stop profiling it first!" << endl;
			}
	}
	cout << endl << "Finished" << endl;
}

void Memory_Profiler::Print_process_symbol_table(const pid_t PID) const{

	map<const pid_t, Process_handler>::const_iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {
		//TODO: complete this
	}

}
