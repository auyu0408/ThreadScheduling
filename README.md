# OS2021_Hw3_Template

* [requirements](https://docs.google.com/presentation/d/1UFuPUwd17Hogh5Vp8GZbnrLRAddGvC1j/edit#slide=id.p3)

## Directories

- function_library.h, function_library.c -> Thread function
- os_2021_thread_api.h, os_2021_thread_api.c -> Thread application interface function
- self_thread.h, self_thread.c -> Self define data structure and related function
- simulator.c -> main source
- init_threads.json -> Thread configuration file

## Building the Project

- install json-c library

```bash
sudo apt-get install libjson-c-dev
```

- Git Hook install

```
make
```

- compile simulator

```bash
make simulator
```

- delete all output expect `simulator`

```bash
make clean
```

## Implementations

### Structure In My Thread

- thread name
- thread id
- original priority
- current priority
- cancel mode
  - 0: asynchronous
  - 1: deffer
- cancel status
  - if thread's cancel mode is 1, when cancel status = 1, thread should cancel at cancel point
  - other's cancel status = 0
- wait
  - -1: not waiting any event
  - 0~7: waiting some event
- wait time
  - 0: no need to wait
  - \>0: if thread need to wait for a while
- already wait
  - if thread need to wait for a while, this will record how long you have wait
- queueing time
- waiting time
- thread context
- next thread
  - pointer pointed to next thread


### Multilevel Feedback Queue

- The two thing we considerate in our queue is: 1.Thread's priority 2. Enter order  
  - If their priority is different, high priority serve first
  - Else, first come first serve

- If the input order is: 1.L 2.H 3.M 4.M 5.H 6.L, the thread in queue be like:   
Head -> 2 -> 5 -> 3 -> 4 -> 1 -> 6  

## References

- [ucontext](https://pubs.opengroup.org/onlinepubs/7908799/xsh/ucontext.h.html)

- [signal handler](https://calvinkam.github.io/csci3150-Fall17-lab-pipes-signal/custom-signal-handler.html)

- [timer](https://www.ibm.com/docs/en/i/7.3?topic=ssw_ibm_i_73/apis/setitime.htm)
