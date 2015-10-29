#ifndef MEMORY_PROFILER_H_INCLUDED
#define MEMORY_PROFILER_H_INCLUDED

#define max_length 10000


typedef struct malloc_struct_s {
    int len;
    int buf[max_length];
}malloc_struct_t;

class Process_handler {

    pid_t PID;
    bool profiled;
    int shared_memory;



    public:

    malloc_struct_t *malloc_struct;
    	//Process_handler();
        Process_handler(pid_t PID);
        ~Process_handler();

        Process_handler(const Process_handler &obj);

        void Init_shared_memory();

        pid_t GetPID(){return PID;};

        void Set_profiled(bool value){this->profiled = value;};
        bool Get_profiled(){return profiled;};

        void Send_signal();

        malloc_struct_t* Get_shared_memory();
};


class Memory_Profiler {


        std::map<pid_t,Process_handler> Processes;

    public:

        Memory_Profiler();
        ~Memory_Profiler();

        void Add_Process_to_list(pid_t PID);

        void Add_process_to_profiling(pid_t PID);
        void Add_all_process_to_profiling();

        std::map<pid_t,Process_handler> Get_profiled_processes_list()const;
        std::map<pid_t,Process_handler> Get_all_processes_list()const;

        void Send_signal_to_process(pid_t PID);
        void Send_signal_to_all_processes();

        void Print_all_processes()const;
        void Print_profiled_processes();

        void Print_profiled_process_shared_memory(pid_t PID);
        void Print_all_profiled_processes_shared_memory();

        void Read_FIFO();

};

#endif // MEMORY_PROFILER_H_INCLUDED
