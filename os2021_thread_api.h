#ifndef OS2021_API_H
#define OS2021_API_H

#define STACK_SIZE 8192

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <json-c/json.h>//parse jsonfile
#include "function_libary.h"

//self thread structure
struct thread
{
    char *th_name;
    int th_id;
    //priority: H=2, M=1, L=0
    int b_priority;//original priority
    int th_priority;//current priority
    int th_cancelmode;//0=asynchronous, 1=deffer(reclaimer can't goto the end)
    int th_cancel_status;//0=nothing, 1=waiting cancel
    int th_wait;//event I want wait, -1 = no waiting
    long th_qtime;//queueing time
    long th_wtime;//waiting time
    long th_waittime;//if you need ThreadWaitTime
    long th_already_wait;
    ucontext_t th_ctx;//to save your context
    struct thread *th_next;
};
typedef struct thread thread_t;
typedef struct thread * thread_tptr;

void enq(thread_tptr *, thread_tptr *);
void Timer_Handler();
void Scheduler();

int OS2021_ThreadCreate(char *job_name, char *p_function, int priority, int cancel_mode);
void OS2021_ThreadCancel(char *job_name);
void OS2021_ThreadWaitEvent(int event_id);
void OS2021_ThreadSetEvent(int event_id);
void OS2021_ThreadWaitTime(int msec);
void OS2021_DeallocateThreadResource();
void OS2021_TestCancel();


void CreateContext(ucontext_t *, ucontext_t *, void *);
void ResetTimer();
void Dispatcher();
void StartSchedulingSimulation();
void ParsedJson();
void Report(int signal);

#endif
