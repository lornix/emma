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
        EMMA_HANDLE handle=emma_init();
        int err=emma_open(&handle,argv[i]);
        if (err!=0) {
            EXITERROR("Error opening file: %s",argv[i]);
        }
        printf("%d sections\n",emma_section_count(&handle));
        for (unsigned int j=0; j<emma_section_count(&handle); ++j) {
            EMMA_SECTION* section=emma_section(&handle,j);
            printf("%08lx %08lx %s\n",
                    section->vma_start,
                    section->length,
                    section->name);
        }
        printf("%d symbols\n",emma_symbol_count(&handle));
        for (unsigned int j=0; j<emma_symbol_count(&handle); ++j) {
            EMMA_SYMBOL* symbol=emma_symbol(&handle,j);
            printf("%08lx %s\n",
                    symbol->value,
                    symbol->name);
        }
        emma_close(&handle);
    }

    return 0;
}
