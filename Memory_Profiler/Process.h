/*
 * Process.h
 *
 *  Created on: Nov 2, 2015
 *      Author: egezhus
 */

#ifndef PROCESS_H_
#define PROCESS_H_

#include <stdint.h>
#include <semaphore.h>


using namespace std;

#define max_log_entry 1000
#define max_call_stack_depth 15

typedef struct memory_profiler_log_entry_s{
	pthread_t thread_id;
	int type; //malloc = 1, free = 2
	size_t size; //allocated bytes in case of malloc
	void *call_stack[max_call_stack_depth];
	uint64_t address;
	bool valid;
}memory_profiler_log_entry_t;


typedef struct memory_profiler_struct_s {
	int log_count;
	memory_profiler_log_entry_t log_entry[max_log_entry];
} memory_profiler_struct_t;


class Process_handler {

    pid_t PID;
    string PID_string;
    bool profiled;
    bool alive;
    int shared_memory;
    memory_profiler_struct_t *memory_profiler_struct;
    int semaphore_shared_memory;
    sem_t* semaphore;

    void Init_semaphore();

    public:

    	Process_handler();
        Process_handler(pid_t PID);
        ~Process_handler();
        Process_handler(const Process_handler &obj) = delete;
        Process_handler(Process_handler &&obj);
        Process_handler& operator=(Process_handler &&obj);

        void Init_shared_memory();

        pid_t GetPID(){return PID;};

        void Set_profiled(bool value){this->profiled = value;};
        bool Get_profiled(){return profiled;};

        void Set_alive(bool value){this->alive = value;};
		bool Get_alive(){return alive;};

        void Send_signal();
        void Start_Stop_profiling();

        memory_profiler_struct_t* Get_shared_memory();
};



#endif /* PROCESS_H_ */
