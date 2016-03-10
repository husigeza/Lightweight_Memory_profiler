#include <iostream>
#include <memory>
#include <algorithm>


#include "Memory_Profiler_process.h"
#include "Memory_Profiler_handler_template.h"
#include "Memory_Profiler_pattern.h"

#include "Memory_Profiler_filter.h"

using namespace std;

bool operator==(template_handler<Filter> filter_1, const template_handler<Filter> filter_2){
	if(filter_1.object == filter_2.object) return true;
	else return false;
}

Filter::Filter() {
	filter_type = filter_type_unknown;
	type_string = "";
}

Filter::Filter(unsigned int filtertype, string type_string_p) {
	filter_type = filtertype;
	type_string = type_string_p;
}

Filter::Filter(const Filter &obj){
	filter_type = obj.filter_type;
	type_string = obj.type_string;
}

Filter::~Filter(){

	for(vector<template_handler<Pattern> >::iterator pattern = Pattern_vector.begin();pattern != Pattern_vector.end();pattern++){
		pattern->object->Filter_deregister(this);
	}
}

void Filter::Pattern_register(template_handler<Pattern> pattern){

	vector<template_handler<Pattern> >::iterator it = find(Pattern_vector.begin(),Pattern_vector.end(),pattern);

	if(it == Pattern_vector.end()){
		Pattern_vector.push_back(pattern);
	}
	else{
		//cout << "Filter has been already added to the pattern!" << endl;
	}
}

void Filter::Pattern_deregister(string name){
	vector<template_handler<Pattern> >::iterator pattern = find(Pattern_vector.begin(), Pattern_vector.end(), name);
	if(pattern != Pattern_vector.end()){
		Pattern_vector.erase(pattern);
	}
	else {
		cout << "Pattern " << name << " has not been bounded to this filter" << endl;
	}
}

void Filter::Print_patterns()const {
	cout << "   Filter is bounded to patterns:" << endl;
	for(vector<template_handler<Pattern> >::const_iterator pattern = Pattern_vector.begin();pattern != Pattern_vector.end();pattern++){
		cout <<"      "<< pattern->object->Get_name() << endl;
	}
}

string Filter::Get_type_string() const{
	return type_string;
}

Size_filter::Size_filter(unsigned long size_p, string operation_p) : Filter(size_filter,"Size filter"), size(size_p){

	if(operation_p.find("equal") != string::npos){
		operation = equal_op;
		operation_string = "equal";
	}
	else if(operation_p.find("bigger") != string::npos) {
		operation = bigger;
		operation_string = "bigger";
	}
	else if (operation_p.find("less") != string::npos){
		operation = less_;
		operation_string = "less";
	}
	else {
		operation = operation_type_unknown;
		operation_string = "Unknown";
		throw false;
	}
}

Size_filter::Size_filter(const Size_filter &obj) :  Filter(obj){

	operation = obj.operation;
	operation_string = obj.operation_string;
	size = obj.size;
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

	Print_patterns();
}

bool Size_filter::Filter_func(template_handler< memory_profiler_sm_object_log_entry_class> log_entry) const{

	if(log_entry.object->type == free_func){
		return false;
	}

	if(operation == equal_op){
		if(log_entry.object->size == size){
			return true;
		}
		else{
			return false;
		}
	}
	else if(operation == bigger){
		if(log_entry.object->size > size){
			return true;
		}
		else{
			return false;
		}

	}
	else if(operation == less_){
		if(log_entry.object->size < size){
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
