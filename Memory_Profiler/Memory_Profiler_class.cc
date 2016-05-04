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

#include "Memory_Profiler_handler_template.h"
#include "Memory_Profiler_class.h"


using namespace std;

Memory_Profiler::Memory_Profiler(string fifo_path, string overload_fifo_path) {

	if(sem_init(&save_sem,0,1) == -1){
		cout << "Failed initializing semaphore for shared memory reading" << "errno: " << errno << endl;
		return;
	}


	this->fifo_path = fifo_path;

	if (mkfifo(this->fifo_path.c_str(), 0666) == -1) {

		if (errno == EEXIST) {
			//cout << "FIFO already exists" << endl;
		} else {
			cout << "Failed creating FIFO" << "errno: " << errno << endl;
			return;
		}
	}
	else {
	//cout << "FIFO is created" << endl;
	}

	mem_prof_fifo = open(fifo_path.c_str(), O_RDONLY | O_NONBLOCK);

	mem_prof_overload_fifo_path = overload_fifo_path;

	if (mkfifo(overload_fifo_path.c_str(), 0666) == -1) {
		if (errno == EEXIST) {
			//cout << "overload FIFO already exists" << endl;
		} else {
			cout << "Failed creating FIFO" << "errno: " << errno << endl;
			return;
		}
	}
	else {
	//cout << "overload FIFO is created" << endl;
	}
	mem_prof_overload_fifo = 0;
	//mem_prof_overload_fifo = open(overload_fifo_path.c_str(), O_RDONLY /*| O_NONBLOCK*/);

}

Memory_Profiler::~Memory_Profiler() {

	close(mem_prof_fifo);
	close(mem_prof_overload_fifo);
	unlink(fifo_path.c_str());
	unlink(mem_prof_overload_fifo_path.c_str());
	Processes.clear();
}

void Memory_Profiler::Read_FIFO() {

	vector<pid_t> alive_processes;
	pid_t pid;
	string buffer;
	size_t buff_size = 7;
	int res;
	map<const pid_t, template_handler<Process_handler> >::iterator it;

	while ((res = read(mem_prof_fifo, (char*)buffer.c_str(), buff_size)) != 0) {
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
		if(it->second.object->Get_alive() == true){
			if(find(alive_processes.begin(), alive_processes.end(), it->first) == alive_processes.end()) {
				Processes[it->first].object->Set_alive(false);
				cout << "Process " << dec << it->first <<" is no more alive!" << endl;
				Remove_process_from_profiling(it->first);
				try{
					Save_process_shared_memory(it->first);
				}
				catch(ofstream::failure &e){
					cout << e.what() << endl;
					cout << "Shared memory saving failed!" << endl;
				}
			}
			else {
				Processes[it->first].object->Set_alive(true);
			}
		}
	}
	alive_processes.clear();
}

void Memory_Profiler::Read_overload_FIFO(){

	pid_t pid;
	int res;
	string buffer;
	size_t buff_size = 7;

	mem_prof_overload_fifo = open(mem_prof_overload_fifo_path.c_str(), O_RDONLY /*| O_NONBLOCK*/);

	while ((res = read(mem_prof_overload_fifo, (char*)buffer.c_str(), buff_size)) != 0) {

		if (res > 0) {
			pid = atol( buffer.c_str() );
			try{
				Save_process_shared_memory(pid);
			}
			catch(ofstream::failure &e){
				cout << e.what() << endl;
				cout << "Shared memory saving failed!" << endl;
			}
		}
	}
}

bool Memory_Profiler::Add_Process_to_list(const pid_t PID) {


	if (Processes.find(PID) == Processes.end()) {

		try{

			Processes.insert(make_pair(PID,template_handler<Process_handler> (*(new Process_handler(PID)))));
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


	map<const pid_t, template_handler<Process_handler> >::iterator it = Processes.find(PID);

	if(it->second.object->Get_alive() == true){
		if(it->second.object->Get_profiled() == true){
			cout << "Process " << dec << PID << " is already under profiling!"<< endl;
			return;
		}
		if(it->second.object->Is_shared_memory_initialized() == false){
			//cout << "Shared memory has not been initialized, initializing it... Process: " << dec << PID << endl;
			if(it->second.object->Init_shared_memory() == false){
				cout << "Shared memory init unsuccessful, not added to profiled state." << endl;
				return;
			}
			//cout << "Shared memory init successful!" << endl;
		}
		else {
			//cout << "Shared memory has been initialized for process: " << dec << PID << endl;
		}
		it->second.object->Set_profiled(true);
		it->second.object->Start_Stop_profiling();
		cout << "Process " << dec << PID << " added to profiled state" << endl;
	}
	else{
		cout << "Process " << PID << " is not alive, not added to profiled state.!" << endl;
	}
}

void Memory_Profiler::Add_all_process_to_profiling() {

	map<const pid_t, template_handler<Process_handler> >::iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {

		Add_process_to_profiling(it->first);
	}
}


void Memory_Profiler::Remove_process_from_profiling(const pid_t PID){

	if(Processes[PID].object->Get_profiled() == false){
		cout << "Process " << dec << PID << " is not under profiling!"<< endl;
		return;
	}
		Processes[PID].object->Start_Stop_profiling();
		Processes[PID].object->Set_profiled(false);
		cout << "Process " << dec << PID << " removed from profiled state!" << endl;
}

void Memory_Profiler::Remove_all_process_from_profiling(){

	map<const pid_t, template_handler<Process_handler> >::iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {

		Remove_process_from_profiling(it->first);
	}
}



void Memory_Profiler::Start_stop_profiling(const pid_t PID){

	Processes[PID].object->Start_Stop_profiling();
}


void Memory_Profiler::Start_stop_profiling_all_processes(){

	map<const pid_t, template_handler<Process_handler> >::iterator it;

	for (it = Processes.begin(); it != Processes.end(); it++) {

		if (it->second.object->Get_profiled() == true) {
			Start_stop_profiling(it->first);
		}
	}
}



void Memory_Profiler::Save_process_shared_memory(pid_t PID){

	if (Processes.find(PID) != Processes.end()) {

		sem_wait(&save_sem);

		template_handler<Process_handler> process = Processes[PID];


		//cout << endl << "Saving " << dec << PID <<" process shm..." << endl;

		if(process.object->Is_shared_memory_initialized()){


			try{
				// B segment's "profiled" flag means: the profiler process has read the currently passive segment's content
				// Needed when the user program crashed, and did not call swap_shared_pointers function, which sets the "active" flags
				// In this case B segment's profiler flag is true, thus need to read entries from segment which active flag is true
				if(process.object->memory_profiler_struct_B->profiled == false){
					if(process.object->memory_profiler_struct_A->active == false){
						//cout << " Saving from segment A..." << endl;
						//cout << " segment A log count: " << process.object->memory_profiler_struct_A->log_count << endl;
						process.object->total_entry_number += process.object->memory_profiler_struct_A->log_count;
						process.object->memory_profiler_struct_A->write_to_binary_file(process.object->PID_string,process.object->total_entry_number);
					}
					else if(process.object->memory_profiler_struct_B->active == false){
						//cout << " Saving from segment B..." << endl;
						//cout << " segment B log count: " << process.object->memory_profiler_struct_B->log_count << endl;
						process.object->total_entry_number += process.object->memory_profiler_struct_B->log_count;
						process.object->memory_profiler_struct_B->write_to_binary_file(process.object->PID_string,process.object->total_entry_number);
					}
					else{
						cout <<"Shared memory is in inconsistent state, don't parse it!" << endl;
					}

					process.object->memory_profiler_struct_B->profiled = true;
					//cout << dec << PID << "  log count:  " << process.object->total_entry_number << endl<<endl;
				}
				else{
					if(process.object->memory_profiler_struct_A->active == true){
						//cout << " Saving from segment A..." << endl;
						//cout << " segment A log count: " << process.object->memory_profiler_struct_A->log_count << endl;
						process.object->total_entry_number += process.object->memory_profiler_struct_A->log_count;
						process.object->memory_profiler_struct_A->write_to_binary_file(process.object->PID_string,process.object->total_entry_number);
					}
					else if(process.object->memory_profiler_struct_B->active == true){
						//cout << " Saving from segment B..." << endl;
						//cout << " segment B log count: " << process.object->memory_profiler_struct_B->log_count << endl;
						process.object->total_entry_number += process.object->memory_profiler_struct_B->log_count;
						process.object->memory_profiler_struct_B->write_to_binary_file(process.object->PID_string,process.object->total_entry_number);
					}
					else{
						cout <<"Shared memory is in inconsistent state, don't parse it!" << endl;
					}

					//cout << dec << PID << "  log count:  " << process.object->total_entry_number << endl <<endl;
				}
			}
			catch(ofstream::failure &e){
				if(sem_post(&save_sem) == -1){
					cout <<"Error in sem_post, errno: " << errno << endl;
				}
				throw e;
			}
		}

		if(sem_post(&save_sem) == -1){
			cout <<"Error in sem_post, errno: " << errno << endl;
		}
	}
	else {
		//cout << "Process " << dec << PID << " has not been added, cannot read shared memory" << endl;
	}

}

void Memory_Profiler::Create_new_pattern(string name){

	vector< template_handler<Pattern> >::iterator pattern = Find_pattern_by_name(name);

	if(pattern != Patterns_vector.end()){
		cout << "Pattern with that name already exists!" << endl;
	}
	else{
		Patterns_vector.push_back( template_handler<Pattern> (*(new Pattern(name))));
	}
}

vector< template_handler<Pattern> >::iterator Memory_Profiler::Find_pattern_by_name(string Pattern_name){

	return find(Patterns_vector.begin(),Patterns_vector.end(),Pattern_name);

}

void Memory_Profiler::Print_patterns() const{

	unsigned int i = 0;
	for(vector< template_handler<Pattern> >::const_iterator pattern = Patterns_vector.begin();pattern != Patterns_vector.end();pattern++){
		cout << endl;
		cout <<"Index: " << dec << i << endl;
		pattern->object->Print();
		i++;
	}
}

void Memory_Profiler::Print_pattern(string pattern_name){

	vector<template_handler<Pattern> >::iterator pattern = Find_pattern_by_name(pattern_name);

		if (pattern == Patterns_vector.end()){
			cout << "Wrong pattern name!" << endl;
		}
		else{
			pattern->object->Print();
		}
}

void Memory_Profiler::Create_new_analyzer(Analyzer& analyzer){

	Analyzers_vector.push_back(analyzer);
}

void Memory_Profiler::Remove_analyzer(unsigned int analyzer_index){

	if(analyzer_index >= Analyzers_vector.size()){
		cout << "Wrong analyzer index" << endl;
	}
	else{
		Analyzers_vector.erase(Analyzers_vector.begin() + analyzer_index);
	}
}

void Memory_Profiler::Print_analyzers() const{

	unsigned int i = 0;
	for(vector< template_handler<Analyzer> >::const_iterator analyzer = Analyzers_vector.begin();analyzer != Analyzers_vector.end();analyzer++){
		cout << endl;
		cout <<"Index: " << dec << i << endl;
		analyzer->object->Print();
		i++;
	}
}

void Memory_Profiler::Print_analyzer(unsigned int index) const{
	if(index < Analyzers_vector.size()){
		Analyzers_vector[index].object->Print();
	}
	else{
		cout <<"Wrong ID!"<< endl;
	}
}

void Memory_Profiler::Create_new_filter(Filter& filter){

	Filters_vector.push_back(template_handler<Filter>(filter));
}

void Memory_Profiler::Create_new_size_filter_cli(unsigned long size_p, string operation_p){

	try{
		Create_new_filter (*(new Size_filter(size_p,operation_p)));
	}
	catch(const bool v){
		if( v == false){
			cout << "Filter not added: bad operation type." << endl;
			cout << "Possibilities: equal, less, bigger" << endl;
		}
	}
}

void Memory_Profiler::Create_new_time_filter_cli(string time,__suseconds_t usec,string time_type, string operation_p){

	try{
			Create_new_filter (*(new Time_filter(time,usec,time_type,operation_p)));
		}
		catch(const bool v){
			if( v == false){
				cout << "Filter not added: bad operation type or bad time format or type." << endl;
				cout << "Possibilities for operations: equal, less, bigger" << endl;
				cout << "Time format: %Y-%m-%d-%H:%M:%S" << endl;
				cout << "Possibilities for time type: after, before" << endl;
			}
		}
}

void Memory_Profiler::Remove_filter(unsigned int filter_index){

	if(filter_index >= Filters_vector.size()){
		cout << "Wrong filter index" << endl;
	}
	else{
		Filters_vector.erase(Filters_vector.begin() + filter_index);
	}
}

void Memory_Profiler::Print_filters() const{

	unsigned int i = 0;
	for(vector< template_handler<Filter> >::const_iterator filter = Filters_vector.begin();filter != Filters_vector.end();filter++){
		cout << endl;
		cout <<"Index: " << dec << i << endl;
		filter->object->Print();
		i++;
	}
}

void Memory_Profiler::Print_filter(unsigned int index) const{
	if(index < Filters_vector.size()){
		Filters_vector[index].object->Print();
	}
	else{
		cout <<"Wrong ID!"<< endl;
	}
}

void Memory_Profiler::Add_analyzer_to_pattern(unsigned int analyzer_index,unsigned int pattern_index){

	if(analyzer_index >= Analyzers_vector.size()){
		cout << "Wrong Analyzer ID" << endl;
	}
	else if (pattern_index >= Patterns_vector.size()){
		cout << "Wrong Pattern ID" << endl;
	}
	else{
		Patterns_vector[pattern_index].object->Analyzer_register(Analyzers_vector[analyzer_index]);
		Analyzers_vector[analyzer_index].object->Pattern_register(Patterns_vector[pattern_index]);
	}
}

void Memory_Profiler::Add_analyzer_to_pattern_by_name(unsigned int analyzer_index,string pattern_name){

	vector<template_handler<Pattern> >::iterator pattern = Find_pattern_by_name(pattern_name);

	if (pattern == Patterns_vector.end()){
		cout << "Wrong pattern name!" << endl;
	}
	else if(analyzer_index >= Analyzers_vector.size()){
		cout << "Wrong Analyzer ID" << endl;
	}
	else{
		pattern->object->Analyzer_register(Analyzers_vector[analyzer_index]);
		Analyzers_vector[analyzer_index].object->Pattern_register(*pattern);
	}
}

void Memory_Profiler::Add_analyzer_all_to_pattern_by_name(string pattern_name){

	vector<template_handler<Pattern> >::iterator pattern = Find_pattern_by_name(pattern_name);
	vector<template_handler<Analyzer> >::iterator analyzer;

	if (pattern == Patterns_vector.end()){
		cout << "Wrong pattern name!" << endl;
	}
	else{
		for(analyzer = Analyzers_vector.begin();analyzer != Analyzers_vector.end(); analyzer++){
			pattern->object->Analyzer_register(*analyzer);
			analyzer->object->Pattern_register(*pattern);
		}
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
		Patterns_vector[pattern_index].object->Filter_register(Filters_vector[filter_index]);
		Filters_vector[filter_index].object->Pattern_register(Patterns_vector[pattern_index]);
	}
}


void Memory_Profiler::Add_filter_to_pattern_by_name(unsigned int filter_index,string pattern_name){

	vector< template_handler<Pattern> >::iterator pattern = Find_pattern_by_name(pattern_name);

	if(filter_index >= Filters_vector.size()){
		cout << "Wrong Filter ID" << endl;
	}
	else if (pattern == Patterns_vector.end()){
		cout << "Wrong Pattern name" << endl;
	}
	else{
		pattern->object->Filter_register(Filters_vector[filter_index]);
		Filters_vector[filter_index].object->Pattern_register(*pattern);
	}
}

void Memory_Profiler::Remove_analyzer_from_pattern(unsigned int analyzer_index,unsigned int pattern_index){

	if (pattern_index >= Patterns_vector.size()){
			cout << "Wrong Pattern ID" << endl;
	}
	else if(analyzer_index >= Patterns_vector[pattern_index].object->Get_number_of_analyzers()){
		cout << "Wrong Analyzer ID" << endl;
	}
	else{
		Patterns_vector[pattern_index].object->Analyzer_deregister(analyzer_index);
	}
}



void Memory_Profiler::Remove_analyzer_from_pattern_by_name(unsigned int analyzer_index,string pattern_name){

	vector< template_handler<Pattern> >::iterator pattern = Find_pattern_by_name(pattern_name);

	pattern->object->Analyzer_deregister(analyzer_index);

}


void Memory_Profiler::Remove_filter_from_pattern(unsigned int filter_index,unsigned int pattern_index){

	if (pattern_index >= Patterns_vector.size()){
			cout << "Wrong Pattern ID" << endl;
	}
	else if(filter_index >= Patterns_vector[pattern_index].object->Get_number_of_analyzers()){
		cout << "Wrong Filter ID" << endl;
	}
	else{
		Patterns_vector[pattern_index].object->Filter_deregister(filter_index);
	}
}

void Memory_Profiler::Remove_filter_from_pattern_by_name(unsigned int filter_index,string pattern_name){

	vector< template_handler<Pattern> >::iterator pattern = Find_pattern_by_name(pattern_name);

	pattern->object->Filter_deregister(filter_index);
}

void Memory_Profiler::Run_pattern(unsigned int pattern_index, pid_t PID){


	if (pattern_index >= Patterns_vector.size()){
			cout << "Wrong pattern ID" << endl;
	}
	else{
		try{
			Processes[PID].object->Read_shared_memory();
			Patterns_vector[pattern_index].object->Run_analyzers(Processes[PID]);
			delete Processes[PID].object->memory_profiler_struct;
		}
		catch(ifstream::failure &e){
			cout << e.what() << endl;
			cout << "Cannot run pattern for Process " << dec << PID << endl;
		}
	}
}

void Memory_Profiler::Run_pattern(string pattern_name, pid_t PID){

	vector< template_handler<Pattern> >::iterator pattern = Find_pattern_by_name(pattern_name);

	if (pattern == Patterns_vector.end()){
		cout << "Wrong pattern name" << endl;
	}
	else{
		try{
			Processes[PID].object->Read_shared_memory();
			pattern->object->Run_analyzers(Processes[PID]);
			// In case of exception, memory_profiler_struct is deleted from exception handler in "Run_analyzers"
			delete Processes[PID].object->memory_profiler_struct;
		}
		catch(ifstream::failure &e){
			cout << "Exception type: " << e.what() << endl;
			cout << "Problems with reading from binary files..." << endl;
			cout << "Cannot run pattern for Process " << dec << PID << endl;
		}
	}
}

void  Memory_Profiler::Run_pattern_all_process(unsigned int pattern_index){


	for(map<pid_t const,template_handler<Process_handler> >::iterator process = Processes.begin();process != Processes.end();process++){
		Run_pattern(pattern_index,process->first);
	}

}

void Memory_Profiler::Run_pattern_all_process(string name){

	for(map<pid_t const,template_handler<Process_handler> >::iterator process = Processes.begin();process != Processes.end();process++){
		Run_pattern(name,process->first);
	}
}

