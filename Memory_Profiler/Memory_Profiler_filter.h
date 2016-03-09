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
	less_ = 3,
	operation_type_unknown
};

class Filter {
private:
	unsigned int filter_type;
	vector< template_handler<Pattern> > Pattern_vector;

protected:
	string type_string;
	Filter();
	Filter(unsigned int filtertype, string type_string_p);
	Filter(const Filter &obj);

public:
	virtual ~Filter();

	unsigned int GetType()const;
	string Get_type_string() const;
	virtual void Print()const = 0;
	void Print_patterns()const;

	virtual bool Filter_func(template_handler< memory_profiler_sm_object_log_entry_class> &log_entry) const = 0;

	void Pattern_register(template_handler<Pattern> &pattern);
	void Pattern_deregister(string name);
};

class Size_filter : public Filter{

unsigned int operation;
string operation_string;
unsigned long size;

public:
	Size_filter(unsigned long size_p, string operation_p);
	~Size_filter(){}

	Size_filter(const Size_filter &obj);

	unsigned long Get_size()const;
	void Set_size(unsigned long new_size);

	string Get_operation()const;

	void Print() const ;

	bool Filter_func(template_handler< memory_profiler_sm_object_log_entry_class> &log_entry) const ;
};

class Time_filter : public Filter{

};

bool operator==(template_handler<Filter> &filter_1, const template_handler<Filter> &filter_2);

#endif /* MEMORY_PROFILER_FILTER_H_ */
