#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void handler(int sig)
{
    printf("Hi %d\n", sig);
    return;
}
int main()
{
    signal(SIGTSTP, handler);
    while(1);
    return 0;
}