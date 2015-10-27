#ifndef MEM_PROF_STATIC_H_INCLUDED
#define MEM_PROF_STATIC_H_INCLUDED

extern int enable;
int profiling_allowed(void);
void signal_callback_handler(int signum);

#endif // MEM_PROF_STATIC_H_INCLUDED
