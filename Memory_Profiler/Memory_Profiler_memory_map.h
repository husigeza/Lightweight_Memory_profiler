/*
 * memory_map.h
 *
 *  Created on: Nov 20, 2015
 *      Author: egezhus
 */

#ifndef MEMORY_PROFILER_MEMORY_MAP_H_
#define MEMORY_PROFILER_MEMORY_MAP_H_

#include <string>
#include <stdint.h>

using namespace std;

class memory_map_table_entry_class {

public:
	uint64_t start_address;
	uint64_t end_address;
	string shared_lib_path;

	memory_map_table_entry_class() : start_address{0},end_address{0},shared_lib_path{""} {};
	memory_map_table_entry_class(uint64_t start_address_p,uint64_t end_address_p,string shared_lib_path_p) : start_address{start_address_p},end_address{end_address_p},shared_lib_path{shared_lib_path_p} {};

	memory_map_table_entry_class(const memory_map_table_entry_class &obj)noexcept;
	memory_map_table_entry_class& operator=(const memory_map_table_entry_class &obj)noexcept;

	memory_map_table_entry_class(memory_map_table_entry_class &&obj)noexcept;
	memory_map_table_entry_class& operator=(memory_map_table_entry_class &&obj)noexcept;

	bool operator==(const string& shared_lib_path) const {
	    if(this->shared_lib_path.compare(shared_lib_path) == 0)return true;
	    else return false;
	}
	bool operator!=(const string& shared_lib_path) const {return(!(this->shared_lib_path == shared_lib_path));}
	bool operator<(const memory_map_table_entry_class& entry) const {return (start_address < entry.start_address);}
	bool operator>(const memory_map_table_entry_class& entry) const {return (!(start_address < entry.start_address));}
};


#endif /* MEMORY_PROFILER_MEMORY_MAP_H_ */
