#include <stdio.h>
#include <signal.h>




extern "C" {

    extern void *__libc_malloc(size_t size);
    extern void *__libc_free(void *);

    int __attribute__((weak)) profiling_allowed(void);

   // void __attribute__((weak)) signal_callback_handler(int signum);

    void _init() {

       printf("Shared object init message\n");
       //signal(SIGINT, signal_callback_handler);
        printf("Shared object init message END\n");
       return;
    }

    void* malloc(size_t size) {
        //char *error;
        //void *caller = __builtin_return_address(0);
        printf("In malloc, before check\n");
        if(profiling_allowed()) printf("!!!!!!!!!!!!This is from my malloc!!!!!!!!!!!!\n");

        //printf("Caller: %s\n",(char*)caller);

        return __libc_malloc(size);
    }

    void free(void* pointer) {

        if(profiling_allowed()) {
            printf("!!!!!!!!!!!!This is from my free!!!!!!!!!!!!!!!\n");
        }
        __libc_free(pointer);
        return;
    }


}

