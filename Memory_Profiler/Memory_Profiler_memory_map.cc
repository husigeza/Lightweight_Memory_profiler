/*
 * memory_map.cpp
 *
 *  Created on: Nov 20, 2015
 *      Author: egezhus
 */

#include "Memory_Profiler_memory_map.h"

memory_map_table_entry_class::memory_map_table_entry_class(const memory_map_table_entry_class &obj){
		start_address = obj.start_address;
		end_address = obj.end_address;
		path = obj.path;
	}

memory_map_table_entry_class& memory_map_table_entry_class::operator=(const memory_map_table_entry_class &obj){

		start_address = obj.start_address;
		end_address = obj.end_address;
		path = obj.path;
		return *this;
	}
