#ifndef MEMORY_PROFILER_HANDLER_TEMPLATE_H_INCLUDED
#define MEMORY_PROFILER_HANDLER_TEMPLATE_H_INCLUDED

#include <iostream>

using namespace std;

template <class class_template> class template_handler{

	bool delete_enable; //Needed because of shared memory entries, we don't want to delete them in destructor with delete
public:
	static unsigned int counter;

	class_template* object;

	template_handler(){
		object = 0;
		delete_enable = false;
	};

	~template_handler(){
		counter--;
		if(counter == 0 && delete_enable){
			delete object;
		}
	}

	template_handler(class_template &object_p){
		object = &object_p;
		delete_enable = true;
		counter++;
	}

	template_handler(class_template &object_p, bool delete_enable_p){
		object = &object_p;
		delete_enable = delete_enable_p;
		counter++;
	}

	template_handler(const template_handler &obj){
		object = obj.object;
		delete_enable = obj.delete_enable;
		counter++;
	}

	template_handler& operator=(const template_handler &obj){
		if(this != &obj){
			object = obj.object;
			delete_enable = obj.delete_enable;
			counter++;
		}

		return *this;
	}

	bool operator ==(class_template & entry){
		if(object == entry.object) return true;
		else return false;
	}

};

template <class class_template> unsigned int template_handler<class_template>::counter = 0;

#endif
