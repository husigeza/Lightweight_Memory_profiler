#ifndef MEMORY_PROFILER_H_INCLUDED
#define MEMORY_PROFILER_H_INCLUDED

#include <map>

#include "Memory_Profiler_process.h"

//using namespace std;

class Memory_Profiler {

        map<pid_t const,Process_handler> Processes;
        string fifo_path;
        int mem_prof_fifo;

        void Set_process_alive_flag(const pid_t PID, bool value);

        bool Remap_process_shared_memory(const pid_t PID);
        void Remap_all_process_shared_memory();

        void Start_stop_profiling(const pid_t PID);
        void Start_stop_profiling_all_processes();

    public:
        Memory_Profiler();
        Memory_Profiler(string fifo_path);
        ~Memory_Profiler();

        bool Add_Process_to_list(const pid_t PID);

        void Add_process_to_profiling(const pid_t PID);
        void Add_all_process_to_profiling();

        void Remove_process_from_profiling(const pid_t PID);
        void Remove_all_process_from_profiling();

        bool Get_process_alive_flag(const pid_t PID);
        bool Get_process_shared_memory_initilized_flag(const pid_t PID);


        void Read_FIFO();

        void Analyze_process(const pid_t PID);
        void Analyze_all_process();

        void Print_process(const pid_t PID) const;
        void Print_all_processes() const;
        void Print_alive_processes() const;
        void Print_profiled_processes() const;
        void Print_process_shared_memory(const pid_t PID) const;
        void Print_all_processes_shared_memory() const;
        void Print_process_symbol_table(const pid_t PID) const;
};

#endif // MEMORY_PROFILER_H_INCLUDED
