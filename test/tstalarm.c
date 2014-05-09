#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void handler(int signum __attribute__((unused)))
{
    printf("Caught signal %d\n",signum);
}

struct sigaction new_action;

int main()
{
    unsigned long int i;
    char* ptr;

    new_action.sa_handler = handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags=0;

    printf("Sizeof new_action: %lu (%lx)\n",sizeof(new_action),(unsigned long)&new_action);
    printf("Sizeof sa_handler: %lu (%ld) %lx\n",sizeof(new_action.sa_handler),(long int)&new_action.sa_handler-(long int)&new_action,(unsigned long)new_action.sa_handler);
    printf("Sizeof sa_mask: %lu (%ld)\n",sizeof(new_action.sa_mask),(long int)&new_action.sa_mask-(long int)&new_action);
    ptr=(char*)((void*)&new_action.sa_mask);
    for (i=0; i<sizeof(new_action.sa_mask); ++i) {
        printf("%02x",*ptr);
        ++ptr;
        if ((i%32)==31) puts("");
    }
    printf("Sizeof sa_flags: %lu (%ld) %x\n",sizeof(new_action.sa_flags),(long int)&new_action.sa_flags-(long int)&new_action,new_action.sa_flags);

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

