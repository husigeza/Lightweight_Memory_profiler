
#include "Memory_Profiler_analyzer_pattern.h"

#include "Memory_Profiler_process.h"
#include <iostream>
#include <iterator>

using namespace std;

Analyzer_Pattern::Analyzer_Pattern(Analyzer_Pattern &&obj){
	name = obj.name;
	Analyzer_vector = obj.Analyzer_vector;
	Filter_vector = obj.Filter_vector;


	obj.name = "";
	obj.Analyzer_vector.clear();
	obj.Filter_vector.clear();
}
Analyzer_Pattern& Analyzer_Pattern::operator=(Analyzer_Pattern &&obj){

	if(this != &obj){
		name = obj.name;
		Analyzer_vector = obj.Analyzer_vector;
		Filter_vector = obj.Filter_vector;

		obj.name = "";
		obj.Analyzer_vector.clear();
		obj.Filter_vector.clear();
	}
	return *this;
}


void Analyzer_Pattern::Analyzer_register(shared_ptr<Analyzer> analyzer){

	Analyzer_vector.push_back(analyzer);
}

void Analyzer_Pattern::Filter_register(shared_ptr<Filter_class> filter){

	Filter_vector.push_back(filter);
}

void Analyzer_Pattern::Analyzer_deregister(unsigned int index){

	Analyzer_vector.erase(Analyzer_vector.begin() + index);
}

void Analyzer_Pattern::Filter_deregister(unsigned int index){

	Filter_vector.erase(Filter_vector.begin() + index);
}

void Analyzer_Pattern::Print_analyzers(){

	vector<shared_ptr<Analyzer> >::iterator it;

	for(it = Analyzer_vector.begin(); it != Analyzer_vector.end(); it++){
		cout <<"Index: " << dec << distance(Analyzer_vector.begin(), it) << endl;
		(*it)->Print();
	}
}

void Analyzer_Pattern::Print_filters(){

	vector<shared_ptr<Filter_class> >::iterator it;

	for(it = Filter_vector.begin(); it != Filter_vector.end(); it++){
		cout <<"Index: " << dec << distance(Filter_vector.begin(), it) << endl;
		(*it)->Print();
	}
}

void Analyzer_Pattern::Filter_entries(const memory_profiler_sm_object_class &shared_memory){

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


void Analyzer_Pattern::Run_analyzers(Process_handler &process){


	for(auto &analyzer : Analyzer_vector){
		if(analyzer->Start(process)){
			Filter_entries(*process.Get_shared_memory());
			analyzer->Analyze(log_entry_vector);
			analyzer->Stop(log_entry_vector);
		}
	}
}
