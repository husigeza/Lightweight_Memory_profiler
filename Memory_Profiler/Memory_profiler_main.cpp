#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include "Memory_Profiler.h"
#include "Process.h"

#include <signal.h>

#define fifo_path "/home/egezhus/mem_prof_fifo"

using namespace std;

static Memory_Profiler mem_prof;
static pthread_t FIFO_read_thread_id;


void* Read_FIFO_thread(void *arg) {

	while (true) {
		mem_prof.Read_FIFO();
		sleep(2);
	}
	return 0;
}

void
signal_callback_handler(int signum)
{
   printf("Caught signal %d\n",signum);

   // Terminate program
   exit(signum);
}


int main() {

	signal(SIGINT, signal_callback_handler);

	int err = pthread_create(&FIFO_read_thread_id, NULL, &Read_FIFO_thread, NULL);
	if (err) {
		printf("Thread creation failed error:%d \n", err);
	} else {
		printf("Thread created\n");
	}



	while (1) {

		getchar();
		mem_prof.Print_all_processes();
		mem_prof.Print_alive_processes();
		mem_prof.Print_profiled_processes();
		/*getchar();
		mem_prof.Add_all_process_to_profiling();
		cout << "Added all to profiled" << endl;
		mem_prof.Start_stop_profiling_all_processes();
		cout << "Signal sent" << endl;
		getchar();
		mem_prof.Start_stop_profiling_all_processes();
		cout << "Signal sent" << endl;
		mem_prof.Remove_all_process_from_profiling();
		cout << "Removed all from profiled" << endl;
		getchar();
		mem_prof.Print_all_processes_shared_memory();*/

		//sleep(3);
	}

	return 0;
}
