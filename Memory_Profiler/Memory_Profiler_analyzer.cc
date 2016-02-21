
#include "Memory_Profiler_analyzer.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;


bool Analyzer::Start(Process_handler &process){


	ofstream log_file;
	log_file.open(("Analyzation_output_"+ process.PID_string + ".txt").c_str(), ios::app);

	try{
		if(process.Is_shared_memory_initialized() == true){
			if(process.Get_profiled() == true){
				log_file << "Process "<< process.PID_string <<" is still being profiled, stop profiling first! "<< endl;
				cout << "Process "<< process.PID_string <<" is still being profiled, stop profiling first! "<< endl;
				throw false;
			}
			const memory_profiler_sm_object_class &shared_memory = *process.Get_shared_memory();
			if(shared_memory.log_count == 0){
				cout << endl << "shared_memory log_count = 0, no data to analyze!" << endl;
				log_file << endl << "shared_memory log_count = 0, no data to analyze!" << endl;
				throw false;
			}
		}
		else{
			cout << endl << "NO DATA AVAILABLE, shared memory has not been initialized!" << endl;
			log_file << endl << "NO DATA AVAILABLE, shared memory has not been initialized!" << endl;
			throw false;
		}
		throw true;
	}
	catch(bool b){
		log_file.close();
		if(b == true){
			this->process = &process;
			return true;
		}
		else {
			return false;
		}
	}
}

void Analyzer::Stop(vector<const memory_profiler_sm_object_log_entry_class *> &entries){

	ofstream log_file;
	log_file.open(("Analyzation_output_"+ process->PID_string + ".txt").c_str(), ios::app);

	entries.clear();

	cout << endl << "Analyzation "<< type << " has finished!" << endl;
	log_file << endl << "Analyzation "<< type << " has finished!" << endl;

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

	//Need to count with shared_memory.log_count-1 because shared_memory.log_count shows a bigger value with 1 than the real number of elements
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
