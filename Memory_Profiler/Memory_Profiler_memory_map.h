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
	string path;

	memory_map_table_entry_class() {
		start_address = 0;
		end_address = 0;
		path = "";
	}
	memory_map_table_entry_class(uint64_t start_address_p,uint64_t end_address_p,string path_p) {
		start_address = start_address_p;
		end_address = end_address_p;
		path = path_p;
	}
	memory_map_table_entry_class(const memory_map_table_entry_class &obj);
	memory_map_table_entry_class& operator=(const memory_map_table_entry_class &obj);

	bool operator==(const string& path) const {
	    if(this->path.compare(path) == 0)return true;
	    else return false;
	}
	bool operator!=(const string& path) const {return(!(this->path == path));}
	bool operator<(const memory_map_table_entry_class& entry) const {return (end_address < entry.end_address);}
	bool operator>(const memory_map_table_entry_class& entry) const {return (end_address > entry.end_address);}

	bool operator!=(const uint64_t& address) const {return(this->end_address != address);}
	bool operator<(const uint64_t& address) const  {return (end_address < address);}
	bool operator>(const uint64_t& address) const  {return (end_address > address);}
};


#endif /* MEMORY_PROFILER_MEMORY_MAP_H_ */
