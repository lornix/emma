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
            case   3:	SHOW("read"); break;
            case   5:	SHOW("open"); break;
            case   4:	SHOW("write"); break;
            case   6:	SHOW("close"); break;
            case  11:	SHOW("execve"); break;
            case  13:	SHOW("time"); break;
            case  19:	SHOW("seek"); break;
            case  27:	SHOW("alarm"); break;
            case  33:	SHOW("access"); break;
            case  45:	SHOW("brk"); break;
            case  54:	SHOW("ioctl"); break;
            case  91:	SHOW("munmap"); break;
            case 102:	SHOW("socketcall"); break;
            case 125:	SHOW("mprotect"); break;
            case 140:	SHOW("_llseek"); break;
            case 141:	SHOW("getdents"); break;
            case 174:	SHOW("rt_sigaction"); break;
            case 192:	SHOW("mmap2"); break;
            case 195:	SHOW("stat64"); break;
            case 197:	SHOW("fstat64"); break;
            case 221:	SHOW("fcntl64"); break;
            case 243:	SHOW("set_thread_area"); break;
            case 252:	SHOW("exit_group"); break;
            default:	printf("SysCall %3d: ?\n",syscall_nr); break;
        }

        /* Allow system call. */
        ptrace(PTRACE_SYSCALL, child, NULL, NULL);
        wait(&status);
        ptrace(PTRACE_SYSCALL, child, NULL, NULL);
    }

    exit(EXIT_SUCCESS);
}
