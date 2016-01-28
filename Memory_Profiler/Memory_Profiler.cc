#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include "Memory_Profiler_class.h"
#include "Memory_Profiler_process.h"

//#define path_to_FIFO "/home/egezhus/Memory_profiler/mem_prof_fifo"
#define path_to_FIFO "/dev/mem_prof_fifo"

using namespace std;

static Memory_Profiler mem_prof(path_to_FIFO);
static pthread_t FIFO_read_thread_id;


void* Read_FIFO_thread(void *arg) {

	while (true) {
		mem_prof.Read_FIFO();
		sleep(2);
	}
	return 0;
}

int main() {

	//signal(SIGINT, signal_callback_handler);

	int err = pthread_create(&FIFO_read_thread_id, NULL, &Read_FIFO_thread, NULL);
	if (err) {
		cout << "Thread creation failed error: " << err << endl;
	} else {
		cout << "Read_FIFO_thread created" << endl;
	}

	getchar();
	mem_prof.Print_all_processes();
	mem_prof.Print_alive_processes();
	mem_prof.Print_profiled_processes();

	mem_prof.Start_stop_profiling_all_processes();
	mem_prof.Remove_all_process_from_profiling();
	cout << "Removed all from profiled" << endl;
	mem_prof.Remap_all_process_shared_memory();
	mem_prof.Analyze_all_process();



	while (1) {

		getchar();
		mem_prof.Print_all_processes();
		mem_prof.Print_alive_processes();
		mem_prof.Print_profiled_processes();

		getchar();
		mem_prof.Add_all_process_to_profiling();
		cout << "Added all to profiled" << endl;
		mem_prof.Start_stop_profiling_all_processes();
		cout << "Signal sent" << endl;
		getchar();
		mem_prof.Start_stop_profiling_all_processes();
		cout << "Signal sent" << endl;
		getchar();
		mem_prof.Remove_all_process_from_profiling();
		cout << "Removed all from profiled" << endl;
		mem_prof.Remap_all_process_shared_memory();
		cout << "Remapped shared memories" << endl;
		//mem_prof.Print_all_processes_shared_memory();
		mem_prof.Analyze_all_process();


	}

	return 0;
}
