#ifndef MEMORY_PROFILER_SHARED_LIBRARY_H_INCLUDED
#define MEMORY_PROFILER_SHARED_LIBRARY_H_INCLUDED

#include <stdbool.h>

void* Hearthbeat(void *arg);
bool profiling_allowed(void);
void signal_callback_handler(int signum);


#endif // MEMORY_PROFILER_SHARED_LIBRARY_H_INCLUDED
