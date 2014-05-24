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

void dis_x86(emma_handle* H,section_t* section)
{
    unsigned long addr=0;
    unsigned long offset=section->vma_start;
    unsigned long maxaddr=section->length;
    char* content=section->contents;
    /* unsigned int bits=(*H)->mach; */

    while (addr<maxaddr) {
        char addrline[15+1],hexline[31+1],mnemline[81],cmntline[81],tmpline[81];
        int instrlen=15;
        int prefix66=0;
        hexline[0]=0;
        mnemline[0]=0;
        cmntline[0]=0;
        snprintf(addrline,15,"%08lx:",addr+offset);
        while (addr<maxaddr) {
            unsigned long tmp;
            unsigned int byte=*(content+addr)&0xff;
            snprintf(tmpline,4,"%02x",byte);
            strncat(hexline,tmpline,31);
            switch (byte) {
                case 0x0f: /* special stuff */
                    byte=*(content+addr+1)&0xff;
                    switch (byte) {
                        case 0x05: /* syscall */
                            snprintf(tmpline,4,"%02x",byte);
                            strncat(hexline,tmpline,31);
                            snprintf(mnemline,80,"syscall");
                            addr+=2;
                            instrlen=0;
                            break;
                        default:
                            snprintf(mnemline,80,".byte\t0x0f");
                            addr++;
                            instrlen=0;
                    }
                    break;
                case 0x66: /* size-prefix byte */
                    prefix66=1;
                    addr++;
                    break;
                case 0xb0:
                    addr++;
                    byte=*(content+addr)&0xff;
                    snprintf(tmpline,4,"%02x",byte);
                    strncat(hexline,tmpline,31);
                    snprintf(mnemline,80,"mov\t$0x%0x,%%%s",byte,prefix66?"ax":"al");
                    addr++;
                    instrlen=0;
                    break;
                case 0xb2:
                    addr++;
                    byte=*(content+addr)&0xff;
                    snprintf(tmpline,4,"%02x",byte);
                    strncat(hexline,tmpline,31);
                    snprintf(mnemline,80,"mov\t$0x%x,%%%s",byte,prefix66?"dx":"dl");
                    addr++;
                    instrlen=0;
                    break;
                case 0xb8: /* mov $11223344,%eax */
                    addr++;
                    byte=*(content+addr)&0xff;
                    snprintf(tmpline,4,"%02x",byte);
                    strncat(hexline,tmpline,31);
                    tmp=byte;
                    addr++;
                    byte=*(content+addr)&0xff;
                    snprintf(tmpline,4,"%02x",byte);
                    strncat(hexline,tmpline,31);
                    tmp=(byte<<8)|tmp;
                    if (!prefix66) {
                        addr++;
                        byte=*(content+addr)&0xff;
                        snprintf(tmpline,4,"%02x",byte);
                        strncat(hexline,tmpline,31);
                        tmp=(byte<<16)|tmp;
                        addr++;
                        byte=*(content+addr)&0xff;
                        snprintf(tmpline,4,"%02x",byte);
                        strncat(hexline,tmpline,31);
                        tmp=(byte<<24)|tmp;
                    }
                    snprintf(mnemline,80,"mov\t$0x%lx,%%%s",tmp,prefix66?"ax":"eax");
                    addr++;
                    instrlen=0;
                    break;
                case 0xb9: /* mov $11223344,%ecx */
                    addr++;
                    byte=*(content+addr)&0xff;
                    snprintf(tmpline,4,"%02x",byte);
                    strncat(hexline,tmpline,31);
                    tmp=byte;
                    addr++;
                    byte=*(content+addr)&0xff;
                    snprintf(tmpline,4,"%02x",byte);
                    strncat(hexline,tmpline,31);
                    tmp=(byte<<8)|tmp;
                    if (!prefix66) {
                        addr++;
                        byte=*(content+addr)&0xff;
                        snprintf(tmpline,4,"%02x",byte);
                        strncat(hexline,tmpline,31);
                        tmp=(byte<<16)|tmp;
                        addr++;
                        byte=*(content+addr)&0xff;
                        snprintf(tmpline,4,"%02x",byte);
                        strncat(hexline,tmpline,31);
                        tmp=(byte<<24)|tmp;
                    }
                    snprintf(mnemline,80,"mov\t$0x%lx,%%%s",tmp,prefix66?"cx":"ecx");
                    addr++;
                    instrlen=0;
                    break;
                case 0xba: /* mov $11223344,%edx */
                    addr++;
                    byte=*(content+addr)&0xff;
                    snprintf(tmpline,4,"%02x",byte);
                    strncat(hexline,tmpline,31);
                    tmp=byte;
                    addr++;
                    byte=*(content+addr)&0xff;
                    snprintf(tmpline,4,"%02x",byte);
                    strncat(hexline,tmpline,31);
                    tmp=(byte<<8)|tmp;
                    if (!prefix66) {
                        addr++;
                        byte=*(content+addr)&0xff;
                        snprintf(tmpline,4,"%02x",byte);
                        strncat(hexline,tmpline,31);
                        tmp=(byte<<16)|tmp;
                        addr++;
                        byte=*(content+addr)&0xff;
                        snprintf(tmpline,4,"%02x",byte);
                        strncat(hexline,tmpline,31);
                        tmp=(byte<<24)|tmp;
                    }
                    snprintf(mnemline,80,"mov\t$0x%lx,%%%s",tmp,prefix66?"dx":"edx");
                    addr++;
                    instrlen=0;
                    break;
                case 0xbb: /* mov $11223344,%ebx */
                    addr++;
                    byte=*(content+addr)&0xff;
                    snprintf(tmpline,4,"%02x",byte);
                    strncat(hexline,tmpline,31);
                    tmp=byte;
                    addr++;
                    byte=*(content+addr)&0xff;
                    snprintf(tmpline,4,"%02x",byte);
                    strncat(hexline,tmpline,31);
                    tmp=(byte<<8)|tmp;
                    if (!prefix66) {
                        addr++;
                        byte=*(content+addr)&0xff;
                        snprintf(tmpline,4,"%02x",byte);
                        strncat(hexline,tmpline,31);
                        tmp=(byte<<16)|tmp;
                        addr++;
                        byte=*(content+addr)&0xff;
                        snprintf(tmpline,4,"%02x",byte);
                        strncat(hexline,tmpline,31);
                        tmp=(byte<<24)|tmp;
                    }
                    snprintf(mnemline,80,"mov\t$0x%lx,%%%s",tmp,prefix66?"bx":"ebx");
                    addr++;
                    instrlen=0;
                    break;
                case 0xbe: /* mov $11223344,%esi */
                    addr++;
                    byte=*(content+addr)&0xff;
                    snprintf(tmpline,4,"%02x",byte);
                    strncat(hexline,tmpline,31);
                    tmp=byte;
                    addr++;
                    byte=*(content+addr)&0xff;
                    snprintf(tmpline,4,"%02x",byte);
                    strncat(hexline,tmpline,31);
                    tmp=(byte<<8)|tmp;
                    if (!prefix66) {
                        addr++;
                        byte=*(content+addr)&0xff;
                        snprintf(tmpline,4,"%02x",byte);
                        strncat(hexline,tmpline,31);
                        tmp=(byte<<16)|tmp;
                        addr++;
                        byte=*(content+addr)&0xff;
                        snprintf(tmpline,4,"%02x",byte);
                        strncat(hexline,tmpline,31);
                        tmp=(byte<<24)|tmp;
                    }
                    snprintf(mnemline,80,"mov\t$0x%lx,%%%s",tmp,prefix66?"si":"esi");
                    addr++;
                    instrlen=0;
                    break;
                case 0xbf: /* mov $11223344,%edi */
                    addr++;
                    byte=*(content+addr)&0xff;
                    snprintf(tmpline,4,"%02x",byte);
                    strncat(hexline,tmpline,31);
                    tmp=byte;
                    addr++;
                    byte=*(content+addr)&0xff;
                    snprintf(tmpline,4,"%02x",byte);
                    strncat(hexline,tmpline,31);
                    tmp=(byte<<8)|tmp;
                    if (!prefix66) {
                        addr++;
                        byte=*(content+addr)&0xff;
                        snprintf(tmpline,4,"%02x",byte);
                        strncat(hexline,tmpline,31);
                        tmp=(byte<<16)|tmp;
                        addr++;
                        byte=*(content+addr)&0xff;
                        snprintf(tmpline,4,"%02x",byte);
                        strncat(hexline,tmpline,31);
                        tmp=(byte<<24)|tmp;
                    }
                    snprintf(mnemline,80,"mov\t$0x%lx,%%%s",tmp,prefix66?"di":"edi");
                    addr++;
                    instrlen=0;
                    break;
                case 0xcd: /* int xx */
                    addr++;
                    byte=*(content+addr)&0xff;
                    snprintf(tmpline,4,"%02x",byte);
                    strncat(hexline,tmpline,31);
                    snprintf(mnemline,80,"int\t$0x%02x",byte);
                    addr++;
                    instrlen=0;
                    break;
                case 0xf4: /* hlt */
                    snprintf(mnemline,80,"hlt");
                    addr++;
                    instrlen=0;
                    break;
                default:
                    snprintf(mnemline,80,".byte\t0x%02x",byte);
                    addr++;
                    instrlen=0;
            }
            instrlen--;
            if (instrlen<1) {
                break;
            }
        }
        snprintf(tmpline,31,"%-30s",hexline);
        strncpy(hexline,tmpline,31);
        if (strlen(addrline)>0) {
            printf("%s %s %s %s\n",addrline,hexline,mnemline,cmntline);
        }
    }
}
