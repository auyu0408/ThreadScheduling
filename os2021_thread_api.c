#include "all_include.h"

struct itimerval Signaltimer;
ucontext_t dispatch_context;//dispatcher context
int th_num = 0;//total thread_id
long time_past = 0;//time quantum

thread_tptr run = NULL;
/*priority feedback queue head*/
thread_tptr ready_head = NULL;
thread_tptr wait_head = NULL;
thread_tptr terminate_head = NULL;

/*priority parameter*/
char priority_char[3] = {'L', 'M', 'H'};
int tq[3] = {300, 200, 100};//time quantum value, tq[0] = low priority's time quantum

int OS2021_ThreadCreate(char *job_name, char *p_function, char *priority, int cancel_mode)
{
    thread_tptr new_th = create_thread(job_name, th_num, priority, cancel_mode);
    th_num++;

    if(strcmp(p_function, "Function1") == 0)
        CreateContext(&(new_th->th_ctx), &dispatch_context, &Function1);
    else if(strcmp(p_function, "Function2") == 0)
        CreateContext(&(new_th->th_ctx), &dispatch_context, &Function2);
    else if(strcmp(p_function, "Function3") == 0)
        CreateContext(&(new_th->th_ctx), &dispatch_context, &Function3);
    else if(strcmp(p_function, "Function4") == 0)
        CreateContext(&(new_th->th_ctx), &dispatch_context, &Function4);
    else if(strcmp(p_function, "Function5") == 0)
        CreateContext(&(new_th->th_ctx), &dispatch_context, &Function5);
    else if(strcmp(p_function, "ResourceReclaim") == 0)
        CreateContext(&(new_th->th_ctx), NULL, &ResourceReclaim);
    else
    {
        free(new_th);
        return -1;
    }

    enq(&new_th, &ready_head);
    return new_th->th_id;
}

void OS2021_ThreadCancel(char *job_name)
{
    thread_tptr target = NULL;
    thread_tptr temp_th = ready_head;
    thread_tptr ex_th = NULL;
    if(strcmp("reclaimer", job_name)==0)
        return;//reclaimer can't enter terminate state
    /*tried to find target in ready queue or wait queue */
    while (temp_th!=NULL)
    {
        if(strcmp(temp_th->th_name, job_name)==0)
        {
            target = temp_th;
            break;
        }
        else
        {
            ex_th = temp_th;
            temp_th = temp_th->th_next;
        }
    }
    if(target == NULL)
    {
        temp_th = wait_head;
        ex_th = NULL;
        while(temp_th != NULL)
        {
            if(strcmp(temp_th->th_name, job_name) == 0)
            {
                target = temp_th;
                break;
            }
            else
            {
                ex_th = temp_th;
                temp_th = temp_th->th_next;
            }
        }
    }
    if(target == NULL)
    {
        temp_th = run;
        if(strcmp(temp_th->th_name, job_name)==0)
        {
            target = temp_th;
            target->th_cbit = 1;
            if(target->th_cancelmode == 0)
                enq(&target, &terminate_head);
            else
            {
                printf("%s wants to cancel thread %s\n", run->th_name, target->th_name);
                return;
            }
            swapcontext(&(target->th_ctx), &dispatch_context);
        }
    }
    /*if find target(not running), move it to terminate queue or change state*/
    if(target != NULL)
    {
        target->th_cbit = 1;
        if(target->th_cancelmode == 0)
        {
            /*dequeue from original queue*/
            if(target == wait_head)
                wait_head = target->th_next;
            else if(target == ready_head)
                ready_head = target->th_next;
            else
                ex_th->th_next = target->th_next;
            printf("%s cancel thread %s\n", run->th_name, target->th_name);
            enq(&target, &terminate_head);//enqueue to terminate queue
        }
        else
            printf("%s wants to cancel thread %s\n", run->th_name, target->th_name);
    }
    return;
}

void OS2021_ThreadWaitEvent(int event_id)
{
    thread_t *target = run;
    target->th_wait = event_id;
    printf("%s wants to waiting for event %d\n", target->th_name, event_id);
    priority_change(&target, time_past, tq[target->th_priority]);//change priority
    enq(&target, &wait_head);//change to wait queue
    swapcontext(&(target->th_ctx), &dispatch_context);//save current status and reschedule
    return;
}

void OS2021_ThreadSetEvent(int event_id)
{
    thread_tptr bullet_th = run;
    thread_tptr hit_th = NULL;
    thread_tptr temp_th = wait_head;
    thread_tptr ex_th = NULL;
    //try to find target
    while(temp_th != NULL)
    {
        if(temp_th->th_wait != event_id)
        {
            ex_th = temp_th;
            temp_th = temp_th->th_next;
        }
        else
        {
            hit_th = temp_th;
            hit_th->th_wait = -1;
            if(hit_th == wait_head)
                wait_head = wait_head->th_next;
            else
            {
                ex_th->th_next = hit_th->th_next;
                hit_th->th_next = NULL;
            }
            printf("%s changes the status of %s to READY.\n", bullet_th->th_name, hit_th->th_name);
            enq(&hit_th, &ready_head);
            return;
        }
    }
    return;
}

void OS2021_ThreadWaitTime(int msec)
{
    thread_t *target = run;
    priority_change(&target, time_past, tq[target->th_priority]);
    target->th_waittime = msec;
    target->th_next = NULL;
    enq(&target, &wait_head);
    swapcontext(&(target->th_ctx), &dispatch_context);//save current status and reschedule
    return;
}

void OS2021_DeallocateThreadResource()
{
    thread_tptr target = terminate_head;
    while(target != NULL)
    {
        printf("The memory space by %s has been released.\n", target->th_name);
        terminate_head = terminate_head->th_next;
        free(target);
        target = terminate_head;
    }
    return;
}

void OS2021_TestCancel()
{
    if((run->th_cbit))//change when it was cancel by somebody
    {
        enq(&run, &terminate_head);//change to terminate state
        setcontext(&dispatch_context);
    }
}

void CreateContext(ucontext_t *context, ucontext_t *next_context, void *func)
{
    getcontext(context);
    context->uc_stack.ss_sp = malloc(STACK_SIZE);
    context->uc_stack.ss_size = STACK_SIZE;
    context->uc_stack.ss_flags = 0;
    context->uc_link = next_context;
    makecontext(context,(void (*)(void))func,0);
}

void ResetTimer()
{
    Signaltimer.it_value.tv_sec = 0;
    Signaltimer.it_value.tv_usec = 10000;
    if(setitimer(ITIMER_REAL, &Signaltimer, NULL) < 0)
        printf("ERROR SETTING TIME SIGALRM!\n");
}

void TimerHandler()
{
    time_past += 10;
    //calculate related time
    thread_tptr temp_th = ready_head;
    thread_tptr ex_th = NULL;
    while(temp_th != NULL)
    {
        temp_th->th_qtime += 10;
        temp_th = temp_th->th_next;
    }
    // add wait time
    temp_th = wait_head;
    while(temp_th != NULL)
    {
        temp_th->th_wtime += 10;
        if(temp_th->th_waittime != 0)
        {
            thread_tptr target = temp_th;
            thread_tptr target_ex = ex_th;
            target->th_already_wait ++;
            if(target->th_already_wait >= target->th_waittime)
            {
                target->th_waittime = 0;
                target->th_already_wait = 0;
                if(target == wait_head)
                    wait_head = wait_head->th_next;
                else
                    target_ex->th_next = target->th_next;
                enq(&target, &ready_head);
            }
        }
        ex_th = temp_th;
        temp_th = temp_th->th_next;
    }
    //if time excess time quantum, change another thread
    if(time_past >= tq[run->th_priority])
    {
        if(run->th_cbit == 1)
            enq(&run, &terminate_head);
        //change priority
        else
        {
            if(run->th_priority !=0)
            {
                run->th_priority--;
                printf("The priority of thread %s is changed from %c to %c\n", run->th_name, priority_char[run->th_priority+1], priority_char[run->th_priority]);
            }
            enq(&run, &ready_head);//send it to ready queue
        }
        swapcontext(&(run->th_ctx), &dispatch_context);//reschedule
    }
    //if not execeed, keep going
    ResetTimer();
    return;
}

void Report(int signal)
{
    printf("\n");
    printf("**************************************************************************************************\n");
    printf("*\tTID\tName\t\tState\t\tB_Priority\tC_Priority\tQ_Time\tW_time\t *\n");
    printf("*\t%d\t%-10s\tRUNNING\t\t%c\t\t%c\t\t%ld\t%ld\t *\n",
           run->th_id, run->th_name, priority_char[run->b_priority], priority_char[run->th_priority], run->th_qtime, run->th_wtime);
    thread_tptr temp_th = ready_head;
    while(temp_th!=NULL)
    {
        printf("*\t%d\t%-10s\tREADY\t\t%c\t\t%c\t\t%ld\t%ld\t *\n",
               temp_th->th_id, temp_th->th_name, priority_char[temp_th->b_priority], priority_char[temp_th->th_priority], temp_th->th_qtime, temp_th->th_wtime);
        temp_th = temp_th->th_next;
    }
    temp_th = wait_head;
    while(temp_th!=NULL)
    {
        printf("*\t%d\t%-10s\tWAITING\t\t%c\t\t%c\t\t%ld\t%ld\t *\n",
               temp_th->th_id, temp_th->th_name, priority_char[temp_th->b_priority], priority_char[temp_th->th_priority], temp_th->th_qtime, temp_th->th_wtime);
        temp_th = temp_th->th_next;
    }
    printf("**************************************************************************************************\n");
    return;
}

void Dispatcher()
{
    run = deq(&ready_head);
    time_past = 0;
    ResetTimer();
    setcontext(&(run->th_ctx));
}

void StartSchedulingSimulation()
{
    /*Set Timer and Interrupt*/
    Signaltimer.it_interval.tv_usec = 0;
    Signaltimer.it_interval.tv_sec = 0;
    signal(SIGALRM, TimerHandler);
    signal(SIGTSTP, Report);
    /*Create Context*/
    CreateContext(&dispatch_context, NULL, &Dispatcher);
    /*Create thread*/
    OS2021_ThreadCreate("reclaimer", "ResourceReclaim", "L", 1);
    ParsedJson();
    /*Start Scheduling*/
    setcontext(&dispatch_context);
}