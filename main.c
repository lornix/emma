/* emma - main.c */

#include "emma.h"

#include <unistd.h>
// for exit
#include <stdio.h>
#include <locale.h>

#include "parsefile.h"
#include "dis_x86.h"

int main(int argc,const char* argv[])
{
    /* printf("Ver: " VERREV "\n"); */

    if (argc<2) {
        fprintf(stderr,"Please supply a filename to analyze\n");
        exit(1);
    }
    int arg=1;
    emma_handle H=emma_init();
    int err=emma_open(&H,argv[arg]);
    if (err!=0) {
        EXITERROR("Unable to open file: %s",argv[arg]);
    }
    setlocale(LC_ALL,"");
    printf("File: %s\n",argv[arg]);
    printf("File Type: %s\n",filetype_str(H->filetype));
    printf("Base Addr:  0x%lx\n",H->baseaddress);
    printf("Start Addr: 0x%lx\n",H->startaddress);
    printf("Length: %'ld bytes\n",H->length);
    if (emma_section_count(&H)>0) {
        printf("%d sections\n",emma_section_count(&H));
        for (unsigned int j=0; j<emma_section_count(&H); ++j) {
            section_t* section=emma_section(&H,j);
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
            dis_x86(&H,section);
        }
    }
    if (emma_symbol_count(&H)>0) {
        printf("%d symbols\n",emma_symbol_count(&H));
    }
    emma_close(&H);

    return 0;
}
