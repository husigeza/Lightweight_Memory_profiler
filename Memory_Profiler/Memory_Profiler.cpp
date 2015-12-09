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

bool Memory_Profiler::Add_Process_to_list(const pid_t PID) {

	if (Processes.find(PID) == Processes.end()) {
		Processes.insert(pair<const pid_t,Process_handler> (PID,Process_handler(PID)));
		cout << "Process added, PID: " << PID << endl;
		return true;
	} else {
		return false;
		//cout<< "Process is already added to process list: " << PID << endl;
	}

}

bool Memory_Profiler::Add_Process_to_list(const pid_t PID, const string EXE_path){

	if (Processes.find(PID) == Processes.end()) {
			Processes.insert(pair<const pid_t,Process_handler> (PID,Process_handler(PID,EXE_path)));
			cout << "Process added: "<< PID << endl;
			return true;
	} else {
			return false;
			//cout<< "Process is already added to process list: " << PID << endl;
	}
}

void Memory_Profiler::Add_process_to_profiling(const pid_t PID) {

	//Shared memory is initialized here (if it has not been initialized before) because before starting the first profiling we want to
	//prevent the system to create unnecessary shared memories

	//TODO: It would be better (faster) to get the reference of the Process object, and check the values. With this methods it is searched again and again from the map
	if(Get_process_alive_flag(PID) == true){
		if(Get_process_shared_memory_initilized_flag(PID) == false){
			if(Processes[PID].Init_shared_memory() == false){
				cout << "Shared memory init unsuccessful" << endl;
			}
		}
		Processes[PID].Set_profiled(true);
	}
	else{
		cout << "Process " << PID << " is not alive!" << endl;
	}
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

bool Memory_Profiler::Get_process_shared_memory_initilized_flag(const pid_t PID){

	return Processes[PID].Is_shared_memory_initialized();
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

bool Memory_Profiler::Remap_process_shared_memory(const pid_t PID){

	if(Processes[PID].Get_profiled()){
		return Processes[PID].Remap_shared_memory();
	}
	else return false;

}

void Memory_Profiler::Remap_all_process_shared_memory(){

	/*for(auto &elem : Processes){
		if(elem.second.Remap_shared_memory() == false){
			cout << "Failed remapping process " << elem.first << "shared memory" << endl;
		}
	}*/
	map<const pid_t, Process_handler>::iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {
			if(it->second.Remap_shared_memory() == false){
				cout << "Failed remapping process's " << it->first << " shared memory" << endl;
			}
		}

 }


struct FIFO_struct_s{
	char PID_string[6];
	// TODO CHange this to max path size on Linux
	char Binary_location[1024];
};


void Memory_Profiler::Read_FIFO() {

	vector<pid_t> alive_processes;
	struct FIFO_struct_s FIFO_struct;
	pid_t pid;
	string PID;
	string EXE_path;
	size_t buff_size = sizeof(FIFO_struct.PID_string);

	int res;
	map<const pid_t, Process_handler>::iterator it;


	if (mem_prof_fifo != -1) {
		while ((res = read(mem_prof_fifo, &FIFO_struct, sizeof(FIFO_struct))) != 0) {
			if (res > 0) {

				EXE_path = FIFO_struct.Binary_location;
				pid = stoi(FIFO_struct.PID_string, &buff_size);
				Add_Process_to_list(pid,EXE_path);
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


