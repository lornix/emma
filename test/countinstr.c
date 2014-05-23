#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <locale.h>

int main(int argc,char* argv[])
{
    long long counter = 0;  /*  machine instruction counter */
    int wait_val;           /*  child's return value        */
    int pid;                /*  child's process id          */
    char *arg0;             /* argv[0] pointer              */

    if (argc<2) {
        arg0=basename(strdup(argv[0]));
        fprintf(stderr,"%s: Usage:\n\t%s prog [args] ...\n",arg0,arg0);
        return 1;
    }

    switch (pid = fork()) {
        case -1:
            perror("fork");
            break;
        case 0: /*  child process starts        */
            ptrace(PTRACE_TRACEME, 0, 0, 0);
            /*
             *  must be called in order to allow the
             *  control over the child process
             */
            execl(argv[1],argv[1],*(argv+2),NULL);
            /*
             *  executes the program and causes
             *  the child to stop and send a signal
             *  to the parent, the parent can now
             *  switch to PTRACE_SINGLESTEP
             */
            break;
            /*  child process ends  */
        default:/*  parent process starts       */
            wait(&wait_val);
            /*
             *   parent waits for child to stop at next
             *   instruction (execl())
             */
            while (wait_val == W_STOPCODE(SIGTRAP)) {
                counter++;
                if (ptrace(PTRACE_SINGLESTEP, pid, 0, 0) != 0)
                    perror("ptrace");
                /*
                 *   switch to singlestep tracing and
                 *   release child
                 *   if unable call error.
                 */
                wait(&wait_val);
                /*   wait for next instruction to complete  */
            }
            /*
             * continue to stop, wait and release until
             * the child is finished; wait_val != 0x57f (1407)
             * Low=0177 and High=05 (SIGTRAP)
             */
    }
    setlocale(LC_ALL,"");
    fprintf(stderr,"Number of machine instructions : %'lld\n", counter);
    return 0;
}
