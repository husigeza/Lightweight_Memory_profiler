#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "../Memory_Profiler_Static_library/mem_prof_static.h"


extern void *__libc_malloc(size_t size);
extern void  __libc_free(void *);

void *handle;
int __attribute__((weak)) SampleAddInt (int a, int b);

//extern int enable;


int main()
{
    char *error;
    int *result_malloc;

    printf("Before first malloc\n");
    result_malloc = malloc(sizeof(int));
    free(result_malloc);
    printf("After first malloc\n");


    SampleAddInt(3,4);

    printf("Before second malloc\n");
    printf("Set enable to 1\n");
    enable = 1;
    result_malloc = malloc(sizeof(int));
    free(result_malloc);
    printf("After second malloc\n");



    printf("Before third malloc\n");
    printf("Set enable to 0\n");
    enable = 0;
    result_malloc = malloc(sizeof(int));
    free(result_malloc);
    printf("After third malloc\n");


    return 0;
}



