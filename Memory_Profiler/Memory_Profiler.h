#ifndef MEMORY_PROFILER_H_INCLUDED
#define MEMORY_PROFILER_H_INCLUDED

#define max_length 10000


typedef struct malloc_struct_s {
    int len;
    int buf[max_length];
}malloc_struct_t;

class Process_handler {

    uint32_t PID;
    void Init_shared_memory();
    malloc_struct_t *malloc_struct;

    public:
        Process_handler(uint32_t PID);
        ~Process_handler();

        uint32_t GetPID(){return PID;};

        void Print_shared_memory();
};


class Memory_Profiler {


        std::vector<uint32_t> All_processes;
        std::vector<uint32_t> Profiled_processes;

    public:
        Memory_Profiler();
        ~Memory_Profiler();

        void Add_Process_to_list(uint32_t PID);

        void Add_process_to_profiling(uint32_t PID);
        void Add_all_process_to_profiling();

        std::vector<uint32_t> Get_profiled_processes_list()const;
        std::vector<uint32_t> Get_all_processes_list()const;

        void Print_all_processes()const;
        void Print_profiled_processes()const;

        void Read_FIFO();

};

#endif // MEMORY_PROFILER_H_INCLUDED
