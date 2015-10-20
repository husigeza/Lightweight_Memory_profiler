#ifndef MEMORY_PROFILER_H_INCLUDED
#define MEMORY_PROFILER_H_INCLUDED


int filter_func(const struct dirent * directory);

class Process_handler {

    uint32_t PID;

    public:
        Process_handler(uint32_t PID) {this->PID = PID;}
        uint32_t GetPID(){return PID;};

};


class Memory_Profiler {

        int NumOfProcesses = 0;
        struct dirent **ProcessesList;
        std::vector<Process_handler> Profiled_Processes;


    public:
        ~Memory_Profiler();
        void GetProcessList();
        void Print_processes()const;

        void Add_Process(uint32_t PID);
        void Add_all_Process();
        std::vector<Process_handler> Get_profiled_processes_list()const;
        void Print_profiled_processes();

};

#endif // MEMORY_PROFILER_H_INCLUDED
