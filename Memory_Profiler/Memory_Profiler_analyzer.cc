#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

#include "Memory_Profiler_process.h"
#include "Memory_Profiler_handler_template.h"
#include "Memory_Profiler_pattern.h"

#include "Memory_Profiler_analyzer.h"

using namespace std;

bool operator==(template_handler<Analyzer> &analyzer_1, const template_handler<Analyzer> &analyzer_2){
	if(analyzer_1.object == analyzer_2.object) return true;
	else return false;
}

Analyzer::Analyzer(unsigned int type_p){

	switch (type_p){
	case 1:
		type = leak_analyzer;
		type_string = "Memory Leak analyzer";
	break;
	case 2:
		type = dfree_analyzer;
		type_string = "Double free analyzer";
	break;
	case 3:
		type = print_analyzer;
		type_string = "Print analyzer";
	break;
	default:
		type = analyzer_type_unknown;
		type_string = "Unknown";
	break;
	}
	//process = 0;
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

void Analyzer::Pattern_register(template_handler<Pattern> &pattern){

	vector<template_handler<Pattern> >::iterator it = find(Pattern_vector.begin(),Pattern_vector.end(),pattern);

	if(it == Pattern_vector.end()){
		Pattern_vector.push_back(pattern);
	}
	else{
		//cout << "Analyzer has been already added to the pattern!" << endl;
	}
}


bool operator==(const template_handler<Pattern> &pattern, string name){
	if(pattern.object->Get_name() == name ) return true;
	else return false;
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

void Analyzer::Start(template_handler<Process_handler> &process){

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

void Print_Analyzer::Analyze(vector<template_handler< memory_profiler_sm_object_log_entry_class> > &entries) const {

	for(vector<template_handler< memory_profiler_sm_object_log_entry_class> >::iterator entry = entries.begin();entry != entries.end();entry++){
		entry->object->Print(process);
	}
}


void Memory_Leak_Analyzer::Analyze(vector<template_handler< memory_profiler_sm_object_log_entry_class> > &entries) const {

	ofstream log_file;
	log_file.open(("Analyzation_output_"+ process.object->PID_string + ".txt").c_str(), ios::app);

	cout << endl << endl <<"Running memory leak analyzation for Process: "<< process.object->PID_string<< endl;
	log_file << endl << endl <<"Running memory leak analyzation for Process: "<< process.object->PID_string<< endl;

	unsigned long int malloc_counter = 0;
	unsigned long int free_counter = 0;
	unsigned long int total_memory_allocated = 0;
	unsigned long int total_memory_freed = 0;
	unsigned long int address = 0;
	unsigned long int total_memory_leaked = 0;

	vector<template_handler< memory_profiler_sm_object_log_entry_class> >::iterator it;
	for(it = entries.begin(); it != entries.end(); it++){

		if(it->object->valid && it->object->type == malloc_func){

			total_memory_allocated += it->object->size;
			address = it->object->address;
			malloc_counter++;

			vector<template_handler< memory_profiler_sm_object_log_entry_class> >::iterator it2 = it;
			for(; it2 != entries.end(); it2++){
				if(it2->object->valid && it2->object->type == free_func){
					if(it2->object->address == address){
							total_memory_freed += it->object->size;
							break;
					}
				}
			}
			if(it2 == entries.end()){

				log_file << endl << "Memory 0x" << std::hex << address << " has not been freed yet!" << endl;

				char buffer[30];
				strftime(buffer,30,"%m-%d-%Y %T.",gmtime(&(it->object->tval_before.tv_sec)));
				log_file << "GMT before: " << buffer << dec << it->object->tval_before.tv_usec << endl;
				strftime(buffer,30,"%m-%d-%Y %T.",gmtime(&(it->object->tval_after.tv_sec)));
				log_file << "GMT after: " << buffer << dec << it->object->tval_after.tv_usec << endl;

				log_file << "Call stack: " << endl;
				for(int  k = 0; k < it->object->backtrace_length; k++){

					log_file << it->object->call_stack[k]<< " --- ";
					log_file << process.object->Find_function_name((uint64_t)it->object->call_stack[k]) << endl;
				}
				total_memory_leaked += it->object->size;
			}
		}
		else if(it->object->valid && it->object->type == free_func){
			free_counter++;
		}
	}

	cout << endl << "PID: " << process.object->PID_string << endl;
	log_file<< endl << "PID: " << process.object->PID_string << endl;
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

void Double_Free_Analyzer::Analyze(vector<template_handler< memory_profiler_sm_object_log_entry_class> > &entries) const {

	ofstream log_file;
	log_file.open(("Analyzation_output_"+ process.object->PID_string + ".txt").c_str(), ios::app);

	cout << endl << endl <<"Running dummy free searching for Process: "<< process.object->PID_string<< endl;
	log_file << endl << endl <<"Running dummy free searching for Process: "<< process.object->PID_string<< endl;

	vector<uint64_t> malloc_vector;
	vector<uint64_t>::iterator it_address;

	vector<template_handler< memory_profiler_sm_object_log_entry_class> >::iterator it;

		for(it = entries.begin(); it != entries.end(); it++){
			if(it->object->valid && it->object->type == malloc_func){
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
