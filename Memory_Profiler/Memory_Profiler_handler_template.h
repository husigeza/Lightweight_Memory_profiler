#ifndef MEMORY_PROFILER_HANDLER_TEMPLATE_H_INCLUDED
#define MEMORY_PROFILER_HANDLER_TEMPLATE_H_INCLUDED

#include <iostream>

using namespace std;

template <class class_template> class template_handler{

public:
	class_template* object;

	template_handler(){
		//cout << " template_handler default constructor " << endl;
		object = 0;
	}

	~template_handler(){
		//cout << " template_handler destructor, this: "<< hex << this<< endl;
		//delete object;
	}

	template_handler(class_template *object_p){
		//cout << " template_handler constructor, this: "<< hex << this << endl;
		object = object_p;
	}

	template_handler(const template_handler &obj){
		//cout << " template_handler copy constructor,this: " << hex << this << " obj: " <<hex << &obj << endl;
		object = obj.object;
	}

	template_handler& operator=(const template_handler &obj){
		object = obj.object;
		return *this;
	}

	void delete_object(){
		delete object;
	}

	bool operator ==(class_template & entry){
		if(object == entry.object) return true;
		else return false;
	}

};

#endif
