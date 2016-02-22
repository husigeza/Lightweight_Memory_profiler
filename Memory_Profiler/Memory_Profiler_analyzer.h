/*
 * Memory_Profiler_analyzators.h
 *
 *  Created on: Feb 18, 2016
 *      Author: root
 */

#ifndef MEMORY_PROFILER_ANALYZER_H_
#define MEMORY_PROFILER_ANALYZER_H_

#include "Memory_Profiler_process.h"
#include <iostream>

using namespace std;

class Analyzer {
private:
	string type;

protected:

	Analyzer(string type_p) : type(type_p), process(nullptr) {}
	virtual ~Analyzer(){}

	Process_handler *process;

public:

	string GetType(){return type;}

	bool Start(Process_handler & process);
	virtual void Analyze(vector<const memory_profiler_sm_object_log_entry_class *> &entries) const = 0;
	void Stop();

	virtual void Print()const {cout << "   type: " << type << endl;}
};

class Memory_Leak_Analyzer : public Analyzer{

public:
	Memory_Leak_Analyzer() : Analyzer("Memory leak analyzer"){}
	~Memory_Leak_Analyzer(){}

	void Analyze(vector<const memory_profiler_sm_object_log_entry_class *> &entries) const override;
};

class Double_Free_Analyzer : public Analyzer{

public:
	Double_Free_Analyzer() : Analyzer("Double free analyzer"){}
	~Double_Free_Analyzer(){}

	void Analyze(vector<const memory_profiler_sm_object_log_entry_class *> &entries) const override;

};


#endif /* MEMORY_PROFILER_ANALYZER_H_ */
