#include "Memory_Profiler_process.h"

#include <iostream>
#include <iterator>
#include <vector>
#include <memory>
#include <algorithm>


#include "Memory_Profiler_handler_template.h"

#include "Memory_Profiler_analyzer.h"
#include "Memory_Profiler_filter.h"
#include "Memory_Profiler_pattern.h"

using namespace std;

bool operator==(template_handler<Pattern> pattern_1, const template_handler<Pattern> pattern_2){
	if(pattern_1.object == pattern_2.object) return true;
	else return false;
}

bool operator==(template_handler<Analyzer> analyzer, const Analyzer* analyzer_ptr){
	if(analyzer.object == analyzer_ptr) return true;
	else return false;
}

bool operator==(template_handler<Pattern> pattern, const string s){
	if(pattern.object->Get_name() == s) return true;
	else return false;
}

Pattern::Pattern(){
	name = "";
}

Pattern::~Pattern(){}

Pattern::Pattern(string name){
	this->name =  name;
}

bool Pattern::operator==(const string Pattern_name){
	if(name == Pattern_name) return true;
	else return false;
}

const string Pattern::Get_name() const {
	return this->name;
}

void Pattern::Print() const {
	cout << "Name: " << this->name;
}

unsigned int Pattern::Get_number_of_analyzers() {
	return Analyzer_vector.size();
}

unsigned int Pattern::Get_number_of_filters() {
	return Filter_vector.size();
}

void Pattern::Notify_analyzer(unsigned int index){

	Analyzer_vector[index].object->Pattern_deregister(name);
}

void Pattern::Notify_filter(unsigned int index){

	Filter_vector[index].object->Pattern_deregister(name);
}

bool Pattern::Check_process(template_handler<Process_handler> process){

	ofstream log_file;
	log_file.open(("Analyzation_output_"+ process.object->PID_string + ".txt").c_str(), ios::app);

	try{
		if(process.object->Is_shared_memory_initialized() == true){
			if(process.object->Get_profiled() == true){
				log_file << "Process "<< process.object->PID_string <<" is still being profiled, stop profiling first! "<< endl;
				cout << "Process "<< process.object->PID_string <<" is still being profiled, stop profiling first! "<< endl;
				throw false;
			}
			const memory_profiler_sm_object_class &shared_memory = *process.object->Get_shared_memory();
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


void Pattern::Analyzer_register(template_handler<Analyzer> analyzer){

	vector< template_handler<Analyzer> >::iterator it = find(Analyzer_vector.begin(),Analyzer_vector.end(),analyzer);
	if(it == Analyzer_vector.end()){
		Analyzer_vector.push_back(analyzer);
	}
	else {
		cout << "This analyzer has been already added to this pattern!" << endl;
	}
}

void Pattern::Filter_register(template_handler<Filter> filter){

	vector<template_handler<Filter> >::iterator it = find(Filter_vector.begin(),Filter_vector.end(),filter);
		if(it == Filter_vector.end()){
			Filter_vector.push_back(filter);
		}
		else {
			cout << "This filter has been already added to this pattern!" << endl;
		}
}

void Pattern::Analyzer_deregister(unsigned int index){

	if(index >= Analyzer_vector.size()){
		cout << "Wrong Analyzer ID" << endl;
	}
	else{
		Notify_analyzer(index);
		Analyzer_vector.erase(Analyzer_vector.begin() + index);
	}
}

void Pattern::Analyzer_deregister(const Analyzer *analyzer){

	vector< template_handler<Analyzer> >::iterator it = find(Analyzer_vector.begin(),Analyzer_vector.end(),analyzer);
	if(it != Analyzer_vector.end()){
		Analyzer_vector.erase(it);
	}
	else {
		cout << "Analyzer has not been bounded to this pattern" << endl;
	}
}

void Pattern::Filter_deregister(unsigned int index){

	if(index >= Filter_vector.size()){
		cout << "Wrong Filter ID" << endl;
	}
	else{
		Notify_filter(index);
		Filter_vector.erase(Filter_vector.begin() + index);
	}
}


bool operator==(template_handler<Filter> filter, const Filter *filter_ptr){
	if(filter.object == filter_ptr) return true;
	else return false;
}

void Pattern::Filter_deregister(const Filter *filter){

	vector< template_handler<Filter> >::iterator it = find(Filter_vector.begin(),Filter_vector.end(),filter);
	if(it != Filter_vector.end()){
		Filter_vector.erase(it);
	}
	else {
		cout << "Filter has not been bounded to this pattern" << endl;
	}
}

void Pattern::Print_analyzers(){

	vector<template_handler<Analyzer> >::iterator it;

	for(it = Analyzer_vector.begin(); it != Analyzer_vector.end(); it++){
		cout <<"Index: " << dec << distance(Analyzer_vector.begin(), it) << endl;
		it->object->Print();
	}
}

void Pattern::Print_filters(){

	vector<template_handler<Filter> >::iterator it;

	for(it = Filter_vector.begin(); it != Filter_vector.end(); it++){
		cout <<"Index: " << dec << distance(Filter_vector.begin(), it) << endl;
		it->object->Print();
	}
}

void Pattern::Filter_entries(template_handler<memory_profiler_sm_object_class> shared_memory){

	bool filter = true;

	for(unsigned long int i = 0; i < shared_memory.object->log_count ; i++){

		template_handler<memory_profiler_sm_object_log_entry_class> log_entry(shared_memory.object->log_entry[i],false);

		for(vector<template_handler<Filter> >::iterator filter_entry = Filter_vector.begin();filter_entry != Filter_vector.end();filter_entry++){
			filter &= filter_entry->object->Filter_func(log_entry);
		}
		if(filter){
			log_entry_vector.push_back(log_entry);
		}
		filter = true;
	}
}


void Pattern::Run_analyzers(template_handler<Process_handler> process){

	if(Check_process(process)){

		Filter_entries(template_handler<memory_profiler_sm_object_class>((*process.object->Get_shared_memory()),false));

		for(vector<template_handler<Analyzer> >::iterator analyzer = Analyzer_vector.begin();analyzer != Analyzer_vector.end();analyzer++){
			analyzer->object->Start(process);
			analyzer->object->Analyze(log_entry_vector);
			analyzer->object->Stop();
		}

		log_entry_vector.clear();
	}

}
