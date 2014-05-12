/* emma - main.c */

#include <unistd.h>
// for exit
#include <stdio.h>

// #include <elf.h>
// #include <bfd.h>
// #include <dis-asm.h>

#include "emma.h"
#include "parsefile.h"

int main(int argc,const char* argv[])
{
    /* printf("Ver: " VERREV "\n"); */

    for (int i=1; i<argc; i++) {
        parsefile(argv[i]);
    }

    return 0;
}
