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

enum filter_type{
	size_filter = 1,
	time_filter = 2,
	filter_type_unknown
};

enum operation_type{
	equal_op = 1,
	bigger = 2,
	less_eq = 3,
	operation_type_unknown
};

class Filter_class {
private:
	unsigned int filter_type;

protected:
	Filter_class(unsigned int filtertype, string type_string_p) : filter_type(filtertype), type_string(type_string_p){}
	virtual ~Filter_class(){}
	string type_string;


public:
	virtual bool Filter(const memory_profiler_sm_object_log_entry_class &log_entry) const = 0;
	virtual void Print() const = 0;
};

class Size_filter : public Filter_class{

unsigned int operation;
string operation_string;
unsigned long size;

public:
	Size_filter(unsigned long size_p, string operation_p);
	~Size_filter(){}

	unsigned long Get_size()const {return size;}
	void Set_size(unsigned long new_size) {size = new_size;}

	string Get_operation()const {return operation_string;}

	void Print() const override;

	bool Filter(const memory_profiler_sm_object_log_entry_class &log_entry) const override;
};



#endif /* MEMORY_PROFILER_FILTER_H_ */
