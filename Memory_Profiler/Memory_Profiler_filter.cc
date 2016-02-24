#include "Memory_Profiler_filter.h"


Filter_class::Filter_class(unsigned int filtertype, string type_string_p) : filter_type(filtertype), type_string(type_string_p){}

Size_filter::Size_filter(unsigned long size_p, string operation_p) : Filter_class(size_filter,"Size filter"), size(size_p){

	if(operation_p.find("equal") != string::npos){
		operation = equal_op;
		operation_string = "equal";
	}
	else if(operation_p.find("bigger") != string::npos) {
		operation = bigger;
		operation_string = "bigger";
	}
	else if (operation_p.find("less") != string::npos){
		operation = less_eq;
		operation_string = "less and equal";
	}
	else {
		operation = operation_type_unknown;
		operation_string = "Unknown";
		throw false;
	}
}

unsigned long Size_filter::Get_size()const {
	return size;
}

void Size_filter::Set_size(unsigned long new_size){
	size = new_size;
}

string Size_filter::Get_operation()const {
	return operation_string;
}

void Size_filter::Print() const{
	cout << "   Type: " << type_string << endl;
	cout << "   Operation: " << operation_string << endl;
	cout << "   Size: " << dec << size << endl;
}

bool Size_filter::Filter(const memory_profiler_sm_object_log_entry_class &log_entry) const{

	if(log_entry.type == free_func){
		return true;
	}

	if(operation == equal_op){
		if(log_entry.size == size){
			return true;
		}
		else{
			return false;
		}
	}
	else if(operation == bigger){
		if(log_entry.size > size){
			return true;
		}
		else{
			return false;
		}

	}
	else if(operation == less_eq){
		if(log_entry.size <= size){
			return true;
		}
		else{
			return false;
		}
	}
	else {
		return true;
	}
}
