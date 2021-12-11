## Parser
1. parse json file O
2. create task context by ucontext O
3. enqueue task O


Multi-level feedback queue
just a policy for select next task


## Timer
timer tick for scheduling.
in timer interrupt(handler)
1. check time expiration O
2. set next timer O
3. reschedule (modify queue and pick next task) O
4. context switch O
- 第一次響了後，在程式運行的時間就會持續響
- sit_value是設定第一次響的時間，不能是0

## APIs
### OS2021_ThreadCreate
1. create task context by ucontext O
2. enqueue task O
3. reschedule (depends on homework requirement) O, no need(non-preemptive)

OS2021_ThreadWaitEvent
1. change task state O
2. dequeue from multi-level feedback queue O
3. reschedule O

OS2021_ThreadSetEvent
1. find the task which is waiting the event O
2. enqueue task O
3. reschedule (depends on homework requirement) O, no need

OS2021_ThreadWaitTime
1. change task state
2. dequeue from multi-level feedback queue
3. set timer
4. reschedule

OS2021_DeallocateThreadResource
1. delete context O

Dispatcher:
each time I need to put sth in run, use it.
wait or scheduler

scheduler: choose with priority and FIFO
when we need reschedule, call

Question:
1. reclaimer 基本上是不是跟其他thread行為一樣，只是他負責的工作是把terminate queue裡面的東西free掉
2. 我們按C+z可以印出東西，所以基本上就是設定signal(SIGTSTP, print_func())？
3. scheduler的作用是選出下一個要執行的thread，請問在這邊是不是希望我們用dispatcher代替？也就是說dispatcher的用途是在需要context switch的時候把ready queue的東西擺進去？像thread wait的時候把priority更改完後就呼叫dispatcher這樣
4. getcontext的用途我有看過了，我的理解是可以紀錄當下程式執行到哪一行了，所以我如果執行到一半時間到了，理論上是不是用getcontext紀錄起來，並存到自己的structure裡，下次要用的時候直接setcontext
5. 但這樣的話不太知道timer_context是做什麼用的？因為dispatch_context還可以明白是每次使用dispatcher都要呼叫，但timer到時會有SIGALRM