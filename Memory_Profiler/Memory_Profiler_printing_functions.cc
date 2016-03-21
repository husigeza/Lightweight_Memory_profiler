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
		cout <<"Total number of entries have been read: " << dec << it->second.object->total_entry_number << endl;
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
			Print_process(it->first);
		}
	}

	cout << endl << "Dead processes:" << endl;
	for (it = Processes.begin(); it != Processes.end(); it++) {
		if(!it->second.object->Get_alive()){
			Print_process(it->first);
		}
	}
}

void Memory_Profiler::Print_profiled_processes() const{

	cout << endl <<"Profiled processes: " << endl;
	map<const pid_t, template_handler<Process_handler> >::const_iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {
		if(it->second.object->Get_profiled() == true){
			Print_process(it->first);
		}

	}
}

