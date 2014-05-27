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
    printf("File Type: %s (%s %s)\n",
            estr(H->filetype),
            estr(H->bits),
            estr(H->endianness));
    printf("Base Addr:  0x%llx\n",H->baseaddress);
    printf("Start Addr: 0x%llx\n",H->startaddress);
    printf("Length: %'lld bytes\n",H->length);
    printf("\n");

    unsigned int segcount=emma_segment_count(&H);
    if (segcount>0) {
        printf("Segments (%d)\n",segcount);
        printf("%3s %8s %8s %8s %8s %8s %8s %8s %8s\n",
                "Seg",
                "type","flags","foffset","vaddr",
                "paddr","filesz","memsz","align");
        for (unsigned int j=0; j<segcount; ++j) {
            segment_t* segment=emma_get_segment(&H,j);
            printf("%2d)",j);
            printf(" %08x",segment->type);
            printf(" %08x",segment->flags);
            printf(" %8llx",segment->offset);
            printf(" %8llx",segment->vaddr);
            printf(" %8llx",segment->paddr);
            printf(" %8llx",segment->sizefile);
            printf(" %8llx",segment->sizemem);
            printf(" %8llx",segment->alignment);
            printf("\n");
        }
        printf("\n");
    }

    unsigned int seccount=emma_section_count(&H);
    if (seccount>0) {
        printf("Sections (%d)\n",seccount);
        for (unsigned int show=0; show<2; ++show) {
            for (unsigned int j=0; j<seccount; ++j) {
                section_t* section=emma_get_section(&H,j);
                printf("%3d) ",j);
                printf("0x%08llx ",section->addr);
                printf("%08llx-of ",section->offset);
                printf("%8llx-sz ",section->size);
                printf("%8x-lk ",section->link);
                printf("%08llx-fl [",section->flags);
                for (unsigned int k=0; k<8; ++k) {
                    if (k<section->size) {
                        printf("%02x",*(H->memmap+section->offset+k)&0xff);
                    } else {
                        printf("  ");
                    }
                }
                printf("]");
                if(section->name!=0) {
                    /* printf(" %s",(H->memmap+section->name)); */
                    printf(" %8llx",section->name);
                }
                printf("\n");
                if (show) {
                    dis_x86(&H,section);
                }
            }
            printf("\n");
        }
    }

    unsigned int symcount=emma_symbol_count(&H);
    if (symcount>0) {
        printf("Symbols (%d)\n",symcount);
        for (unsigned int j=0; j<symcount; ++j) {
            symbol_t* symbol=emma_get_symbol(&H,j);
            printf("0x%08llx ",symbol->value);
            printf("%08llx ",symbol->flags);
            printf("%08llx ",symbol->type);
            printf("%s\n",symbol->name);
        }
    }

    emma_close(&H);

    return 0;
}
