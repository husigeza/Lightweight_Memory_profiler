#include "Memory_Profiler_class.h"

#include <iostream>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <map>
#include <vector>
#include <algorithm>
#include <fstream>
#include <time.h>


#include "Memory_Profiler_process.h"

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

	//cout << "Memory profiler destructor" << endl;
	close(mem_prof_fifo);
	unlink(fifo_path.c_str());
	Processes.clear();
}

bool Memory_Profiler::Add_Process_to_list(const pid_t PID) {


	if (Processes.find(PID) == Processes.end()) {

		try{
		Processes.insert(pair<const pid_t,Process_handler> (PID,Process_handler(PID)));
		}
		catch(const bool v){
			if(v == false){
				cout << "Process NOT added, PID: " << dec << PID << endl;
				return false;
			}
		}

		if(Processes.find(PID) != Processes.end()){
			cout << "Process added, PID: " << dec << PID << endl;
			return true;
		}
	}

	//cout<< "Process is already in the list: " << PID << endl;
	return true;


}
void Memory_Profiler::Add_process_to_profiling(const pid_t PID) {

	//Shared memory is initialized here (if it has not been initialized before) because before starting the first profiling we want to
	//prevent the system to create unnecessary shared memories

	//TODO: It would be better (faster) to get the reference of the Process object, and check the values. With this methods it is searched again and again from the map
	if(Get_process_alive_flag(PID) == true){
		if(Get_process_shared_memory_initilized_flag(PID) == false){
			cout << "Shared memory has not been initialized, initializing it... Process: " << dec << PID << endl;
			if(Processes[PID].Init_shared_memory() == false){
				cout << "Shared memory init unsuccessful, not added to profiled state." << endl;
				return;
			}
			cout << "Shared memory init successful, added to profiled state." << endl;
		}
		else {
			cout << "Shared memory has been initialized for process: " << dec << PID << endl;
		}
		Processes[PID].Set_profiled(true);
		cout << "Process " << dec << PID << " added to profiled state" << endl;
	}
	else{
		cout << "Process " << PID << " is not alive, not added to profiled state.!" << endl;
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

	return Processes[PID].Remap_shared_memory();

}

void Memory_Profiler::Remap_all_process_shared_memory(){

	map<const pid_t, Process_handler>::iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {
		//cout << "Remapping Process: " << it->first << endl;
			if(it->second.Remap_shared_memory() == false){
				cout << "Failed remapping process " << it->first << " shared memory " << endl;
			}
			else {
				//cout << "Remapping successful "<< endl;
			}
		}
 }


void Memory_Profiler::Read_FIFO() {

	vector<pid_t> alive_processes;
	pid_t pid;
	string buffer;
	size_t buff_size = 6;
	int res;
	map<const pid_t, Process_handler>::iterator it;

	if (mem_prof_fifo != -1) {
		while ((res = read(mem_prof_fifo, (char*)buffer.c_str(), buff_size/*sizeof(buffer)*/)) != 0) {

			if (res > 0) {
				pid = atol( buffer.c_str() );
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
				//cout << "Process " << dec << it->first <<" is no more alive!" << endl;
			}
			else {
				Set_process_alive_flag(it->first,true);
			}
		}
	} else {
		cout << "Failed opening the FIFO, errno: " << errno << endl;
	}
}

enum {
	malloc_func = 1,
	free_func = 2
};


void Memory_Profiler::Analyze_process(const pid_t PID){

	ofstream log_file;
	log_file.open(("Analyzation_output_"+ Processes[PID].PID_string + ".txt").c_str(), ios::app);

	log_file << endl << endl <<"ANALYZING DATA" << endl;

	if(Processes[PID].Is_shared_memory_initialized() == true){
		cout << "Process is being analyzed " << dec << PID << endl;

		const memory_profiler_sm_object_class &shared_memory = *Processes[PID].Get_shared_memory();
		if(shared_memory.log_count == 0){
			log_file << endl << "shared_memory log_count = 0, no data to analyze!" << endl;
			log_file.close();
			cout << endl << "Process " << PID << " shared_memory log_count = 0, no data to analyze!" <<endl;
			return;
		}

		unsigned long int total_counter = 0;
		unsigned long int total_counter_2 = 0;

		unsigned long int malloc_counter = 0;
		unsigned long int free_counter = 0;
		unsigned long int total_memory_allocated = 0;
		unsigned long int total_memory_freed = 0;
		unsigned long int address = 0;
		unsigned long int total_memory_leaked = 0;

		//Need to count with shared_memory.log_count-1 because shared_memory.log_count shows a bigger value with 1 than the real number of elements
		for(total_counter = 0; total_counter < shared_memory.log_count-1; total_counter++){

			if(shared_memory.log_entry[total_counter].valid && shared_memory.log_entry[total_counter].type == malloc_func){

				total_memory_allocated += shared_memory.log_entry[total_counter].size;
				address = shared_memory.log_entry[total_counter].address;
				malloc_counter++;

				for(total_counter_2 = total_counter; total_counter_2 < shared_memory.log_count-1; total_counter_2++){
					if(shared_memory.log_entry[total_counter_2].valid && shared_memory.log_entry[total_counter_2].type == free_func){
						if(shared_memory.log_entry[total_counter_2].address == address){
								total_memory_freed += shared_memory.log_entry[total_counter].size;
								break;
						}
					}
				}
				if(total_counter_2 == shared_memory.log_count-1){

					log_file << endl << "Memory 0x" << std::hex << address << " has not been freed yet!" << endl;

					char buffer[30];
					strftime(buffer,30,"%m-%d-%Y %T.",gmtime(&(shared_memory.log_entry[total_counter].tval.tv_sec)));
					log_file << "GMT: " << buffer << dec << shared_memory.log_entry[total_counter].tval.tv_usec << endl;

					log_file << "Call stack: " << endl;
					for(int  k = 0; k < shared_memory.log_entry[total_counter].backtrace_length; k++){

						log_file << shared_memory.log_entry[total_counter].call_stack[k]<< " --- ";
						log_file << Processes[PID].Find_function_name((uint64_t)shared_memory.log_entry[total_counter].call_stack[k]) << endl;
					}
					total_memory_leaked += shared_memory.log_entry[total_counter].size;
				}
			}
			else if(shared_memory.log_entry[total_counter].valid && shared_memory.log_entry[total_counter].type == free_func){
				free_counter++;
			}
		}

		cout << endl << "PID: " << std::dec << PID << endl;
		log_file<< endl << "PID: " << std::dec << PID << endl;
		cout <<"Total memory allocated: " << std::dec << total_memory_allocated << " bytes" << endl;
		log_file <<"Total memory allocated: " << std::dec << total_memory_allocated << " bytes" << endl;
		cout << "Total memory freed: " << std::dec << total_memory_freed << " bytes" << endl;
		log_file << "Total memory freed: " << std::dec << total_memory_freed << " bytes" << endl;
		cout << "Total memory leaked yet: " << std::dec << total_memory_leaked << " bytes" << endl;
		log_file << "Total memory leaked yet: " << std::dec << total_memory_leaked << " bytes" << endl;
		cout << "Total number of mallocs: " << std::dec << malloc_counter << endl;
		log_file << "Total number of mallocs: " << std::dec << malloc_counter << endl;
		cout << "Total number of frees: " << std::dec << free_counter << endl<<endl;
		log_file << "Total number of frees: " << std::dec << free_counter << endl<<endl;

		log_file.close();
	}
	else{
		log_file << endl << "NO DATA AVAILABLE, shared memory has not been initialized!" << endl;
		log_file.close();
		cout << endl << "Process " << PID << " cannot be analyzed, no shared memory has been initialized" <<endl;
	}
}

void Memory_Profiler::Analyze_all_process(){

	map<const pid_t, Process_handler>::iterator it;
	for (it = Processes.begin(); it != Processes.end(); it++) {
		Analyze_process(it->first);
	}
}

