#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

#include "Memory_Profiler_process.h"
#include "Memory_Profiler_handler_template.h"

#include "Memory_Profiler_analyzer.h"
#include "Memory_Profiler_pattern.h"



using namespace std;

bool operator==(template_handler<Analyzer> analyzer_1, const template_handler<Analyzer> analyzer_2){
	if(analyzer_1.object == analyzer_2.object) return true;
	else return false;
}

Analyzer::Analyzer(){
	type = unknown_analyzer;
	type_string = "Unknown type";
}

Analyzer::~Analyzer(){

	for(vector<template_handler<Pattern> >::iterator pattern = Pattern_vector.begin();pattern != Pattern_vector.end();pattern++){
		pattern->object->Analyzer_deregister(this);
	}
}

Analyzer::Analyzer(const Analyzer &obj){

	type = obj.type;
	type_string = obj.type_string;
	process = obj.process;
	Pattern_vector = obj.Pattern_vector;

}
Analyzer& Analyzer::operator=(const Analyzer &obj){

	type = obj.type;
	type_string = obj.type_string;
	process = obj.process;
	Pattern_vector = obj.Pattern_vector;

	return *this;
}

unsigned int Analyzer::GetType() const{
	return type;
}

string Analyzer::Get_type_string() const{
	return type_string;
}

void Analyzer::Pattern_register(template_handler<Pattern> pattern){

	vector<template_handler<Pattern> >::iterator it = find(Pattern_vector.begin(),Pattern_vector.end(),pattern);

	if(it == Pattern_vector.end()){
		Pattern_vector.push_back(pattern);
	}
	else{
		//cout << "Analyzer has been already added to the pattern!" << endl;
	}
}

void Analyzer::Pattern_deregister(string name){
	vector<template_handler<Pattern> >::iterator it = find(Pattern_vector.begin(), Pattern_vector.end(), name);
	if(it != Pattern_vector.end()){
		Pattern_vector.erase(it);
	}
	else {
		cout << "Pattern " << name << " has not been bounded to this analyzer" << endl;
	}
}


void Analyzer::Print() const{
	cout << "   type: " << type_string << endl;
	cout << "   Analyzer is bounded to patterns:" << endl;
	for(vector<template_handler<Pattern> >::const_iterator pattern = Pattern_vector.begin();pattern != Pattern_vector.end();pattern++){
		cout <<"      "<< pattern->object->Get_name() << endl;
	}
}

void Analyzer::Start(template_handler<Process_handler> process){

	ofstream log_file;
	log_file.open(("Analyzation_output_"+ process.object->PID_string + ".txt").c_str(), ios::app);

	cout << "Analyzer " << type_string << " starting..." << endl;
	log_file << "Analyzer " << type_string << " starting..." << endl;

	this->process = process;

	log_file.close();
}

void Analyzer::Stop(){

	ofstream log_file;
	log_file.open(("Analyzation_output_"+ process.object->PID_string + ".txt").c_str(), ios::app);

	cout << endl << "Analyzer "<< type_string << " has finished!" << endl;
	log_file << endl << "Analyzer "<< type_string << " has finished!" << endl;

	log_file.close();

}

Print_Analyzer::Print_Analyzer(){
	type = print_analyzer;
	type_string = "Print analyzer";
}

void Print_Analyzer::Analyze(vector<template_handler< memory_profiler_sm_object_log_entry_class> > entries) const {

	for(vector<template_handler< memory_profiler_sm_object_log_entry_class> >::iterator entry = entries.begin();entry != entries.end();entry++){
		entry->object->Print(process);
	}
}

Memory_Leak_Analyzer::Memory_Leak_Analyzer(){
	type = leak_analyzer;
	type_string = "Memory Leak analyzer";
}

void Memory_Leak_Analyzer::Analyze(vector<template_handler< memory_profiler_sm_object_log_entry_class> > entries) const {

	ofstream log_file;
	log_file.open(("Analyzation_output_"+ process.object->PID_string + ".txt").c_str(), ios::app);

	cout << endl << endl <<"Running memory leak analyzation for Process: "<< process.object->PID_string<< endl;
	log_file << endl << endl <<"Running memory leak analyzation for Process: "<< process.object->PID_string<< endl;

	unsigned long long int counter = 0;
	unsigned long long int entries_size = entries.size();

	unsigned long long int malloc_counter = 0;
	unsigned long long int free_counter = 0;
	unsigned long long int realloc_counter = 0;
	unsigned long long int total_memory_allocated = 0;
	unsigned long long int total_memory_freed = 0;
	unsigned long long int total_memory_leaked = 0;
	unsigned long int address = 0;
	unsigned long long int size_to_free = 0;
	unsigned long long int realloc_size = 0;

	vector<template_handler< memory_profiler_sm_object_log_entry_class> >::iterator it;
	for(it = entries.begin(); it != entries.end(); it++){

		++counter;
		cout <<"Total entries processed: " << dec << counter << " / " << dec << entries_size
				<< " (" << dec << (int)((double)counter/((double)entries_size)*100) << "%)" << '\r';

		// Catch only mallocs and callocs here, because isf realloc is used as malloc its type is malloc_func
		if(it->object->valid && (it->object->type == malloc_func || it->object->type == calloc_func )){

			total_memory_allocated += it->object->size;
			malloc_counter++;
			address = it->object->address;
			size_to_free = it->object->size;

			vector<template_handler< memory_profiler_sm_object_log_entry_class> >::iterator it2 = it;
			// Iterate through the remaining items looking for free or realloc
			for(; it2 != entries.end(); it2++){
				if(it2->object->valid){
					if(it2->object->address == address || it2->object->realloc_address == address){
						if (it2->object->type == free_func){
							total_memory_freed += size_to_free;
							size_to_free = 0;
							break;
						}
						else if(it2->object->type == realloc_func){
							// If size in realloc and size from malloc/calloc do not equal
							// it means the allocated space is expanded (reduced) with (new size - original size) bytes
							if(it2->object->size != size_to_free){
								total_memory_allocated -= size_to_free;
								total_memory_allocated += it2->object->size;
								// The realloc will contain the new allocated size
								size_to_free = it2->object->size;
							}
							 /*
							  * In case of realloc both address field is interpreted:
							  * address: realloc returns with this
							  * realloc_address: pointer passed to realloc
							  * If those 2 do not equal it means realloc returned with a different address
							  * thus the object at the original place is moved to the new place
							  * and freed from the original place.
							  * In this case the newly given address becomes the "original" address.
							  *
							  */
							if(it2->object->realloc_address != it2->object->address){
								address = it2->object->address;
							}
							realloc_counter++;
						}
					}
				}
			}
			if(it2 == entries.end()){

				size_to_free = 0;

				if(address == it->object->address){
				log_file << endl << "Memory 0x" << std::hex << it->object->address << " has not been freed yet!" << endl;
				}
				else {
					log_file << endl << "Memory 0x" << std::hex << it->object->address << " has been freed, however " << endl
							<< " it has been changed (with realloc) to: 0x" << address << " which has not been freed yet! " << endl;
				}

				it->object->Print(process,log_file);

				/*char buffer[30];
				strftime(buffer,30,"%m-%d-%Y %T.",gmtime(&(it->object->tval_before.tv_sec)));
				log_file << "GMT before: " << buffer << dec << it->object->tval_before.tv_usec << endl;
				strftime(buffer,30,"%m-%d-%Y %T.",gmtime(&(it->object->tval_after.tv_sec)));
				log_file << "GMT after: " << buffer << dec << it->object->tval_after.tv_usec << endl;


				it->object->size

				log_file << "Call stack: " << endl;
				for(int  k = 0; k < it->object->backtrace_length; k++){

					log_file << it->object->call_stack[k]<< " --- ";
					log_file << process.object->Find_function_name((uint64_t)it->object->call_stack[k]) << endl;
				}*/
			}
		}
		else if(it->object->valid && it->object->type == free_func){
			free_counter++;
		}
	}

	total_memory_leaked = total_memory_allocated - total_memory_freed;

	cout << endl << "PID: " << process.object->PID_string << endl;
	log_file<< endl << "PID: " << process.object->PID_string << endl;
	cout <<"Total memory allocated: " << std::dec << total_memory_allocated << " bytes" << endl;
	log_file <<"Total memory allocated: " << std::dec << total_memory_allocated << " bytes" << endl;
	cout << "Total memory freed: " << std::dec << total_memory_freed << " bytes" << endl;
	log_file << "Total memory freed: " << std::dec << total_memory_freed << " bytes" << endl;
	cout << "Total memory has not been freed yet: " << std::dec << total_memory_leaked << " bytes" << endl;
	log_file << "Total memory has not been freed yet: " << std::dec << total_memory_leaked << " bytes" << endl;
	cout << "Total number of mallocs: " << std::dec << malloc_counter << endl;
	log_file << "Total number of mallocs: " << std::dec << malloc_counter << endl;
	cout << "Total number of reallocs: " << std::dec << realloc_counter << endl;
	log_file << "Total number of reallocs: " << std::dec << realloc_counter << endl;
	cout << "Total number of frees: " << std::dec << free_counter << endl<<endl;
	log_file << "Total number of frees: " << std::dec << free_counter << endl<<endl;

	log_file.close();

}

Double_Free_Analyzer::Double_Free_Analyzer(){
	type = dfree_analyzer;
	type_string = "Double free analyzer";
}

void Double_Free_Analyzer::Analyze(vector<template_handler< memory_profiler_sm_object_log_entry_class> > entries) const {

	ofstream log_file;
	log_file.open(("Analyzation_output_"+ process.object->PID_string + ".txt").c_str(), ios::app);

	cout << endl << endl <<"Running dummy free searching for Process: "<< process.object->PID_string<< endl;
	log_file << endl << endl <<"Running dummy free searching for Process: "<< process.object->PID_string<< endl;

	vector<uint64_t> malloc_vector;
	vector<uint64_t>::iterator it_address;

	vector<template_handler< memory_profiler_sm_object_log_entry_class> >::iterator it;

		for(it = entries.begin(); it != entries.end(); it++){
			if(it->object->valid && (it->object->type == malloc_func || it->object->type == calloc_func || it->object->type == realloc_func)){
				malloc_vector.push_back(it->object->address);
		}
		else if(it->object->valid && it->object->type == free_func){
			it_address = find(malloc_vector.begin(), malloc_vector.end(), it->object->address);
			if(it_address == malloc_vector.end()){
				cout << endl <<"Address 0x"<< hex << it->object->address << " is freed but has not been allocated!"<< endl;
				log_file << endl <<"Address 0x"<< hex << it->object->address << " is freed but has not been allocated!"<< endl;

				it->object->Print(process,log_file);
			}
			else{
				malloc_vector.erase(it_address);
			}
		}
	}

	log_file.close();

}

Malloc_Counter_Analyzer::Malloc_Counter_Analyzer(){
	type = malloc_counter_analyzer;
	type_string = "Malloc,calloc,realloc and free counter analyzer";
}

void Malloc_Counter_Analyzer::Analyze(vector<template_handler< memory_profiler_sm_object_log_entry_class> > entries) const {

	ofstream log_file;
	log_file.open(("Analyzation_output_"+ process.object->PID_string + ".txt").c_str(), ios::app);

	cout << endl << endl <<"Running malloc,calloc,realloc and free counting for Process: "<< process.object->PID_string<< endl;
	log_file << endl << endl <<"Running malloc,calloc,realloc and free counting for Process: "<< process.object->PID_string<< endl;

	unsigned long int malloc = 0;
	unsigned long int malloc_size = 0;
	unsigned long int realloc = 0;
	unsigned long int calloc = 0;
	unsigned long int calloc_size = 0;
	unsigned long int free = 0;

	vector<template_handler< memory_profiler_sm_object_log_entry_class> >::iterator it;

	for(it = entries.begin(); it != entries.end(); it++){
		if(it->object->valid){
			if(it->object->type == malloc_func){
				malloc++;
				malloc_size += it->object->size;
			}
			else if(it->object->type == calloc_func){
				calloc++;
				calloc_size += it->object->size;
			}
			else if(it->object->type == realloc_func){
				realloc++;
			}
			else if(it->object->type == free_func){
				free++;
			}
		}
	}

	cout << endl << "Total number of entries (after filtering): " << dec << entries.size() << endl;
	log_file << endl << "Total number of entries (after filtering): " << dec << entries.size() << endl;

	cout << endl << "Number of mallocs: " << dec << malloc << endl;
	log_file << endl << "Number of mallocs: " << dec << malloc << endl;
	cout << endl << "Total memory allocated: " << dec << malloc_size << " bytes" << endl;
	log_file << endl << "Total memory allocated: " << dec << malloc_size << " bytes" << endl;

	cout << endl << "Number of callocs: " << dec << calloc << endl;
	log_file << endl << "Number of callocs: " << dec << calloc << endl;
	cout << endl << "Total memory allocated: " << dec << calloc_size << " bytes" << endl;
	log_file << endl << "Total memory allocated: " << dec << calloc_size << " bytes" << endl;


	cout << endl << "Number of reallocs: " << dec << realloc << endl;
	log_file << endl << "Number of reallocs: " << dec << realloc << endl;
	cout << endl << "Total memory allocated: not relevant for realloc" << endl;
	log_file << endl << "Total memory allocated: not relevant for realloc" << endl;

	cout << endl << "Number of frees: " << dec << free << endl;
	log_file << endl << "Number of frees: " << dec << free << endl;

	log_file.close();
}

Save_symbol_table_Analyzer::Save_symbol_table_Analyzer(){
	type = save_symbol_table_analyzer;
	type_string = "Saving symbol table";
}

void Save_symbol_table_Analyzer::Analyze(vector<template_handler< memory_profiler_sm_object_log_entry_class> > entries) const {

	ofstream symbol_file;

	vector<symbol_table_entry_class>::const_iterator it2;
	map<memory_map_table_entry_class const,vector<symbol_table_entry_class>,memory_map_table_entry_class_comp >::const_iterator it;

	symbol_file.open(process.object->symbol_file_name.c_str(), ofstream::trunc);

	for (it = process.object->all_function_symbol_table.begin(); it != process.object->all_function_symbol_table.end(); it++) {
		symbol_file << endl << endl << it->first.path << endl;
			for(it2 = it->second.begin(); it2 != it->second.end(); it2++){
			symbol_file << it2->name << " ---- " << "0x" << std::hex << it2->address << endl;
		}
	}

	cout << "Process " << process.object->PID_string << " symbol table saved!" << endl;
	symbol_file.close();

}

Save_memory_mappings_Analyzer::Save_memory_mappings_Analyzer(){
	type = save_memory_mappings_analyzer;
	type_string = "Saving virtual memory mapping";
}

void Save_memory_mappings_Analyzer::Analyze(vector<template_handler< memory_profiler_sm_object_log_entry_class> > entries) const {

	ofstream memory_map_file;

	map<memory_map_table_entry_class const,vector<symbol_table_entry_class>,memory_map_table_entry_class_comp >::const_iterator it;

	memory_map_file.open(process.object->memory_map_file_name.c_str(), ofstream::trunc);

	memory_map_file << "MEMORY MAP from /proc/" + process.object->PID_string + "/maps:" << endl << endl;
	for(it = process.object->all_function_symbol_table.begin(); it != process.object->all_function_symbol_table.end(); it++){
		memory_map_file << "0x" << std::hex << it->first.start_address << "--" << "0x" << std::hex << it->first.end_address << "   " << it->first.path <<endl;
	}
	cout << "Process " << dec << process.object->PID_string << " memory mappings saved!" << endl;
	memory_map_file.close();

}

Save_shared_memory_Analyzer::Save_shared_memory_Analyzer(){
	type = save_shared_memory_analyzer;
	type_string = "Saving backtrace";
}

void Save_shared_memory_Analyzer::Analyze(vector<template_handler< memory_profiler_sm_object_log_entry_class> > entries) const {

	ofstream shared_memory_file;

	cout << "Saving Process " << process.object->PID_string << " backtrace..." << endl;
	shared_memory_file.open(process.object->shared_memory_file_name.c_str(), ofstream::app);

	for (unsigned int j = 0; j < entries.size(); j++) {

		if(entries[j].object->valid == true){
			shared_memory_file << endl <<"Shared memory PID: " << process.object->PID_string << endl;
			shared_memory_file <<"Shared_memory index: " << dec << j << endl;
			shared_memory_file <<"Thread ID: " << dec << entries[j].object->thread_id << endl;
			char buffer[30];
			strftime(buffer,30,"%m-%d-%Y %T.",gmtime(&(entries[j].object->tval_before.tv_sec)));
			shared_memory_file <<"GMT before: " << buffer << dec << entries[j].object->tval_before.tv_usec << endl;
			strftime(buffer,30,"%m-%d-%Y %T.",gmtime(&(entries[j].object->tval_after.tv_sec)));
			shared_memory_file <<"GMT after: " << buffer << dec << entries[j].object->tval_after.tv_usec << endl;
			shared_memory_file <<"Call stack type: " << dec << entries[j].object->type << endl;
			shared_memory_file <<"Address: " << hex <<entries[j].object->address << endl;
			shared_memory_file <<"Allocation size: " << dec << entries[j].object->size << endl;
			shared_memory_file <<"Call stack: " << endl;
			for(int  k=0; k < entries[j].object->backtrace_length;k++){
				shared_memory_file << entries[j].object->call_stack[k]<< " --- ";
				shared_memory_file << process.object->Find_function_name((uint64_t) entries[j].object->call_stack[k])<< endl;
			}
		}
	}

	cout << "Process " << process.object->PID_string << " backtrace saved!" << endl;

	shared_memory_file.close();
}
