/*
 * symbol_table.cpp
 *
 *  Created on: Nov 20, 2015
 *      Author: egezhus
 */
#include "symbol_table.h"


symbol_table_entry_class::symbol_table_entry_class(const symbol_table_entry_class &obj)noexcept{
		address = obj.address;
		name = obj.name;
	}

symbol_table_entry_class& symbol_table_entry_class::operator=(const symbol_table_entry_class &obj)noexcept{
		address = obj.address;
		name = obj.name;
		return *this;
	}

symbol_table_entry_class::symbol_table_entry_class(symbol_table_entry_class &&obj)noexcept{

		if(this != &obj){
			address = obj.address;
			name = obj.name;

			obj.address = 0;
			obj.name = "";
		}
	}

symbol_table_entry_class& symbol_table_entry_class::operator=(symbol_table_entry_class &&obj)noexcept{

		if (this != &obj) {
			name = obj.name;
			address = obj.address;

			obj.name = "";
			obj.address = 0;
		}
		return *this;
	}



bool operator == (const uint64_t &address, const symbol_table_entry_class& entry){ return (address == entry.address);}
bool operator != (const uint64_t &address, const symbol_table_entry_class& entry){ return !(address == entry.address);}
bool operator <  (const uint64_t &address, const symbol_table_entry_class& entry){ return (address < entry.address);}
bool operator >  (const uint64_t &address, const symbol_table_entry_class& entry){ return !(address < entry.address);}
bool operator <= (const uint64_t &address, const symbol_table_entry_class& entry){ return (address < entry.address);}
bool operator >= (const uint64_t &address, const symbol_table_entry_class& entry){ return !(address < entry.address);}

bool operator == (const symbol_table_entry_class& entry, const uint64_t &address){return (entry.address == address);}
bool operator != (const symbol_table_entry_class& entry, const uint64_t &address){return !(entry.address == address);}
bool operator <  (const symbol_table_entry_class& entry, const uint64_t &address){return (entry.address < address);}
bool operator >  (const symbol_table_entry_class& entry, const uint64_t &address){return !(entry.address < address);}
bool operator <= (const symbol_table_entry_class& entry, const uint64_t &address){return (entry.address < address);}
bool operator >= (const symbol_table_entry_class& entry, const uint64_t &address){return (entry.address > address);}


