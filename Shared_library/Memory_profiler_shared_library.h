#ifndef MEMORY_PROFILER_SHARED_LIBRARY_H_INCLUDED
#define MEMORY_PROFILER_SHARED_LIBRARY_H_INCLUDED

#include <stdbool.h>

void* Memory_profiler_start_thread(void *arg);
void* Hearthbeat(void *arg);
bool profiling_allowed(void);
void set_profiling(bool value);



#endif // MEMORY_PROFILER_SHARED_LIBRARY_H_INCLUDED
