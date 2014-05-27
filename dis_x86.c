/* dis_x86.c */

/*
 * disassembler... someday
 */

#define _GNU_SOURCE

#include "emma.h"
#include "parsefile.h"
#include "dis_x86.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

void dis_x86(emma_handle* H,section_t* section)
{
    uint64_t addr=section->offset;
    uint64_t maxaddr=section->offset+section->size;
    const char* mm=(*H)->memmap;

    while (addr<maxaddr) {
        char addrline[20+1],hexline[31+1],mnemline[81],cmntline[81],tmpline[81];
        hexline[0]=0;
        mnemline[0]=0;
        cmntline[0]=0;
        snprintf(addrline,20,"0x%08"PRIx64":",addr);
        unsigned int byte=*(mm+addr)&0xff;
        snprintf(hexline,4,"%02x",byte);
        snprintf(mnemline,80,".byte\t0x%02x",byte);
        if (isprint(byte)) {
            snprintf(cmntline,80,"\t# '%c'",byte);
        }
        addr++;
        snprintf(tmpline,31,"%-30s",hexline);
        strncpy(hexline,tmpline,31);
        if (strlen(addrline)>0) {
            printf("%s %s %s %s\n",addrline,hexline,mnemline,cmntline);
        }
    }
}
