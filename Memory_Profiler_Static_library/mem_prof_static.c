#include "mem_prof_static.h"


int enable = 0;

int profiling_allowed(void) {

    if( enable == 0) return 0;
    else return 1;
}



