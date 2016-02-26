/*
 * Memory_Profiler_filter.h
 *
 *  Created on: Feb 19, 2016
 *      Author: root
 */

#ifndef MEMORY_PROFILER_FILTER_H_
#define MEMORY_PROFILER_FILTER_H_



using namespace std;

class Pattern;


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

class Filter {
private:
	unsigned int filter_type;
	vector<unique_ptr<Pattern>* > Pattern_vector;

protected:
	string type_string;
	Filter(unsigned int filtertype, string type_string_p);

public:
	virtual ~Filter();

	unsigned int GetType()const;
	string Get_type_string() const;
	virtual void Print()const = 0;
	void Print_patterns()const;

	virtual bool Filter_func(const memory_profiler_sm_object_log_entry_class &log_entry) const = 0;

	void Pattern_register(unique_ptr<Pattern>* pattern);
	void Pattern_deregister(string name);
};

class Size_filter : public Filter{

unsigned int operation;
string operation_string;
unsigned long size;

public:
	Size_filter(unsigned long size_p, string operation_p);
	~Size_filter(){}

	unsigned long Get_size()const;
	void Set_size(unsigned long new_size);

	string Get_operation()const;

	void Print() const override;

	bool Filter_func(const memory_profiler_sm_object_log_entry_class &log_entry) const override;
};

class Time_filter : public Filter{

};

#endif /* MEMORY_PROFILER_FILTER_H_ */
