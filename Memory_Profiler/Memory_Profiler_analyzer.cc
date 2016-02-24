#include "Memory_Profiler_process.h"


#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>


#include "Memory_Profiler_pattern.h"
#include "Memory_Profiler_analyzer.h"

using namespace std;

Analyzer::Analyzer(unsigned int type_p){
	switch (type_p){
	case 1:
		type = leak_analyzer;
		type_string = "Memory Leak analyzer";
	break;
	case 2:
		type = dfree_analyzer;
		type_string = "Double free analyzer";
	break;
	default:
		type = analyzer_type_unknown;
		type_string = "Unknown";
	break;
	}
	process = nullptr;
}

Analyzer::~Analyzer(){
	for(auto &pattern : Pattern_vector){
		(**pattern).Analyzer_deregister(*this);

	}
}

unsigned int Analyzer::GetType(){
	return type;
}

string Analyzer::Get_type_string() const{
	return type_string;
}

void Analyzer::Pattern_register(unique_ptr<Pattern>* pattern){
	Pattern_vector.push_back(pattern);
}


bool operator==(unique_ptr<Pattern>* pattern, string name){
	if((**pattern).Get_name() == name ) return true;
	else return false;
}

void Analyzer::Pattern_deregister(string name){
	auto pattern = find(Pattern_vector.begin(), Pattern_vector.end(), name);
	Pattern_vector.erase(pattern);
}


void Analyzer::Print() const{
	cout << "   type: " << type_string << endl;
	cout << "   Analyzer is bounded to patterns:" << endl;
	for (auto pattern : Pattern_vector){
		cout <<"      "<< (**pattern).Get_name() << endl;
	}
}

void Analyzer::Start(Process_handler &process){

	ofstream log_file;
	log_file.open(("Analyzation_output_"+ process.PID_string + ".txt").c_str(), ios::app);

	cout << "Analyzer " << type_string << " starting..." << endl;
	log_file << "Analyzer " << type_string << " starting..." << endl;

	this->process = &process;
}

void Analyzer::Stop(){

	ofstream log_file;
	log_file.open(("Analyzation_output_"+ process->PID_string + ".txt").c_str(), ios::app);

	cout << endl << "Analyzer "<< type_string << " has finished!" << endl;
	log_file << endl << "Analyzer "<< type_string << " has finished!" << endl;

	log_file.close();

}

void Memory_Leak_Analyzer::Analyze(vector<const memory_profiler_sm_object_log_entry_class *> &entries) const {

	ofstream log_file;
	log_file.open(("Analyzation_output_"+ process->PID_string + ".txt").c_str(), ios::app);

	cout << endl << endl <<"Running memory leak analyzation for Process: "<< process->PID_string<< endl;
	log_file << endl << endl <<"Running memory leak analyzation for Process: "<< process->PID_string<< endl;

	unsigned long int malloc_counter = 0;
	unsigned long int free_counter = 0;
	unsigned long int total_memory_allocated = 0;
	unsigned long int total_memory_freed = 0;
	unsigned long int address = 0;
	unsigned long int total_memory_leaked = 0;

	vector<const memory_profiler_sm_object_log_entry_class *>::iterator it;
	for(it = entries.begin(); it != entries.end(); it++){

		if((**it).valid && (**it).type == malloc_func){

			total_memory_allocated += (**it).size;
			address = (**it).address;
			malloc_counter++;

			vector<const memory_profiler_sm_object_log_entry_class *>::iterator it2 = it;
			for(; it2 != entries.end(); it2++){
				if((**it2).valid && (**it2).type == free_func){
					if((**it2).address == address){
							total_memory_freed += (**it).size;
							break;
					}
				}
			}
			if(it2 == entries.end()){

				log_file << endl << "Memory 0x" << std::hex << address << " has not been freed yet!" << endl;

				char buffer[30];
				strftime(buffer,30,"%m-%d-%Y %T.",gmtime(&((**it).tval.tv_sec)));
				log_file << "GMT: " << buffer << dec << (**it).tval.tv_usec << endl;

				log_file << "Call stack: " << endl;
				for(int  k = 0; k < (**it).backtrace_length; k++){

					log_file << (**it).call_stack[k]<< " --- ";
					log_file << process->Find_function_name((uint64_t)(**it).call_stack[k]) << endl;
				}
				total_memory_leaked += (**it).size;
			}
		}
		else if((**it).valid && (**it).type == free_func){
			free_counter++;
		}
	}

	cout << endl << "PID: " << process->PID_string << endl;
	log_file<< endl << "PID: " << process->PID_string << endl;
	cout <<"Total memory allocated: " << std::dec << total_memory_allocated << " bytes" << endl;
	log_file <<"Total memory allocated: " << std::dec << total_memory_allocated << " bytes" << endl;
	cout << "Total memory freed: " << std::dec << total_memory_freed << " bytes" << endl;
	log_file << "Total memory freed: " << std::dec << total_memory_freed << " bytes" << endl;
	cout << "Total memory leaked yet: " << std::dec << total_memory_leaked << " bytes" << endl;
	log_file << "Total memory leaked yet: " << std::dec << total_memory_leaked << " bytes" << endl;
	cout << "Total number of mallocs: " << std::dec << malloc_counter << endl;
	log_file << "Total number of mallocs: " << std::dec << malloc_counter << endl;
	cout << "Total number of frees: " << std::dec << free_counter << endl<<endl;
	log_file << "Total number of frees: " << std::dec << free_counter << endl<<endl;

	log_file.close();

}

void Double_Free_Analyzer::Analyze(vector<const memory_profiler_sm_object_log_entry_class *> &entries) const {

	ofstream log_file;
	log_file.open(("Analyzation_output_"+ process->PID_string + ".txt").c_str(), ios::app);

	cout << endl << endl <<"Running dummy free searching for Process: "<< process->PID_string<< endl;
	log_file << endl << endl <<"Running dummy free searching for Process: "<< process->PID_string<< endl;

	vector<uint64_t> malloc_vector;
	vector<uint64_t>::iterator it_address;

	//Need to count with shared_memory.log_count-1 because shared_memory.log_count shows a bigger value with 1 than the real number of elements
	vector<const memory_profiler_sm_object_log_entry_class *>::iterator it;

		for(it = entries.begin(); it != entries.end(); it++){
			if((**it).valid && (**it).type == malloc_func){
				malloc_vector.push_back((**it).address);
		}
		else if((**it).valid && (**it).type == free_func){
			it_address = find(malloc_vector.begin(), malloc_vector.end(), (**it).address);
			if(it_address == malloc_vector.end()){
				cout << endl <<"Address 0x"<< hex << (**it).address << " is freed but has not been allocated!"<< endl;
				log_file << endl <<"Address 0x"<< hex << (**it).address << " is freed but has not been allocated!"<< endl;

				(**it).Print(process,log_file);
			}
			else{
				malloc_vector.erase(it_address);
			}
		}
	}

	log_file.close();

}
