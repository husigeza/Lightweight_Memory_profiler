#ifndef MEMORY_PROFILER_H_INCLUDED
#define MEMORY_PROFILER_H_INCLUDED

#include "Memory_Profiler_process.h"
#include "Memory_Profiler_handler_template.h"

#include <memory>

#include "Memory_Profiler_analyzer.h"
#include "Memory_Profiler_filter.h"
#include "Memory_Profiler_pattern.h"

class Memory_Profiler {

		string fifo_path;
		string mem_prof_overload_fifo_path;
		int mem_prof_fifo;
		int mem_prof_overload_fifo;

		sem_t save_sem;

		map<pid_t const, template_handler<Process_handler> > Processes;
        vector< template_handler<Pattern> > Patterns_vector;
        vector< template_handler<Analyzer> > Analyzers_vector;
        vector< template_handler<Filter> > Filters_vector;

        void Start_stop_profiling(const pid_t PID);
        void Start_stop_profiling_all_processes();

        void Save_process_shared_memory(pid_t PID);

    public:
        Memory_Profiler();
        Memory_Profiler(string fifo_path, string overload_fifo_path);
        ~Memory_Profiler();

        void Read_FIFO();
        void Read_overload_FIFO();

        bool Add_Process_to_list(const pid_t PID);

        void Add_process_to_profiling(const pid_t PID);
        void Add_all_process_to_profiling();
        void Remove_process_from_profiling(const pid_t PID);
        void Remove_all_process_from_profiling();

        void Create_new_pattern(string name);
        vector< template_handler<Pattern> >::iterator Find_pattern_by_name(string Pattern_name);
        void Print_patterns() const;
        void Print_pattern(string pattern_name);

        void Create_new_analyzer(Analyzer& analyzer);
        void Remove_analyzer(unsigned int analyzer_index);
        void Print_analyzers() const;
        void Print_analyzer(unsigned int index) const;

        void Create_new_filter(Filter& filter);
        void Create_new_size_filter_cli(unsigned long size_p, string operation_p);
		void Create_new_time_filter_cli(string time,__suseconds_t usec,string time_type, string operation_p);
        void Remove_filter(unsigned int filter_index);
        void Print_filters() const;
        void Print_filter(unsigned int index) const;

        void Add_analyzer_to_pattern(unsigned int analyzer_index,unsigned int pattern_index);
        void Add_analyzer_to_pattern_by_name(unsigned int analyzer_index,string pattern_name);
        void Add_analyzer_all_to_pattern_by_name(string pattern_name);
        void Add_filter_to_pattern(unsigned int filter_index,unsigned int pattern_index);
        void Add_filter_to_pattern_by_name(unsigned int analyzer_index,string pattern_name);

        void Remove_analyzer_from_pattern(unsigned int analyzer_index,unsigned int pattern_index);
        void Remove_analyzer_from_pattern_by_name(unsigned int analyzer_index,string pattern_name);
        void Remove_filter_from_pattern(unsigned int filter_index,unsigned int pattern_index);
        void Remove_filter_from_pattern_by_name(unsigned int filter_index,string pattern_name);

        void Run_pattern(unsigned int pattern_index, pid_t PID);
        void Run_pattern(string pattern_name, pid_t PID);
        void Run_pattern_all_process(unsigned int pattern_index);
        void Run_pattern_all_process(string name);

        void Print_process(const pid_t PID) const;
        void Print_all_processes() const;
        void Print_alive_processes() const;
        void Print_profiled_processes() const;

};

#endif // MEMORY_PROFILER_H_INCLUDED
