#ifndef MEM_PROF_DLL_H_INCLUDED
#define MEM_PROF_DLL_H_INCLUDED

void* Hearthbeat(void *arg);
bool profiling_allowed(void);
void signal_callback_handler(int signum);


#endif // MEM_PROF_DLL_H_INCLUDED
