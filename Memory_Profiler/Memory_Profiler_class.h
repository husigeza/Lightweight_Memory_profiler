#ifndef MEMORY_PROFILER_H_INCLUDED
#define MEMORY_PROFILER_H_INCLUDED

#include "Memory_Profiler_process.h"
#include "Memory_Profiler_analyzer.h"
#include <memory>

#include "Memory_Profiler_pattern.h"

class Memory_Profiler {

        map<pid_t const,Process_handler> Processes;
        string fifo_path;
        int mem_prof_fifo;

        void Set_process_alive_flag(const pid_t PID, bool value);

        bool Remap_process_shared_memory(const pid_t PID);
        void Remap_all_process_shared_memory();

        void Start_stop_profiling(const pid_t PID);
        void Start_stop_profiling_all_processes();

        vector< unique_ptr<Pattern> > Patterns_vector;
        vector< shared_ptr<Analyzer> > Analyzers_vector;
        vector< shared_ptr<Filter_class> > Filters_vector;

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

        bool Process_analyze_ready(const pid_t PID);

        void Analyze_process(const pid_t PID);
        void Analyze_all_process();

        void Analyze_process_memory_leak(const pid_t PID);
        void Analyze_process_dummy_free(const pid_t PID);

        void Create_new_pattern(string name);
        void Print_patterns() const;

        void Create_new_analyzer(shared_ptr<Analyzer> analyzer);
        void Print_analyzers() const;

        void Create_new_filter_cli(unsigned long size_p, string operation_p);
        void Create_new_filter(shared_ptr<Filter_class> filter);
        void Print_filters() const;

        void Add_analyzer_to_pattern(unsigned int analyzer_index,unsigned int pattern_index);
        void Add_filter_to_pattern(unsigned int filter_index,unsigned int pattern_index);

        void Run_pattern(unsigned int pattern_index, Process_handler &process);
        void Run_pattern_all_process(unsigned int pattern_index);

        void Print_process(const pid_t PID) const;
        void Print_all_processes() const;
        void Print_alive_processes() const;
        void Print_profiled_processes() const;
        void Print_process_shared_memory(const pid_t PID) const;
        void Print_all_processes_shared_memory() const;

        void Save_process_symbol_table_to_file(const pid_t PID);
        void Save_process_memory_mapping_to_file(const pid_t PID);
        void Save_process_shared_memory_to_file(const pid_t PID);
        void Save_all_process_shared_memory_to_file();

};

#endif // MEMORY_PROFILER_H_INCLUDED
