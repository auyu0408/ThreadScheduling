#include "os2021_thread_api.h"

struct itimerval Signaltimer;
ucontext_t dispatch_context;
//ucontext_t timer_context;
ucontext_t finish_context;//if entry function will goto end, this will be used.
int th_num = 0;//use for set thread_id
long time_pass = 0;//used to compare with time quantum

thread_tptr pick = NULL;
thread_tptr run = NULL;

thread_tptr ready_head = NULL;
thread_tptr wait_head = NULL;
thread_tptr terminate_head = NULL;

char priority_char[3] = {'L', 'M', 'H'};
int tq[3] = {300, 200, 100};//time quantum value, tq[0] = low priority's time quantum

//queue: H->H->...M->M->M...->L, previous H is earlier
void enq(thread_tptr *new_th, thread_tptr *head)
{
    thread_tptr temp = (*head);
    if(temp!=NULL)
    {
        while(temp->th_next != NULL)
        {
            temp = temp->th_next;
        }
        temp ->th_next = (*new_th);
    }
    else//empty queue
    {

        (*head) = (*new_th);
    }
    (*new_th)->th_next = NULL;
    return;
}

thread_tptr deq(thread_tptr *head)
{
    if((*head) == NULL)
        return NULL;
    else
    {
        thread_tptr leave = (*head);
        (*head) = (*head)->th_next;
        leave->th_next = NULL;
        return leave;
    }
}

int OS2021_ThreadCreate(char *job_name, char *p_function, int priority, int cancel_mode)
{
    thread_t *new_th = malloc(sizeof(thread_t));
    new_th->th_name = job_name;
    new_th->th_id = th_num++;//th_id = th_num(original), th_num = th_num+1
    new_th->b_priority = new_th->th_priority = priority;
    new_th->th_cancelmode = cancel_mode;
    new_th->th_cancel_status = 0;
    new_th->th_wait = -1;
    new_th->th_qtime = 0;
    new_th->th_wtime = 0;
    new_th->th_waittime = 0;
    new_th->th_already_wait = 0;
    new_th->th_next = NULL;
    new_th->th_previous = NULL;

    if(strcmp(p_function, "Function1") == 0)
        CreateContext(&(new_th->th_ctx), &finish_context, &Function1);
    else if(strcmp(p_function, "Function2") == 0)
        CreateContext(&(new_th->th_ctx), &finish_context, &Function2);
    else if(strcmp(p_function, "Function3") == 0)
        CreateContext(&(new_th->th_ctx), &finish_context, &Function3);
    else if(strcmp(p_function, "Function4") == 0)
        CreateContext(&(new_th->th_ctx), &finish_context, &Function4);
    else if(strcmp(p_function, "Function5") == 0)
        CreateContext(&(new_th->th_ctx), &finish_context, &Function5);
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
    //reclaimer can't enter terminate state
    if(strcmp("reclaimer", job_name)==0)
        return;
    //tried to find target in ready queue
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
    //tried to find target in wait queue
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
    //after find target, move it to terminate queue or change state
    if(target != NULL)
    {
        if(target->th_cancelmode == 0)
        {
            //dequeue from original queue
            ex_th->th_next = target->th_next;
            enq(&target, &terminate_head);//enqueue to terminate queue
            //no need to reschedule
        }
        else
        {
            target->th_cancel_status = 1;//change cancel state but not goto terminate now, wait for cancel point
            printf("%s wants to cancel thread %s\n", run->th_name, target->th_name);
            //no need to reschedule
        }
    }
    return;
}

void OS2021_ThreadWaitEvent(int event_id)
{
    thread_t *target = run;
    target->th_wait = event_id;

    //change priority
    if(time_pass < tq[target->th_priority])
    {
        if(target->th_priority != 2)
        {
            target->th_priority++;
            printf("The priority of thread %s is changed from %c to %c\n",
                   target->th_name, priority_char[target->th_priority-1], priority_char[target->th_priority]);
        }
    }
    else
    {
        if(target->th_priority != 0)
        {
            target->th_priority--;
            printf("The priority of thread %s is changed from %c to %c\n",
                   target->th_name, priority_char[target->th_priority+1], priority_char[target->th_priority]);
        }
    }

    //change to wait queue
    enq(&target, &wait_head);
    printf("%s wants to waiting for event %d\n", target->th_name, event_id);
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
            {
                wait_head = wait_head->th_next;
            }
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
    target->th_waittime = msec*10;
    enq(&target, &wait_head);
    swapcontext(&(target->th_ctx), &dispatch_context);//save current status and reschedule
    return;
}

void OS2021_DeallocateThreadResource()
{
    thread_tptr target = terminate_head;
    while(target != NULL)
    {
        terminate_head = terminate_head->th_next;
        free(target);
        target = terminate_head;
    }
    return;
}

void OS2021_TestCancel()
{
    thread_t *target = run;
    if(target->th_cancel_status == 1)//change when it was cancel by somebody
    {
        enq(&target, &terminate_head);//change to terminate state
        setcontext(&dispatch_context);//reschedule
    }
    else
        return;
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
    Signaltimer.it_value.tv_usec = 10;
    if(setitimer(ITIMER_REAL, &Signaltimer, NULL) < 0)
    {
        printf("ERROR SETTING TIME SIGALRM!\n");
    }
}

void TimerHandler()
{
    //calculate related time
    thread_tptr temp_th = ready_head;
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
        temp_th = temp_th->th_next;
    }
    //if time excess time quantum, change another thread
    //if(time_pass >= tq[run->th_priority])
    //{
    //    //change priority
    //    if(run->th_priority !=0)
    //    {
    //        run->th_priority--;
    //        printf("The priority of thread %s is changed from %c to %c\n", run->th_name, priority_char[run->th_priority+1], priority_char[run->th_priority]);
    //    }
    enq(&run, &ready_head);//send it to ready queue
    swapcontext(&(run->th_ctx), &dispatch_context);//reschedule
    //}
    //if not execeed, keep going
    ResetTimer();
    return;
}

void Report(int signal)
{
    printf("\n");
    printf("**************************************************************************************************\n");
    printf("*\tTID\tName\t\tState\t\tB_Priority\tC_Priority\tQ_Time\tW_time\t *\n");
    printf("*\t%d\t%s\tRUNNING\t\t%c\t\t%c\t\t%ld\t%ld\t *\n",
           run->th_id, run->th_name, priority_char[run->b_priority], priority_char[run->th_priority], run->th_qtime, run->th_wtime);
    thread_tptr temp_th = ready_head;
    while(temp_th!=NULL)
    {
        printf("*\t%d\t%s\tREADY\t\t%c\t\t%c\t\t%ld\t%ld\t *\n",
               temp_th->th_id, temp_th->th_name, priority_char[temp_th->b_priority], priority_char[temp_th->th_priority], temp_th->th_qtime, temp_th->th_wtime);
        temp_th = temp_th->th_next;
    }
    temp_th = wait_head;
    while(temp_th!=NULL)
    {
        printf("*\t%d\t%s\tWAITING\t\t%c\t\t%c\t\t%ld\t%ld\t *\n",
               temp_th->th_id, temp_th->th_name, priority_char[temp_th->b_priority], priority_char[temp_th->th_priority], temp_th->th_qtime, temp_th->th_wtime);
        temp_th = temp_th->th_next;
    }
    printf("**************************************************************************************************\n");
    return;
}

void Scheduler()
{
    pick = deq(&ready_head);
}

void Dispatcher()
{
    //haven't handle: begining
    //thread_tptr temp = ready_head;
    //while(temp!=NULL)
    //{
    //    printf("%s ", temp->th_name);
    //    temp = temp->th_next;
    //}
    //printf("\n");
    run = NULL;
    Scheduler();
    run = pick;
    time_pass = 0;
    //if(ready_head == NULL)
    //{
    //    printf("No thread to run, now run:%s\n", run->th_name);
    //}
    ResetTimer();
    setcontext(&(run->th_ctx));
}

void FinishThread()
{
    thread_tptr target = run;
    run = NULL;
    enq(&target, &terminate_head);//change from run to terminate state
    setcontext(&dispatch_context);//reschedule
}

void StartSchedulingSimulation()
{
    /*Set Timer*/
    Signaltimer.it_interval.tv_usec = 100;
    Signaltimer.it_interval.tv_sec = 0;
    signal(SIGALRM, TimerHandler);
    signal(SIGTSTP, Report);
    /*Create Context*/
    CreateContext(&dispatch_context, NULL, &Dispatcher);
    CreateContext(&finish_context, &dispatch_context, &FinishThread);
    //create thread
    OS2021_ThreadCreate("my_thread1", "Function1", 2, 1);
    //OS2021_ThreadCreate("my_thread2", "Function5", 1, 1);
    OS2021_ThreadCreate("reclaimer", "ResourceReclaim", 0, 1);
    //ParsedJson();
    setcontext(&dispatch_context);
}

void ParsedJson()
{
    struct json_object *parsed_json;//parse all file into object and get Threads
    //object to save each thread
    struct json_object *thread;
    struct json_object *name;
    struct json_object *entry;
    struct json_object *priority;
    struct json_object *cancel;
    size_t n_thread;//size of Threads
    size_t i;
    int p;//priority, in integer
    int q;

    parsed_json = json_object_from_file("init_threads.json");
    json_object_object_get_ex(parsed_json, "Threads", &parsed_json);

    n_thread = json_object_array_length(parsed_json);

    //create thread one by one
    for(i=0; i<n_thread; i++)
    {
        thread = json_object_array_get_idx(parsed_json, i);
        json_object_object_get_ex(thread, "name", &name);
        json_object_object_get_ex(thread, "entry function", &entry);
        json_object_object_get_ex(thread, "priority", &priority);
        json_object_object_get_ex(thread, "cancel mode", &cancel);

        //change priority into integer
        switch(json_object_get_string(priority)[0])
        {
        case 'H':
            p = 2;
            break;
        case 'M':
            p = 1;
            break;
        default:
            p = 0;
            break;
        }
        //Create thread structure and enqueue
        q = OS2021_ThreadCreate(json_object_get_string(name), json_object_get_string(entry), p, json_object_get_int(cancel));
        if(q == -1)
            printf("Incorrect entry function.\n");
    }

    return;
}
