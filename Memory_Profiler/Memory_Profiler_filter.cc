#include "Memory_Profiler_filter.h"


bool Size_filter::Filter(const memory_profiler_sm_object_log_entry_class &log_entry) const{

	if(log_entry.type == free_func){
		return true;
	}

	if(operation.find("eq") != string::npos){
		if(log_entry.size == size){
			return true;
		}
		else{
			return false;
		}
	}
	else if(operation.find("b") != string::npos){
		if(log_entry.size >= size){
			return true;
		}
		else{
			return false;
		}

	}
	else if(operation.find("l") != string::npos){
		if(log_entry.size < size){
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
