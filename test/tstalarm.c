#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void handler(int signum __attribute__((unused)))
{
    printf("Caught signal %d\n",signum);
}

int main()
{
    struct sigaction new_action;

    new_action.sa_handler = handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags=0;

    puts("Setting up signal handler");

    sigaction(SIGALRM,&new_action,NULL);

    puts("Setting alarm");

    alarm(1);

    puts("Pausing...");

    pause();

    puts("We're back from neverland");

    puts("Trying another alarm...");

    alarm(1);

    puts("Pausing again");

    pause();

    puts("We're back again!");
    return 0;
}

