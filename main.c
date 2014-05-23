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

    if (argc<2) {
        fprintf(stderr,"Please supply a filename to analyze\n");
        exit(1);
    }
    int arg=1;
    emma_handle handle=emma_init();
    int err=emma_open(&handle,argv[arg]);
    if (err!=0) {
        EXITERROR("Unable to open file: %s",argv[arg]);
    }
    printf("File: %s\n",argv[arg]);
    if (emma_section_count(&handle)>0) {
        printf("%d sections\n",emma_section_count(&handle));
        for (unsigned int j=0; j<emma_section_count(&handle); ++j) {
            emma_section_t* section=emma_section(&handle,j);
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
    }
    if (emma_symbol_count(&handle)>0) {
        printf("%d symbols\n",emma_symbol_count(&handle));
        for (unsigned int j=0; j<emma_symbol_count(&handle); ++j) {
            emma_symbol_t* symbol=emma_symbol(&handle,j);
            printf("%8lx ",symbol->value&0xffffffff);
            printf("%08lx ",symbol->flags);
            printf("%c ",(char)(symbol->type&0xff));
            printf("%s",symbol->name);
            printf("\n");
        }
    }
    emma_close(&handle);

    return 0;
}
