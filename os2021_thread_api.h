#ifndef OS2021_API_H
#define OS2021_API_H

#include "all_include.h"

#define STACK_SIZE 8192

/*API*/
int OS2021_ThreadCreate(char *job_name, char *p_function, char *priority, int cancel_mode);
void OS2021_ThreadCancel(char *job_name);
void OS2021_ThreadWaitEvent(int event_id);
void OS2021_ThreadSetEvent(int event_id);
void OS2021_ThreadWaitTime(int msec);
void OS2021_DeallocateThreadResource();
void OS2021_TestCancel();

void CreateContext(ucontext_t *, ucontext_t *, void *);
void ResetTimer();
void Timer_Handler();
void Scheduler();
void Dispatcher();
void StartSchedulingSimulation();
void Report(int signal);

#endif
