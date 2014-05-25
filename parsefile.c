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
        size_t length,
        unsigned int alignment,
        unsigned long flags,
        char* contents);
static int create_symbol(emma_handle* H,
        const char* name,
        unsigned long value,
        unsigned long flags,
        unsigned long symtype);

static char* demangle(const char* name)
{
    /* TODO: find easier-to-use demangler */
    return strdup(name);
}

char* estr(int value)
{
    switch(value) {
        case ENDIAN_LITTLE: return "LE";
        case ENDIAN_BIG:    return "BE";
        case BITS_32:       return "32b";
        case BITS_64:       return "64b";
        case FT_RAW:        return "Raw";
        case FT_RELOC:      return "Reloc";
        case FT_EXEC:       return "Exec";
        case FT_DYNLIB:     return "DynLib";
        case FT_CORE:       return "Core";
        default: return "Unk";
    }
}

uint16_t make_little_endian_word(emma_handle* H,uint16_t value)
{
    if ((*H)->endianness==ENDIAN_BIG) {
        value=(uint16_t)(
                ((value&0xFF00)>>8)|
                ((value&0x00FF)<<8));
    }
    return value;
}
uint32_t make_little_endian_dword(emma_handle* H,uint32_t value)
{
    if ((*H)->endianness==ENDIAN_BIG) {
        value=((value&0xFF000000)>>24)|
            ((value&0x00FF0000)>>8)|
            ((value&0x0000FF00)<<8)|
            ((value&0x000000FF)<<24);
    }
    return value;
}
uint64_t make_little_endian_quad(emma_handle* H,uint64_t value)
{
    if ((*H)->endianness==ENDIAN_BIG) {
        value=((value&0xFF00000000000000)>>56)|
            ((value&0x00FF000000000000)>>40)|
            ((value&0x0000FF0000000000)>>24)|
            ((value&0x000000FF00000000)>>8)|
            ((value&0x00000000FF000000)<<8)|
            ((value&0x0000000000FF0000)<<24)|
            ((value&0x000000000000FF00)<<40)|
            ((value&0x00000000000000FF)<<56);
    }
    return value;
}
static void parse_elf_header(emma_handle* H)
{
    Elf64_Ehdr elf;
    char* mm=(*H)->memmap;

    /* check ELF version */
    if (mm[EI_VERSION]!=EV_CURRENT) {
        /* TODO:perhaps a 'problem' entry? */
        fprintf(stderr,"WARN: Elf Header EI_VERSION!=EV_CURRENT (=0x%02x)\n",mm[EI_VERSION]);
    }

    /* check ABI in use, or zero value */
    if ((mm[EI_OSABI]!=ELFOSABI_GNU)&&(mm[EI_OSABI]!=0)) {
        /* TODO:perhaps a 'problem' entry? */
        fprintf(stderr,"WARN: Elf Header EI_OSABI!=ELFOSABI_GNU (=0x%02x)\n",mm[EI_OSABI]);
    }

    /* Check value of EI_ABIVERSION, should be zero */
    if (mm[EI_ABIVERSION]!=0) {
        /* TODO:perhaps a 'problem' entry? */
        fprintf(stderr,"INFO: Elf Header EI_ABIVERSION!=0x00 (=0x%02x)\n",mm[EI_ABIVERSION]);
    }

    /* remember the padding? It should be all zeros, but I'm curious */
    for (int i=EI_PAD; i<EI_NIDENT; ++i) {
        if (mm[i]!=0) {
            char tmpline1[129];
            char tmpline2[129];
            snprintf(tmpline1,128,"INFO: Elf Header EI_PAD Area Non-Zero: (hex) ");
            i=EI_PAD;
            while (i<EI_NIDENT) {
                snprintf(tmpline2,128,"%02x",mm[i++]);
                strncat(tmpline1,tmpline2,128);
                if ((i%4)==0) {
                    strncat(tmpline1," ",128);
                }
            }
            /* TODO:perhaps a 'problem' entry? */
            fprintf(stderr,"%s\n",tmpline1);
            break;
        }
    }

    /* endianness? */
    if (mm[EI_DATA]==ELFDATA2LSB) {
        (*H)->endianness=ENDIAN_LITTLE;
    } else if (mm[EI_DATA]==ELFDATA2MSB) {
        (*H)->endianness=ENDIAN_BIG;
    }

    /* 32 bit? 64 bit? unknown? */
    /* a default to catch issues */
    (*H)->bits=0;
    if (mm[EI_CLASS]==ELFCLASS32) {
        (*H)->bits=BITS_32;
    } else if (mm[EI_CLASS]==ELFCLASS64) {
        (*H)->bits=BITS_64;
    }

    assert(((*H)->bits==BITS_32)||((*H)->bits==BITS_64));

    if ((*H)->bits==BITS_32) {
        /* handle 32 bit header */
        Elf32_Ehdr* elf32=(Elf32_Ehdr*)mm;
        elf.e_type      = make_little_endian_word(H,elf32->e_type);
        elf.e_machine   = make_little_endian_word(H,elf32->e_machine);
        elf.e_version   = make_little_endian_dword(H,elf32->e_version);
        elf.e_entry     = make_little_endian_dword(H,elf32->e_entry);
        elf.e_phoff     = make_little_endian_dword(H,elf32->e_phoff);
        elf.e_shoff     = make_little_endian_dword(H,elf32->e_shoff);
        elf.e_flags     = make_little_endian_dword(H,elf32->e_flags);
        elf.e_ehsize    = make_little_endian_word(H,elf32->e_ehsize);
        elf.e_phentsize = make_little_endian_word(H,elf32->e_phentsize);
        elf.e_phnum     = make_little_endian_word(H,elf32->e_phnum);
        elf.e_shentsize = make_little_endian_word(H,elf32->e_shentsize);
        elf.e_shnum     = make_little_endian_word(H,elf32->e_shnum);
        elf.e_shstrndx  = make_little_endian_word(H,elf32->e_shstrndx);
    } else {
        /* handle 64 bit header */
        Elf64_Ehdr* elf64=(Elf64_Ehdr*)mm;
        elf.e_type      = make_little_endian_word(H,elf64->e_type);
        elf.e_machine   = make_little_endian_word(H,elf64->e_machine);
        elf.e_version   = make_little_endian_dword(H,elf64->e_version);
        elf.e_entry     = make_little_endian_quad(H,elf64->e_entry);
        elf.e_phoff     = make_little_endian_quad(H,elf64->e_phoff);
        elf.e_shoff     = make_little_endian_quad(H,elf64->e_shoff);
        elf.e_flags     = make_little_endian_dword(H,elf64->e_flags);
        elf.e_ehsize    = make_little_endian_word(H,elf64->e_ehsize);
        elf.e_phentsize = make_little_endian_word(H,elf64->e_phentsize);
        elf.e_phnum     = make_little_endian_word(H,elf64->e_phnum);
        elf.e_shentsize = make_little_endian_word(H,elf64->e_shentsize);
        elf.e_shnum     = make_little_endian_word(H,elf64->e_shnum);
        elf.e_shstrndx  = make_little_endian_word(H,elf64->e_shstrndx);
    }
    switch (elf.e_type) {
        case ET_REL:
            (*H)->filetype=FT_RELOC; break;
        case ET_EXEC:
            (*H)->filetype=FT_EXEC; break;
        case ET_DYN:
            (*H)->filetype=FT_DYNLIB; break;
        case ET_CORE:
            (*H)->filetype=FT_CORE; break;
        default:
            (*H)->filetype=FT_RAW;
    }
    switch (elf.e_machine) {
        /* very limited here, perhaps more later? */
        case EM_386: /* 32 bit i386 */
            (*H)->bits=BITS_32;
            (*H)->baseaddress=DEFAULT_BASE_32bits;
            break;
        case EM_X86_64: /* 64 bit x86_64 */
            (*H)->bits=BITS_64;
            (*H)->baseaddress=DEFAULT_BASE_64bits;
            break;
        default: /* uh oh */
            (*H)->bits=0;
            (*H)->baseaddress=0;
    }
    if (elf.e_version!=EV_CURRENT) {
        /* TODO:perhaps a 'problem' entry? */
        fprintf(stderr,"WARN: Elf Header e_version!=EV_CURRENT (=0x%02x)\n",elf.e_version);
    }
    (*H)->startaddress=elf.e_entry;
    (*H)->programheader=(char*)elf.e_phoff;
    (*H)->sectionheader=(char*)elf.e_shoff;
    (*H)->elfflags=elf.e_flags;
    (*H)->elfheadersize=elf.e_ehsize;
    (*H)->phentsize=elf.e_phentsize;
    (*H)->phnum=elf.e_phnum;
    (*H)->shentsize=elf.e_shentsize;
    (*H)->shnum=elf.e_shnum;
    (*H)->strindex=elf.e_shstrndx;
}
static void parse_program_header(emma_handle* H)
{
    char* mm=(*H)->memmap;
}
static void parse_section_header(emma_handle* H)
{
    /*FIXME: shut up unused warning */ (*H)->bits=(*H)->bits;
}

emma_handle emma_init()
{
    emma_handle H=malloc(sizeof(struct_t));
    if (H==0) {
        /* malloc failed?!? */
        return 0;
    }

    /* zero out structure by default */
    memset(H,0,sizeof(struct_t));

    return H;
}
int emma_open(emma_handle* H,const char* fname)
{
    if ((H==0)||(*H==0)) {
        /* don't deref 0 */
        return 1;
    }

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
    (*H)->length=(size_t)stats.st_size;

    char* mm=mmap(0x0,(*H)->length,PROT_READ,MAP_SHARED,(*H)->fd,0);
    if (mm==MAP_FAILED) {
        close((*H)->fd);
        return 1;
    }

    (*H)->memmap=mm;

    if (memcmp(mm,ELFMAG,SELFMAG)!=0) {
        /* not an ELF file of any sort */
        /* we need to create a raw section to hold contents*/
        (*H)->filetype=FT_RAW;
        /* TODO: option to specify endianness */
        (*H)->endianness=ENDIAN_LITTLE;
        /* TODO: option to specify bit size */
        (*H)->bits=BITS_64;
        /* TODO: option to specify base address */
        create_section(H,".raw_data",0x0,(*H)->length,0,0x0,mm);
        /* TODO: option to specify start address */
        create_symbol(H, "_start",0x0,0x0,0x0);
        return 0;
    }

    parse_elf_header(H);
    parse_program_header(H);
    parse_section_header(H);

    return 0;
}
unsigned int emma_section_count(emma_handle* H)
{
    if ((H==0)||(*H==0)) {
        /* don't deref 0 */
        return 0;
    }
    return (*H)->section_count;
}
section_t* emma_section(emma_handle* H,unsigned int which)
{
    if ((H==0)||(*H==0)) {
        /* don't deref 0 */
        return 0;
    }
    if (which>=(*H)->section_count) {
        return 0;
    }
    return (*H)->sections[which];
}
unsigned int emma_symbol_count(emma_handle* H)
{
    if ((H==0)||(*H==0)) {
        /* don't deref 0 */
        return 0;
    }

    return (*H)->symbol_count;
}
symbol_t* emma_symbol(emma_handle* H,unsigned int which)
{
    if ((H==0)||(*H==0)) {
        /* don't deref 0 */
        return 0;
    }
    if (which>=(*H)->symbol_count) {
        return 0;
    }
    return (*H)->symbols[which];
}
int emma_close(emma_handle* H)
{
    if ((H==0)||(*H==0)) {
        /* don't deref 0 */
        return 1;
    }

    /* release mmapping */
    munmap((*H)->memmap,(*H)->length);

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
        size_t length,
        unsigned int alignment,
        unsigned long flags,
        char* contents)
{
    if ((H==0)||(*H==0)) {
        /* don't deref 0 */
        return 1;
    }
    section_t* savesection=malloc(sizeof(section_t));
    if (savesection==0) {
        /* malloc failed?!? */
        return 1;
    }

    savesection->name=name;
    savesection->vma_start=vma_start;
    savesection->length=length;
    savesection->alignment=alignment;
    savesection->flags=flags;
    savesection->contents=contents;

    (*H)->sections=realloc((*H)->sections,sizeof(section_t*)*((*H)->section_count+1));
    if ((*H)->sections==0) {
        /* realloc failed?!? */
        return 1;
    }
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
    if ((H==0)||(*H==0)) {
        /* don't deref 0 */
        return 1;
    }
    symbol_t* savesymbol=malloc(sizeof(symbol_t));
    if (savesymbol==0) {
        /* malloc failed?!? */
        return 1;
    }

    savesymbol->name=demangle(name);
    savesymbol->value=value;
    savesymbol->flags=flags;
    savesymbol->type=symtype;;

    (*H)->symbols=realloc((*H)->symbols,sizeof(symbol_t*)*((*H)->symbol_count+1));
    if ((*H)->symbols==0) {
        /* realloc failed?!? */
        return 1;
    }
    (*H)->symbols[(*H)->symbol_count]=savesymbol;
    (*H)->symbol_count++;
    return 0;
}
