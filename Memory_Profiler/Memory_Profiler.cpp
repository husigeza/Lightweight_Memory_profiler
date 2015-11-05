#include <iostream>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <map>

#include "Memory_Profiler.h"
#include "Process.h"

#define fifo_path "/home/egezhus/mem_prof_fifo"

using namespace std;


Memory_Profiler::Memory_Profiler() {

	if (mkfifo(fifo_path, 0666) == -1) {

		if (errno == EEXIST) {
			cout << "FIFO already exists" << endl;
		} else {
			cout << "Failed creating FIFO" << "errno: " << errno << endl;
		}
		return;
	}
	cout << "FIFO is created" << endl;
}

Memory_Profiler::~Memory_Profiler() {

	cout << "Memory profiler destructor" << endl;
	unlink(fifo_path);
	Processes.clear();

}

void Memory_Profiler::Print_all_processes() const {

	cout << "Printing all processes" << endl;
	for (auto& element : Processes) {

		cout << "PID: " << element.first << endl;
	}
	cout << "Number of processes: " << Processes.size() << endl;
}

void Memory_Profiler::Print_profiled_processes() {

	cout << "Printing profiled" << endl;
	for (auto& element : Processes) {

		if (element.second.Get_profiled() == true) {
			cout << element.first << endl;
		}
	}
}

void Memory_Profiler::Add_Process_to_list(const pid_t PID) {

	if (Processes.find(PID) == Processes.end()) {
		Processes.insert(pair<const pid_t,Process_handler> (PID,Process_handler(PID)));
	} else {
		//cout<< "Process is already added to process list: " << PID << endl;
	}

}
void Memory_Profiler::Add_process_to_profiling(const pid_t PID) {

	Processes[PID].Init_shared_memory();
	Processes[PID].Set_profiled(true);
}

void Memory_Profiler::Add_all_process_to_profiling() {

	map<const pid_t, Process_handler>::iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {

		Add_process_to_profiling(it->first);
	}
}

void Memory_Profiler::Remove_process_from_profiling(const pid_t PID){

	Processes[PID].Set_profiled(false);
}

void Memory_Profiler::Remove_all_process_from_profiling(){

	map<const pid_t, Process_handler>::iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {

		Remove_process_from_profiling(it->first);
	}
}

/*std::vector<uint32_t> Memory_Profiler::Get_profiled_processes_list()const{

 return Profiled_processes;
 }*/

map<const pid_t, Process_handler>& Memory_Profiler::Get_all_processes_list(){

	return Processes;
}

void Memory_Profiler::Start_stop_profiling(const pid_t PID){

	Processes[PID].Start_profiling();
}


void Memory_Profiler::Start_stop_profiling_all_processes(){

	map<const pid_t, Process_handler>::iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {

		if (it->second.Get_profiled() == true) {
			Start_stop_profiling(it->first);
		}
	}
}

inline void Memory_Profiler::Send_signal_to_process(const pid_t PID) {

	Processes[PID].Send_signal();

}

void Memory_Profiler::Send_signal_to_all_processes() {

	map<const pid_t, Process_handler>::iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {

		if (it->second.Get_profiled() == true) {
			Send_signal_to_process(it->first);
		}
	}
}

void Memory_Profiler::Read_FIFO() {

	int mem_prof_fifo = open(fifo_path, O_RDONLY);
	char buffer[6];
	size_t buff_size = 6;
	pid_t pid;
	int res;

	if (mem_prof_fifo != -1) {
		while ((res = read(mem_prof_fifo, &buffer, sizeof(buffer))) != 0) {

			if (res != -1) {
				pid = stoi(buffer, &buff_size);
				Add_Process_to_list(pid);
			} else {
				printf("Failed reading the FIFO\n");
				break;
			}
		}
	} else {
		cout << "Failed opening the FIFO, errno: " << errno << endl;
	}
	close(mem_prof_fifo);
}

void Memory_Profiler::Print_profiled_process_shared_memory(const pid_t PID) {

	//if (Processes[PID].Get_profiled() == true) {
		memory_profiler_struct_t *shared_memory = Processes[PID].Get_shared_memory();

		for (int j=0; j < shared_memory->log_count; j++) {

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
	//}
}

void Memory_Profiler::Print_all_profiled_processes_shared_memory() {

	cout << "Printing shared memories" << endl;
	map<const pid_t, Process_handler>::iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {
		Print_profiled_process_shared_memory(it->first);
	}
}
