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
            printf("%8lx ",section->vma_start);
            printf("%8lx ",section->length);
            printf("%2d ",1<<(section->alignment));
            printf("%08lx [",section->flags);
            for (unsigned int k=0; k<8; ++k) {
                if (k<section->length) {
                    printf("%02x",*(section->contents+k)&0xff);
                } else {
                    printf("  ");
                }
            }
            printf("] %s\n",section->name);
        }
        printf("%d symbols\n",emma_symbol_count(&handle));
        for (unsigned int j=0; j<emma_symbol_count(&handle); ++j) {
            EMMA_SYMBOL* symbol=emma_symbol(&handle,j);
            printf("%8lx ",symbol->value);
            printf("%08lx ",symbol->flags);
            printf("%c ",(char)(symbol->type&0xff));
            printf("%s",symbol->name);
            printf("\n");
        }
        emma_close(&handle);
    }

    return 0;
}
