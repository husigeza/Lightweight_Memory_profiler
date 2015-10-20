#include <stdio.h>
#include <signal.h>

#include <unistd.h>


#ifdef __cplusplus
extern "C" {
#endif

class Memory_profiler_DLL{

public:
    void init() {current_process = getpid();}

    int GetCurrentProcess();
private:
    int current_process;
};


int Memory_profiler_DLL::GetCurrentProcess(){
        return current_process;
    }


    extern void *__libc_malloc(size_t size);
    extern void *__libc_free(void *);

    extern int profiling_allowed(void);
    void signal_callback_handler(int signum);

    void _init() {

        printf("Shared object init message\n");
        signal(SIGUSR1, signal_callback_handler);
        printf("Shared object init message END\n");

        Memory_profiler_DLL mem_prof;
        mem_prof.init();
        printf("current process: %d\n",mem_prof.GetCurrentProcess());

        return;
    }

    void* malloc(size_t size) {
        //char *error;
        //void *caller = __builtin_return_address(0);

        if(profiling_allowed()) printf("This is from my malloc!\n");

        //printf("Caller: %s\n",(char*)caller);

        return __libc_malloc(size);
    }

    void free(void* pointer) {

        if(profiling_allowed()) {
            printf("This is from my free!\n");
        }
        __libc_free(pointer);
        return;
    }
#ifdef __cplusplus
}
#endif



