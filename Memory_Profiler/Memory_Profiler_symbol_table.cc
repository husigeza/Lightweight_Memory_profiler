/*
 * symbol_table.cpp
 *
 *  Created on: Nov 20, 2015
 *      Author: egezhus
 */
#include "Memory_Profiler_symbol_table.h"


symbol_table_entry_class::symbol_table_entry_class(const symbol_table_entry_class &obj){
		address = obj.address;
		name = obj.name;
	}

symbol_table_entry_class& symbol_table_entry_class::operator=(const symbol_table_entry_class &obj){
		if(this != &obj){
			address = obj.address;
			name = obj.name;
		}
		return *this;
	}


bool operator == ( const uint64_t &address, const symbol_table_entry_class& entry){ return (address == entry.address);};
bool operator != ( const uint64_t &address, const symbol_table_entry_class& entry){ return !(address == entry.address);};
bool operator <  ( const uint64_t &address, const symbol_table_entry_class& entry){ return (entry.address < address);};
bool operator >  ( const uint64_t &address, const symbol_table_entry_class& entry){ return (entry.address > address);};
bool operator <= ( const uint64_t &address, const symbol_table_entry_class& entry){ return (entry.address <= address);};
bool operator >= ( const uint64_t &address, const symbol_table_entry_class& entry){ return (entry.address >= address);};

