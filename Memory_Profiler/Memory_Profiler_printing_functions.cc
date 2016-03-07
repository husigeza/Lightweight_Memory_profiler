#include "Memory_Profiler_class.h"
#include "Memory_Profiler_handler_template.h"

#include <iostream>

using namespace std;


void Memory_Profiler::Print_process(const pid_t PID) const{

	cout << endl << "Process " << dec << PID<< endl;
	map<const pid_t, template_handler<Process_handler> >::const_iterator it = Processes.find(PID);

	if(it == Processes.end()){
		cout << "Process not found!" << endl << endl;
	}
	else {
		cout <<"Alive: " << it->second.object->Get_alive() << endl;
		cout <<"Profiled: " << it->second.object->Get_profiled() << endl;
		cout <<"Shared memory initialized: " << it->second.object->Is_shared_memory_initialized() << endl;
		if(it->second.object->Is_shared_memory_initialized()){
			// Use it->second.Get_shared_memory()->log_count is correct here
			// Indexing starts with 0 that's why need to use the actual value (and not -1) here
			cout <<"Number of backtraces: "<< dec << it->second.object->Get_shared_memory()->log_count  << endl;
		}
		cout << endl;
	}

}

void Memory_Profiler::Print_all_processes() const{

	cout << endl << "All processes:" << endl;
	map<const pid_t, template_handler<Process_handler> >::const_iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {
		Print_process(it->first);
	}
}

void Memory_Profiler::Print_alive_processes() const{

	map<const pid_t, template_handler<Process_handler> >::const_iterator it;

	cout << endl << "Alive processes:" << endl;
	for (it = Processes.begin(); it != Processes.end(); it++) {
		if(it->second.object->Get_alive()){
			cout << "PID: " << std::dec << it->first << endl;
		}
	}

	cout << endl << "Dead processes:" << endl;
	for (it = Processes.begin(); it != Processes.end(); it++) {
		if(!it->second.object->Get_alive()){
			cout << "PID: " << std::dec << it->first << endl;
		}
	}
}

void Memory_Profiler::Print_profiled_processes() const{

	cout << endl <<"Profiled processes: " << endl;
	map<const pid_t, template_handler<Process_handler> >::const_iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {
		if(it->second.object->Get_profiled() == true){
			cout << "PID: " << std::dec << it->first << endl;
		}

	}
}

void Memory_Profiler::Print_process_shared_memory(const pid_t PID) const{

	map<const pid_t, template_handler<Process_handler> >::const_iterator it = Processes.find(PID);
	if (it->second.object->Get_profiled() == false) {
		it->second.object->Print_shared_memory();
	}
	else{
		cout << endl << "Process " << dec << PID << " is under profiling! Stop profiling it first!" << endl;
	}
}

void Memory_Profiler::Print_all_processes_shared_memory() const{

	cout << "Backtraces:" << endl;
	map<const pid_t, template_handler<Process_handler> >::const_iterator it;


	for (it = Processes.begin(); it != Processes.end(); it++) {
		if (it->second.object->Get_profiled() == false) {
			it->second.object->Print_shared_memory();
			}
			else{
				cout << endl << "Process " << dec << it->first << " is under profiling! Stop profiling it first!" << endl;
			}
	}
	cout << endl << "Finished" << endl;
}

void Memory_Profiler::Save_process_symbol_table_to_file(const pid_t PID){

	map<const pid_t, template_handler<Process_handler> >::iterator it = Processes.find(PID);
	if(it != Processes.end()){
		Processes[PID].object->Save_symbol_table_to_file();
	}
	else{
		cout << "No Process with " << dec << PID << " found!" << endl;
	}

}

void Memory_Profiler::Save_process_memory_mapping_to_file(const pid_t PID){

	map<const pid_t, template_handler<Process_handler> >::iterator it = Processes.find(PID);
	if(it != Processes.end()){
		Processes[PID].object->Save_memory_mappings_to_file();
	}
	else{
		cout << "No Process with " << dec << PID << " found!" << endl;
	}
}

void Memory_Profiler::Save_process_shared_memory_to_file(const pid_t PID){

	map<const pid_t, template_handler<Process_handler> >::iterator it = Processes.find(PID);
		if(it != Processes.end()){
			Processes[PID].object->Save_shared_memory_to_file();
		}
		else{
			cout << "No Process with " << dec << PID << " found!" << endl;
		}
}

void Memory_Profiler::Save_all_process_shared_memory_to_file(){

	map<const pid_t, template_handler<Process_handler> >::iterator it;
	for (it = Processes.begin(); it != Processes.end(); it++) 	{
		if(it->second.object->Get_profiled() == false){
			it->second.object->Save_shared_memory_to_file();
		}
		else {
			cout << "Process " << dec << it->first << " is under profiling, backtrace cannot be saved!" << endl;
		}
	}
	cout << "Finished" << endl;
}
