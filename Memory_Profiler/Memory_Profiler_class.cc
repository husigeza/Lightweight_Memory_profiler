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
				cout << "Process NOT added,"
						""
						" PID: " << dec << PID << endl;
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


	map<const pid_t, Process_handler>::iterator it = Processes.find(PID);

	if(it->second.Get_alive() == true){
		if(it->second.Get_profiled() == true){
			cout << "Process " << dec << PID << " is already under profiling!"<< endl;
			return;
		}
		if(it->second.Is_shared_memory_initialized() == false){
			cout << "Shared memory has not been initialized, initializing it... Process: " << dec << PID << endl;
			if(it->second.Init_shared_memory() == false){
				cout << "Shared memory init unsuccessful, not added to profiled state." << endl;
				return;
			}
			cout << "Shared memory init successful!" << endl;
		}
		else {
			cout << "Shared memory has been initialized for process: " << dec << PID << endl;
		}
			it->second.Set_profiled(true);
			it->second.Start_Stop_profiling();
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

	if(Processes[PID].Get_profiled() == false){
		cout << "Process " << dec << PID << "is not under profiling!"<< endl;
		return;
	}

	if(Processes[PID].Remap_shared_memory()){
		Processes[PID].Start_Stop_profiling();
		Processes[PID].Set_profiled(false);
		cout << "Process " << dec << PID << " removed from profiled state!" << endl;
	}
	else{
		cout << "Process " << dec << PID << " NOT removed from profiled state!" << endl;
	}



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
				//cout << "Failed reading the FIFO" << endl;
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

bool Memory_Profiler::Process_analyze_ready(const pid_t PID){

	ofstream log_file;
	log_file.open(("Analyzation_output_"+ Processes[PID].PID_string + ".txt").c_str(), ios::app);

	try{
		if(Processes[PID].Is_shared_memory_initialized() == true){
			if(Processes[PID].Get_profiled() == true){
				log_file << "Process "<< dec << PID <<" is still being profiled, stop profiling first! "<< endl;
				cout << "Process "<< dec << PID <<" is still being profiled, stop profiling first! "<< endl;
				throw false;
			}
			const memory_profiler_sm_object_class &shared_memory = *Processes[PID].Get_shared_memory();
			if(shared_memory.log_count == 0){
				cout << endl << "shared_memory log_count = 0, no data to analyze!" << endl;
				log_file << endl << "shared_memory log_count = 0, no data to analyze!" << endl;
				throw false;
			}
		}
		else{
			cout << endl << "NO DATA AVAILABLE, shared memory has not been initialized!" << endl;
			log_file << endl << "NO DATA AVAILABLE, shared memory has not been initialized!" << endl;
			throw false;
		}
		throw true;
	}
	catch(bool b){
		log_file.close();
		if(b == true){
			return true;
		}
		else {
			return false;
		}
	}
}


void Memory_Profiler::Analyze_process(const pid_t PID){

	Analyze_process_memory_leak(PID);
	Analyze_process_dummy_free(PID);

}
void Memory_Profiler::Analyze_process_dummy_free(const pid_t PID){

	if(Process_analyze_ready(PID) == false){
		return;
	}

	ofstream log_file;
	log_file.open(("Analyzation_output_"+ Processes[PID].PID_string + ".txt").c_str(), ios::app);

	cout << endl << endl <<"Running dummy free searching for Process: "<< dec << PID << endl;
	log_file << endl << endl <<"Running dummy free searching for Process: "<< dec << PID << endl;

	vector<uint64_t> malloc_vector;
	vector<uint64_t>::iterator it;
	const memory_profiler_sm_object_class &shared_memory = *Processes[PID].Get_shared_memory();



	//Need to count with shared_memory.log_count-1 because shared_memory.log_count shows a bigger value with 1 than the real number of elements
	for(unsigned long int total_counter = 0; total_counter <= shared_memory.log_count-1; total_counter++){
		if(shared_memory.log_entry[total_counter].valid && shared_memory.log_entry[total_counter].type == malloc_func){
			malloc_vector.push_back(shared_memory.log_entry[total_counter].address);
		}
		else if(shared_memory.log_entry[total_counter].valid && shared_memory.log_entry[total_counter].type == free_func){
			it = find(malloc_vector.begin(), malloc_vector.end(), shared_memory.log_entry[total_counter].address);
			if(it == malloc_vector.end()){
				cout << endl <<"Address 0x"<< hex << shared_memory.log_entry[total_counter].address << " is freed but has not been allocated!"<< endl;
				log_file << endl <<"Address 0x"<< hex << shared_memory.log_entry[total_counter].address << " is freed but has not been allocated!"<< endl;

				Processes[PID].Print_backtrace(total_counter,log_file);
			}
			else{
				malloc_vector.erase(it);
			}
		}
	}

	cout << "Finished!" << endl;
	log_file << "Finished!" << endl;

	log_file.close();

}

void Memory_Profiler::Analyze_process_memory_leak(const pid_t PID){


	if(Process_analyze_ready(PID) == false){
		return;
	}

	ofstream log_file;
	log_file.open(("Analyzation_output_"+ Processes[PID].PID_string + ".txt").c_str(), ios::app);

	cout << endl << endl <<"Running memory leak analyzation for Process: "<< dec << PID << endl;
	log_file << endl << endl <<"Running memory leak analyzation for Process: "<< dec << PID << endl;

	const memory_profiler_sm_object_class &shared_memory = *Processes[PID].Get_shared_memory();

	unsigned long int total_counter = 0;
	unsigned long int total_counter_2 = 0;

	unsigned long int malloc_counter = 0;
	unsigned long int free_counter = 0;
	unsigned long int total_memory_allocated = 0;
	unsigned long int total_memory_freed = 0;
	unsigned long int address = 0;
	unsigned long int total_memory_leaked = 0;

	//Need to count with shared_memory.log_count-1 because shared_memory.log_count shows a bigger value with 1 than the real number of elements
	for(total_counter = 0; total_counter <= shared_memory.log_count-1; total_counter++){

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

void Memory_Profiler::Analyze_all_process(){

	map<const pid_t, Process_handler>::iterator it;
	for (it = Processes.begin(); it != Processes.end(); it++) {
		Analyze_process(it->first);
	}
}

void Memory_Profiler::Create_new_analyzer(shared_ptr<Analyzer> analyzer){

	Analyzers_vector.push_back(analyzer);
}

void Memory_Profiler::Create_new_filter_cli(unsigned long size_p, string operation_p){

	try{
		Create_new_filter(make_shared<Size_filter>(size_p,operation_p));
	}
	catch(const bool v){
		if( v == false){
			cout << "Filter not added: bad operation type." << endl;
			cout << "Possibilities: equal, less, bigger" << endl;
		}
	}
}

void Memory_Profiler::Create_new_filter(shared_ptr<Filter_class> filter){

	Filters_vector.push_back(filter);
}

void Memory_Profiler::Create_new_pattern(string name){

	auto pattern = Find_pattern_by_name(name);

	if(pattern != Patterns_vector.end()){
		cout << "Pattern with that name already exists!" << endl;
	}
	else{
		Patterns_vector.push_back( move(unique_ptr<Pattern> (new Pattern(name))) );
	}
}

void Memory_Profiler::Print_patterns() const{

	unsigned int i;
	for(auto &a : Patterns_vector){
		cout <<"Index: " << dec << i << endl;
		(*a).Print();
		cout << "Analyzers in the pattern: " << endl;
		(*a).Print_analyzers();
		cout << "Filters in the pattern: " << endl;
		(*a).Print_filters();
		i++;
	}
}

void Memory_Profiler::Print_filters() const{

	unsigned int i;
	for(auto &a : Filters_vector){
		cout <<"Index: " << dec << i << endl;
		(*a).Print();
		i++;
	}
}

void Memory_Profiler::Print_analyzers() const{

	unsigned int i;
	for(auto &a : Analyzers_vector){
		cout <<"Index: " << dec << i << endl;
		(*a).Print();
		i++;
	}
}

bool operator==(unique_ptr<Pattern> &p, string pattern_name){
	if((*p).Get_name() == pattern_name) return true;
	else return false;
}


vector< unique_ptr<Pattern> >::iterator Memory_Profiler::Find_pattern_by_name(string Pattern_name){

	return find(Patterns_vector.begin(),Patterns_vector.end(),Pattern_name);

}

void Memory_Profiler::Add_analyzer_to_pattern(unsigned int analyzer_index,unsigned int pattern_index){

	if(analyzer_index >= Analyzers_vector.size()){
		cout << "Wrong Analyzer ID" << endl;
	}
	else if (pattern_index >= Patterns_vector.size()){
		cout << "Wrong Pattern ID" << endl;
	}
	else{
		Patterns_vector[pattern_index]->Analyzer_register(Analyzers_vector[analyzer_index]);
	}
}

void Memory_Profiler::Add_analyzer_to_pattern_by_name(unsigned int analyzer_index,string pattern_name){

	auto pattern = Find_pattern_by_name(pattern_name);

	if(analyzer_index >= Analyzers_vector.size()){
		cout << "Wrong Analyzer ID" << endl;
	}
	else if (pattern == Patterns_vector.end()){
		cout << "Wrong Pattern name" << endl;
	}
	else{
		(**pattern).Analyzer_register(Analyzers_vector[analyzer_index]);
	}
}

void Memory_Profiler::Add_filter_to_pattern(unsigned int filter_index,unsigned int pattern_index){

	if(filter_index >= Filters_vector.size()){
		cout << "Wrong Filter ID" << endl;
	}
	else if (pattern_index >= Patterns_vector.size()){
		cout << "Wrong Pattern ID" << endl;
	}
	else{
		Patterns_vector[pattern_index]->Filter_register(Filters_vector[filter_index]);
	}
}


void Memory_Profiler::Add_filter_to_pattern_by_name(unsigned int filter_index,string pattern_name){

	auto pattern = Find_pattern_by_name(pattern_name);

	if(filter_index >= Filters_vector.size()){
		cout << "Wrong Filter ID" << endl;
	}
	else if (pattern == Patterns_vector.end()){
		cout << "Wrong Pattern name" << endl;
	}
	else{
		(**pattern).Filter_register(Filters_vector[filter_index]);
	}
}

void  Memory_Profiler::Run_pattern_all_process(unsigned int pattern_index){

	for(auto &process : Processes){
		if (pattern_index >= Patterns_vector.size()){
			cout << "Wrong pattern ID" << endl;
		}
		else {
			Run_pattern(pattern_index,process.second);
		}
	}
}

void Memory_Profiler::Run_pattern(unsigned int pattern_index, Process_handler &process){

	if (pattern_index >= Patterns_vector.size()){
			cout << "Wrong pattern ID" << endl;
		}
	else{
		Patterns_vector[pattern_index]->Run_analyzers(process);
	}
}

