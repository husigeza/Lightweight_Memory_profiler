/*
 * Memory_Profiler_Analyzator_Pattern.h
 *
 *  Created on: Feb 18, 2016
 *      Author: root
 */

#ifndef MEMORY_PROFILER_PATTERN_H_
#define MEMORY_PROFILER_PATTERN_H_

#include "Memory_Profiler_analyzer.h"
#include "Memory_Profiler_filter.h"

#include <vector>
#include <memory>

using namespace std;


class Pattern{
private:
	string name;
	vector< shared_ptr<Analyzer> > Analyzer_vector;
	vector< shared_ptr<Filter_class> > Filter_vector;

	void Filter_entries(const memory_profiler_sm_object_class &shared_memory);
	vector<const memory_profiler_sm_object_log_entry_class *> log_entry_vector;

public:
	Pattern(string name){this->name =  name;}

	bool operator==(const string Pattern_name){
		if(name == Pattern_name) return true;
		else return false;
	}

	const string Get_name() const {return this->name;}

	void Print() const {cout << "Name: " << this->name;}

	bool Check_process(Process_handler & process);

	void Analyzer_register(shared_ptr<Analyzer> analyzer);
	void Analyzer_deregister(unsigned int index);

	void Filter_register(shared_ptr<Filter_class> filter);
	void Filter_deregister(unsigned int index);

	void Print_analyzers();
	void Print_filters();

	void Run_analyzers(Process_handler & process);
};


#endif /* MEMORY_PROFILER_PATTERN_H_ */
