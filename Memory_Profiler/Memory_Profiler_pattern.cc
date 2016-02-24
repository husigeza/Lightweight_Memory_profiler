#include "Memory_Profiler_process.h"

#include <iostream>
#include <iterator>
#include <vector>
#include <memory>
#include <algorithm>


#include "Memory_Profiler_analyzer.h"
#include "Memory_Profiler_filter.h"
#include "Memory_Profiler_pattern.h"

using namespace std;


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
	(**Analyzer_vector[index]).Pattern_deregister(name);
}

bool Pattern::Check_process(Process_handler & process){

	ofstream log_file;
	log_file.open(("Analyzation_output_"+ process.PID_string + ".txt").c_str(), ios::app);

	try{
		if(process.Is_shared_memory_initialized() == true){
			if(process.Get_profiled() == true){
				log_file << "Process "<< process.PID_string <<" is still being profiled, stop profiling first! "<< endl;
				cout << "Process "<< process.PID_string <<" is still being profiled, stop profiling first! "<< endl;
				throw false;
			}
			const memory_profiler_sm_object_class &shared_memory = *process.Get_shared_memory();
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


void Pattern::Analyzer_register(unique_ptr<Analyzer>* analyzer){

	Analyzer_vector.push_back(analyzer);
}

void Pattern::Filter_register(shared_ptr<Filter_class> filter){

	Filter_vector.push_back(filter);
}

void Pattern::Analyzer_deregister(unsigned int index){

	Analyzer_vector.erase(Analyzer_vector.begin() + index);
}

bool operator==(unique_ptr<Analyzer>* unq_analyzer, const Analyzer &analyzer){
	if(&(**unq_analyzer) == &analyzer) return true;
	else return false;
}

void Pattern::Analyzer_deregister(const Analyzer &analyzer){

	vector< unique_ptr<Analyzer>* >::iterator it = find(Analyzer_vector.begin(),Analyzer_vector.end(),analyzer);
	Analyzer_vector.erase(it);
}

void Pattern::Filter_deregister(unsigned int index){

	Filter_vector.erase(Filter_vector.begin() + index);
}

void Pattern::Print_analyzers(){

	vector<unique_ptr<Analyzer>* >::iterator it;

	for(it = Analyzer_vector.begin(); it != Analyzer_vector.end(); it++){
		cout <<"Index: " << dec << distance(Analyzer_vector.begin(), it) << endl;
		cout << "   " << (**it)->Get_type_string() << endl;
	}
}

void Pattern::Print_filters(){

	vector<shared_ptr<Filter_class> >::iterator it;

	for(it = Filter_vector.begin(); it != Filter_vector.end(); it++){
		cout <<"Index: " << dec << distance(Filter_vector.begin(), it) << endl;
		(*it)->Print();
	}
}

void Pattern::Filter_entries(const memory_profiler_sm_object_class &shared_memory){

	bool filter = true;

	for(unsigned long int i = 0; i < shared_memory.log_count ; ++i){
		for(auto &filter_entry : Filter_vector){
			filter &= filter_entry->Filter(shared_memory.log_entry[i]);
		}
		if(filter){
			log_entry_vector.push_back(&shared_memory.log_entry[i]);
		}
		filter = true;
	}
}


void Pattern::Run_analyzers(Process_handler &process){


	if(Check_process(process)){

		Filter_entries(*process.Get_shared_memory());

		for(auto &analyzer : Analyzer_vector){
			(**analyzer).Start(process);
			(**analyzer).Analyze(log_entry_vector);
			(**analyzer).Stop();
		}

		log_entry_vector.clear();
	}

}
