#ifndef MEMORY_PROFILER_H_INCLUDED
#define MEMORY_PROFILER_H_INCLUDED

#include <map>
#include "Process.h"

using namespace std;

class Memory_Profiler {

        map<pid_t const,Process_handler> Processes;

    public:

        Memory_Profiler();
        ~Memory_Profiler();

        void Add_Process_to_list(const pid_t PID);

        void Add_process_to_profiling(const pid_t PID);
        void Add_all_process_to_profiling();
        map<const pid_t,Process_handler>& Get_profiled_processes_list();
        map<const pid_t,Process_handler>& Get_all_processes_list();

        void Remove_process_from_profiling(const pid_t PID);
        void Remove_all_process_from_profiling();

        void Send_signal_to_process(const pid_t PID);
        void Send_signal_to_all_processes();

        void Start_stop_profiling(const pid_t PID);
        void Start_stop_profiling_all_processes();

        void Read_FIFO();

        void Print_all_processes()const;
        void Print_profiled_processes();
        void Print_profiled_process_shared_memory(const pid_t PID);
        void Print_all_profiled_processes_shared_memory();



};

#endif // MEMORY_PROFILER_H_INCLUDED
