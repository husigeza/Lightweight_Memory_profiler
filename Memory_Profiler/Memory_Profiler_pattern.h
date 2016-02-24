/*
 * Memory_Profiler_Analyzator_Pattern.h
 *
 *  Created on: Feb 18, 2016
 *      Author: root
 */

#ifndef MEMORY_PROFILER_PATTERN_H_
#define MEMORY_PROFILER_PATTERN_H_

#include <memory>


using namespace std;

class Analyzer;
class Filter_class;


class Pattern{
private:
	string name;
	vector< unique_ptr<Analyzer>* > Analyzer_vector;
	vector< shared_ptr<Filter_class> > Filter_vector;

	void Filter_entries(const memory_profiler_sm_object_class &shared_memory);
	vector<const memory_profiler_sm_object_log_entry_class *> log_entry_vector;

	void Notify_analyzer(unsigned int index);

public:
	Pattern(string name);

	bool operator==(const string Pattern_name);

	const string Get_name() const;

	void Print() const;

	bool Check_process(Process_handler & process);

	unsigned int Get_number_of_analyzers();
	unsigned int Get_number_of_filters();

	void Analyzer_register(unique_ptr<Analyzer>* analyzer);

	void Analyzer_deregister(unsigned int index);
	void Analyzer_deregister(const Analyzer &analyzer);

	void Filter_register(shared_ptr<Filter_class> filter);
	void Filter_deregister(unsigned int index);

	void Print_analyzers();
	void Print_filters();

	void Run_analyzers(Process_handler & process);
};

bool operator==(unique_ptr<Pattern>* pattern,string name);

#endif /* MEMORY_PROFILER_PATTERN_H_ */
