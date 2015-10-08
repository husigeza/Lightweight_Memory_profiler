#include <stdio.h>



#ifdef __cplusplus
extern "C" {
#endif

extern void *__libc_malloc(size_t size);
extern void *__libc_free(void *);
int __attribute__((weak)) profiling_allowed(void);
int enable = 0;

    void _init() {

       printf("Shared object init message\n");
       return;
    }


    int SampleAddInt(int a, int b) {

        printf("Writing out from shared lib, SampleAddInt\n");
        return a+b;
    }


    void* malloc(size_t size) {
        //char *error;
        void *caller = __builtin_return_address(0);

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

#ifdef __cplusplus
}
#endif
