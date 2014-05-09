#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/reg.h>

#if 0
#define SHOW(x)
#else
#define SHOW(x) printf("SysCall %3d: %s\n",syscall_nr,x)
#endif

int main()
{
    pid_t child;
    int status, syscall_nr;

    child = fork();
    if (child == 0) {
        /* In child. */
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl("/usr/bin/w", "", NULL);
        // not reached
    }

    /* In parent. */
    while (1) {
        wait(&status);

        /* Abort loop if child has exited. */
        if (WIFEXITED(status) || WIFSIGNALED(status))
            break;

        /* Obtain syscall number from the child's process context. */
        syscall_nr = ptrace(PTRACE_PEEKUSER, child, 8 * ORIG_RAX, NULL);
        /* printf("SysCall %3d: ", syscall_nr); */
        switch (syscall_nr)
        {
            case   0:	SHOW("read"); break;
            case   1:	SHOW("write"); break;
            case   2:	SHOW("open"); break;
            case   3:	SHOW("close"); break;
            case   4:	SHOW("stat"); break;
            case   5:	SHOW("fstat"); break;
            case   8:	SHOW("lseek"); break;
            case   9:	SHOW("mmap"); break;
            case  10:	SHOW("mprotect"); break;
            case  11:	SHOW("munmap"); break;
            case  12:	SHOW("brk"); break;
            case  13:	SHOW("rt_sigaction"); break;
            case  16:	SHOW("ioctl"); break;
            case  21:	SHOW("access"); break;
            case  37:	SHOW("alarm"); break;
            case  41:	SHOW("socket"); break;
            case  42:	SHOW("connect"); break;
            case  59:	SHOW("execve"); break;
            case  60:	SHOW("exit"); break;
            case  63:	SHOW("uname"); break;
            case  72:	SHOW("fcntl"); break;
            case  78:	SHOW("getdents"); break;
            case 158:	SHOW("arch_prctl"); break;
            case 257:	SHOW("openat"); break;
            default:	printf("SysCall %3d: ?\n",syscall_nr); break;
        }

        /* Allow system call. */
        ptrace(PTRACE_SYSCALL, child, NULL, NULL);
        wait(&status);
        ptrace(PTRACE_SYSCALL, child, NULL, NULL);
    }

    exit(EXIT_SUCCESS);
}
