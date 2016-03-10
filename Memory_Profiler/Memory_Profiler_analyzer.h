/*
 * Memory_Profiler_analyzators.h
 *
 *  Created on: Feb 18, 2016
 *      Author: root
 */

#ifndef MEMORY_PROFILER_ANALYZER_H_
#define MEMORY_PROFILER_ANALYZER_H_


using namespace std;

class Pattern;

enum analyzer_type{
	leak_analyzer = 1,
	dfree_analyzer = 2,
	print_analyzer = 3,
	malloc_counter_analyzer = 4,
	unknown_analyzer
};

class Analyzer {
private:

	vector<template_handler<Pattern> > Pattern_vector;

protected:
	Analyzer();
	unsigned int type;
	string type_string;
	template_handler<Process_handler> process;

public:

	virtual ~Analyzer();
	Analyzer(const Analyzer &obj);
	Analyzer& operator=(const Analyzer &obj);

	unsigned int GetType()const;
	string Get_type_string() const;
	virtual void Print()const;

	void Start(template_handler<Process_handler> process);
	virtual void Analyze(vector<template_handler<memory_profiler_sm_object_log_entry_class> > entries) const = 0;
	void Stop();

	void Pattern_register(template_handler<Pattern> pattern);
	void Pattern_deregister(string name);
};

class Print_Analyzer : public Analyzer{

public:
	Print_Analyzer();
	~Print_Analyzer(){}

	void Analyze(vector<template_handler< memory_profiler_sm_object_log_entry_class> > entries) const ;

};

class Memory_Leak_Analyzer : public Analyzer{

public:
	Memory_Leak_Analyzer();
	~Memory_Leak_Analyzer(){}

	void Analyze(vector<template_handler< memory_profiler_sm_object_log_entry_class> > entries) const ;
};


class Double_Free_Analyzer : public Analyzer{

public:
	Double_Free_Analyzer();
	~Double_Free_Analyzer(){}

	void Analyze(vector<template_handler< memory_profiler_sm_object_log_entry_class> > entries) const ;

};

class Malloc_Counter_Analyzer : public Analyzer{

public:
	Malloc_Counter_Analyzer();
	~Malloc_Counter_Analyzer(){}

	void Analyze(vector<template_handler< memory_profiler_sm_object_log_entry_class> > entries) const ;

};

bool operator==(template_handler<Analyzer> analyzer_1, const template_handler<Analyzer> analyzer_2);

#endif /* MEMORY_PROFILER_ANALYZER_H_ */
