#include <iostream>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <map>
#include <vector>
#include <algorithm>
#include <bfd.h>

#include "Memory_Profiler.h"
#include "Process.h"

using namespace std;


Memory_Profiler::Memory_Profiler(string fifo_path) {

	this->fifo_path = fifo_path;

	if (mkfifo(this->fifo_path.c_str(), 0666) == -1) {

		if (errno == EEXIST) {
			cout << "FIFO already exists" << endl;
		} else {
			cout << "Failed creating FIFO" << "errno: " << errno << endl;
			return;
		}
	}
	else {
	cout << "FIFO is created" << endl;
	}
	mem_prof_fifo = open(fifo_path.c_str(), O_RDONLY | O_NONBLOCK );

}

Memory_Profiler::~Memory_Profiler() {

	cout << "Memory profiler destructor" << endl;
	close(mem_prof_fifo);
	unlink(fifo_path.c_str());
	Processes.clear();
}


void Memory_Profiler::Print_all_processes() const {

	cout << "Printing all processes" << endl;
	for (auto& element : Processes) {

		cout << "PID: " << std::dec << element.first << endl;
	}
	cout << "Number of processes: " << Processes.size() << endl;
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
	for (auto& element : Processes) {

		if (element.second.Get_profiled() == true) {
			cout << element.first << endl;
		}
	}
}

bool Memory_Profiler::Add_Process_to_list(const pid_t PID) {

	if (Processes.find(PID) == Processes.end()) {
		Processes.insert(pair<const pid_t,Process_handler> (PID,Process_handler(PID)));
		return true;
	} else {
		return false;
		//cout<< "Process is already added to process list: " << PID << endl;
	}

}
void Memory_Profiler::Add_process_to_profiling(const pid_t PID) {

	//Shared memory is initialized here because before starting the first profiling we want to
	//prevent to system to create unnecessary shared memories, e.g: if this would be in constructor
	//lots of unused shared memories were existing before the first profiling
	Processes[PID].Init_shared_memory();
	Processes[PID].Set_profiled(true);
}

void Memory_Profiler::Add_all_process_to_profiling() {

	map<const pid_t, Process_handler>::iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {

		Add_process_to_profiling(it->first);
	}
}

void Memory_Profiler::Set_process_alive_flag(const pid_t PID, bool value){

	Processes[PID].Set_alive(value);
}

bool Memory_Profiler::Get_process_alive_flag(const pid_t PID){

	return Processes[PID].Get_alive();
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

	Processes[PID].Start_Stop_profiling();
}


void Memory_Profiler::Start_stop_profiling_all_processes(){

	map<const pid_t, Process_handler>::iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {

		if (it->second.Get_profiled() == true) {
			Start_stop_profiling(it->first);
		}
	}
}


void Memory_Profiler::Read_FIFO() {

	vector<pid_t> alive_processes;
	pid_t pid;
	char buffer[6];
	size_t buff_size = 6;
	int res;
	map<const pid_t, Process_handler>::iterator it;

	if (mem_prof_fifo != -1) {
		while ((res = read(mem_prof_fifo, &buffer, sizeof(buffer))) != 0) {

			if (res > 0) {
				pid = stoi(buffer, &buff_size);
				Add_Process_to_list(pid);
				alive_processes.push_back(pid);

			} else {
				cout << "Failed reading the FIFO" << endl;
				return;
			}
		}
		// IF nobody has written the FIFO, res = 0, alive_processes = 0, all the processes are dead
		for (it = Processes.begin(); it != Processes.end(); it++) {
			if(find(alive_processes.begin(), alive_processes.end(), it->first) == alive_processes.end()) {
				Set_process_alive_flag(it->first,false);
			}
			else {
				Set_process_alive_flag(it->first,true);
			}
		}
	} else {
		cout << "Failed opening the FIFO, errno: " << errno << endl;
	}
}

void Memory_Profiler::Print_profiled_process_shared_memory(const pid_t PID) {

	if (Processes[PID].Get_profiled() == true) {
		memory_profiler_sm_object_class *shared_memory = Processes[PID].Get_shared_memory();

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
	}
}

void Memory_Profiler::Print_all_processes_shared_memory() {

	cout << "Printing shared memories" << endl;
	map<const pid_t, Process_handler>::iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {
		if(Processes[it->first].Get_shared_memory() != NULL){
			for (int j=0; j < Processes[it->first].Get_shared_memory()->log_count; j++) {

				if(Processes[it->first].Get_shared_memory()->log_entry[j].valid == true){
					cout << endl <<"Shared memory PID: " << std::dec << it->first << endl;
					cout <<"Shared_memory index: " << j << endl;
					cout <<"Thread ID: " << Processes[it->first].Get_shared_memory()->log_entry[j].thread_id << endl;
					cout <<"Call stack type: " << Processes[it->first].Get_shared_memory()->log_entry[j].type << endl;
					cout <<"Address: " << std::hex << Processes[it->first].Get_shared_memory()->log_entry[j].address << endl;
					cout <<"Allocated size: " << Processes[it->first].Get_shared_memory()->log_entry[j].size << endl;
					cout <<"Call stack size: " << Processes[it->first].Get_shared_memory()->log_entry[j].backtrace_length << endl;
					cout <<"call stack: " << endl;

					for(int  k=0; k < Processes[it->first].Get_shared_memory()->log_entry[j].backtrace_length;k++){
						cout << Processes[it->first].Get_shared_memory()->log_entry[j].call_stack[k]<< " --- ";
						cout << Processes[it->first].Find_function((uint64_t&)Processes[it->first].Get_shared_memory()->log_entry[j].call_stack[k])->name<< endl;
					}
				}
			}
		}
	}
	/*for(auto process : Processes){
		if(process.second.Get_shared_memory() != NULL) {
			for(int j=0; j < process.second.Get_shared_memory()->log_count; j++){
				if(process.second.Get_shared_memory()->log_entry[j].valid == true){
					cout <<"Shared memory PID: " << process.first << endl;
					cout <<"Address: " << process.second.Get_shared_memory()->log_entry[j].address << endl;
				}
			}
		}
	}*/

}

void Memory_Profiler::Print_process_symbol_table(pid_t PID){

	map<const pid_t, Process_handler>::iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {


		//TODO: complete this
	}

}
