/*
 * Memory_Profiler_Analyzator_Pattern.h
 *
 *  Created on: Feb 18, 2016
 *      Author: root
 */

#ifndef MEMORY_PROFILER_PATTERN_H_
#define MEMORY_PROFILER_PATTERN_H_


using namespace std;

// Forwarding declaration is needed
class Analyzer;
class Filter;


class Pattern{
private:
	string name;
	vector< template_handler<Analyzer> > Analyzer_vector;
	vector< template_handler<Filter> > Filter_vector;

	void Filter_entries(template_handler<memory_profiler_sm_object_class> shared_memory);
	vector<template_handler<memory_profiler_sm_object_log_entry_class> > log_entry_vector;

	void Notify_analyzer(unsigned int index);
	void Notify_filter(unsigned int index);

public:
	Pattern();
	Pattern(string name);
	~Pattern();

	bool operator==(const string Pattern_name);
	const string Get_name() const;
	void Print() const;
	bool Check_process(template_handler<Process_handler> process);

	unsigned int Get_number_of_analyzers();
	unsigned int Get_number_of_filters();

	void Analyzer_register(template_handler<Analyzer> analyzer);
	void Analyzer_deregister(unsigned int index);
	void Analyzer_deregister(const Analyzer *analyzer);

	void Filter_register(template_handler<Filter> filter);
	void Filter_deregister(unsigned int index);
	void Filter_deregister(const Filter *filter);

	void Print_analyzers()const;
	void Print_filters()const;

	void Run_analyzers(template_handler<Process_handler> process);
};

bool operator==(Pattern* pattern,string name);
bool operator==(template_handler<Pattern> pattern_1, const template_handler<Pattern> pattern_2);
bool operator==(template_handler<Pattern> pattern, const string s);

#endif /* MEMORY_PROFILER_PATTERN_H_ */
