/*
 * Memory_Profiler_filter.h
 *
 *  Created on: Feb 19, 2016
 *      Author: root
 */

#ifndef MEMORY_PROFILER_FILTER_H_
#define MEMORY_PROFILER_FILTER_H_

#include "Memory_Profiler_process.h"
#include <iostream>

using namespace std;

class Filter_class {
private:
	string name;
protected:
	Filter_class(string name_p) : name(name_p){}
	virtual ~Filter_class(){}

public:
	virtual bool Filter(const memory_profiler_sm_object_log_entry_class &log_entry) const = 0;
	virtual void Print() const = 0;
};

class Size_filter : public Filter_class{

unsigned long size;
string operation;

public:
	Size_filter(unsigned long size_p, string operation_p) : Filter_class("Size filter"), size(size_p), operation(operation_p) {}
	~Size_filter(){}

	unsigned long Get_size()const {return size;}
	void Set_size(unsigned long new_size) {size = new_size;}

	string Get_operation()const {return operation;}
	void Set_operation(string new_op){operation = new_op;}

	void Print() const override {cout << "   operation: " << operation << endl << "   size: " << dec << size << endl;}

	bool Filter(const memory_profiler_sm_object_log_entry_class &log_entry) const override;
};



#endif /* MEMORY_PROFILER_FILTER_H_ */
