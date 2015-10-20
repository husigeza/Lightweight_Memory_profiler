#include "mem_prof_static.h"
#include <stdio.h>

int enable = 0;

int profiling_allowed(void) {

    if(enable == 0) return 0;
    else return 1;
}


void signal_callback_handler(int signum) {

    if(profiling_allowed()) enable = 0;
    else enable = 1;

}



