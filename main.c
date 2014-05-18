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
    EMMA_HANDLE handle;

    /* printf("Ver: " VERREV "\n"); */

    for (int i=1; i<argc; i++) {
        handle=emma_open(argv[i]);
        for (int i=0; i<emma_section_count(&handle); ++i) {
            EMMA_SECTION* section=emma_section(&handle,i);
            printf("%08lx %08lx %s\n",
                    section->vma_start,
                    section->length,
                    section->name);
        }
        emma_close(&handle);
    }

    return 0;
}
