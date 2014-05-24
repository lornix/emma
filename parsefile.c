/* parsefile.c */

/*
 * file for the file parsing code, it populates arrays of sections and their
 * contents, symbols and their values, file information, start address, etc.
 */

#include "emma.h"

#include "parsefile.h"

#include <elf.h>

#include <stdio.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

static int create_section(emma_handle* H,
        const char* name,
        unsigned long vma_start,
        unsigned long length,
        int alignment,
        unsigned long flags,
        char* contents);
static int create_symbol(emma_handle* H,
        const char* name,
        unsigned long value,
        unsigned long flags,
        unsigned long symtype);

char* filetype_str(filetype_t filetype)
{
    /* needs to be kept in sync with filetype_t enum */
    char* retval[]={"Unknown","Raw32","Raw64","Elf32","Elf64","Lib32","Lib64"};
    return retval[filetype];
}

static char* demangle(const char* name)
{
    /* TODO: find easier-to-use demangler */
    return strdup(name);
}

emma_handle emma_init()
{
    emma_handle H=malloc(sizeof(struct_t));

    assert(H!=0);

    /* zero out structure by default */
    memset(H,0,sizeof(struct_t));
    /* a few things need default values */
    H->whichendian=LITEND;
    H->baseaddress=0;
    H->startaddress=H->baseaddress;
    H->filetype=FILETYPE_UNKNOWN;

    return H;
}
int emma_open(emma_handle* H,const char* fname)
{
    assert(*H!=0);

    (*H)->filename=strdup(fname);

    (*H)->fd=open(fname,O_RDONLY);
    if ((*H)->fd==0) {
        /* issue opening file? */
        return 1;
    }

    struct stat stats;
    if (fstat((*H)->fd,&stats)<0) {
        /* issue obtaining stats? */
        close((*H)->fd);
        return 1;
    }

    /* might be more we need to copy over */
    (*H)->length=stats.st_size;

    char* mmapaddr=mmap(NULL,(*H)->length,PROT_READ,MAP_SHARED,(*H)->fd,0);
    if (mmapaddr==MAP_FAILED) {
        close((*H)->fd);
        return 1;
    }

    (*H)->mmap=mmapaddr;

    if (memcmp(mmapaddr,ELFMAG,SELFMAG)!=0) {
        /* not an ELF file of any sort */
        (*H)->filetype=FILETYPE_RAW64;
        /* it'll be an option later, for now.. done! */
        (*H)->whichendian=LITEND;
        /* we need to create a raw section to hold contents*/
        create_section(H,".raw_data",0x0,(*H)->length,0,0x0,mmapaddr);
        return 0;
    }

    /* 32 bit? 64 bit? unknown? */
    if (mmapaddr[EI_CLASS]==ELFCLASS32) {
        (*H)->filetype=FILETYPE_ELF32;
    } else if (mmapaddr[EI_CLASS]==ELFCLASS64) {
        (*H)->filetype=FILETYPE_ELF64;
    } else {
        (*H)->filetype=FILETYPE_UNKNOWN;
    }

    /* endianness? */
    if (mmapaddr[EI_DATA]==ELFDATA2LSB) {
        (*H)->whichendian=LITEND;
    } else if (mmapaddr[EI_DATA]==ELFDATA2MSB) {
        (*H)->whichendian=BIGEND;
    } else {
        /* as a fallback, just in case */
        (*H)->whichendian=LITEND;
    }

    /* check ELF version */
    if (mmapaddr[EI_VERSION]!=EV_CURRENT) {
        /* TODO:perhaps a 'problem' entry? */
        fprintf(stderr,"PROBLEM: EI_VERSION!=EV_CURRENT (%d)\n",mmapaddr[EI_VERSION]);
    }

    /* check ABI in use, or zero value */
    if ((mmapaddr[EI_OSABI]!=ELFOSABI_GNU)&&(mmapaddr[EI_OSABI]!=0)) {
        /* TODO:perhaps a 'problem' entry? */
        fprintf(stderr,"PROBLEM: EI_OSABI!=ELFOSABI_GNU (%d)\n",mmapaddr[EI_OSABI]);
    }

    Elf64_Ehdr* elfheader=(Elf64_Ehdr*)mmapaddr;

    return 0;
}
unsigned int emma_section_count(emma_handle* H)
{
    assert(*H!=0);
    return (*H)->section_count;
}
section_t* emma_section(emma_handle* H,unsigned int which)
{
    assert(*H!=0);
    if (which>=(*H)->section_count) {
        return NULL;
    }
    return (*H)->sections[which];
}
unsigned int emma_symbol_count(emma_handle* H)
{
    assert(*H!=0);

    return (*H)->symbol_count;
}
symbol_t* emma_symbol(emma_handle* H,unsigned int which)
{
    assert(*H!=0);

    if (which>=(*H)->symbol_count) {
        return NULL;
    }
    return (*H)->symbols[which];
}
int emma_close(emma_handle* H)
{
    assert(*H!=0);

    /* release mmapping */
    munmap((*H)->mmap,(*H)->length);

    /* all done, close file */
    close((*H)->fd);

    /* all done, clean up */
    if ((*H)->section_count>0) {
        /* empty the section array */
        for (unsigned int i=0; i<(*H)->section_count; ++i) {
            free((*H)->sections[i]);
        }
        free((*H)->sections);
        (*H)->section_count=0;
    }
    if ((*H)->symbol_count>0) {
        /* empty the symbols vector */
        for (unsigned int i=0; i<(*H)->symbol_count; ++i) {
            free((void*)(*H)->symbols[i]->name);
            free((*H)->symbols[i]);
        }
        free((*H)->symbols);
        (*H)->symbol_count=0;
    }
    free((*H)->filename);
    free(*H);
    *H=0;
    return 0;
}

static int create_section(emma_handle* H,
        const char* name,
        unsigned long vma_start,
        unsigned long length,
        int alignment,
        unsigned long flags,
        char* contents)
{
    section_t* savesection=malloc(sizeof(section_t));

    savesection->name=name;
    savesection->vma_start=vma_start;
    savesection->length=length;
    savesection->alignment=alignment;
    savesection->flags=flags;
    savesection->contents=contents;

    (*H)->sections=realloc((*H)->sections,sizeof(section_t*)*((*H)->section_count+1));
    (*H)->sections[(*H)->section_count]=savesection;
    (*H)->section_count++;
    return 0;
}

static int create_symbol(emma_handle* H,
        const char* name,
        unsigned long value,
        unsigned long flags,
        unsigned long symtype)
{
    symbol_t* savesymbol=malloc(sizeof(symbol_t));

    savesymbol->name=demangle(name);
    savesymbol->value=value;
    savesymbol->flags=flags;
    savesymbol->type=symtype;;

    (*H)->symbols=realloc((*H)->symbols,sizeof(symbol_t*)*((*H)->symbol_count+1));
    (*H)->symbols[(*H)->symbol_count]=savesymbol;
    (*H)->symbol_count++;
    return 0;
}
