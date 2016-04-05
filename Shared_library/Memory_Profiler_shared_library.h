#ifndef MEMORY_PROFILER_SHARED_LIBRARY_H_INCLUDED
#define MEMORY_PROFILER_SHARED_LIBRARY_H_INCLUDED

#include <stdbool.h>

void* Memory_profiler_start_thread(void *arg);
void* Hearthbeat(void *arg);
bool profiling_allowed(void);
void set_profiling(bool value);

void set_user_profiling_flag(bool value);
bool get_user_profiling_flag();

bool Create_shared_memory();
bool Open_shared_memory();

void swap_shared_memory_pointers();
void indicate_shm_overload();



#endif // MEMORY_PROFILER_SHARED_LIBRARY_H_INCLUDED
