/* parsefile.c */

/*
 * file for the file parsing code, it populates arrays of sections and their
 * contents, symbols and their values, file information, start address, etc.
 */

#include "emma.h"

#include "parsefile.h"

#include <elf.h>

#include <stdio.h>
/* assert */
#include <assert.h>
/* mmap munmap */
#include <sys/mman.h>
#include <sys/types.h>
/* fstat */
#include <sys/stat.h>
/* close read */
#include <unistd.h>
/* open */
#include <fcntl.h>

static int create_symbol(emma_handle* H,
        char* name,
        uint64_t value,
        uint64_t flags,
        uint64_t type);
static int create_section(emma_handle* H,
        char* name,
        unsigned int type,
        uint64_t flags,
        uint64_t addr,
        uint64_t offset,
        uint64_t size,
        unsigned int link,
        unsigned int info,
        uint64_t align,
        uint64_t entsize);
static int create_segment(emma_handle* H,
        unsigned int type,
        unsigned int flags,
        uint64_t offset,
        uint64_t vaddr,
        uint64_t paddr,
        uint64_t sizefile,
        uint64_t sizemem,
        uint64_t alignment);

static char* demangle(const char* name)
{
    /* TODO: find easier-to-use demangler */
    return strdup(name);
}

const char* estr(int value)
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

inline uint16_t make_little_endian_word(emma_handle* H,uint16_t value)
{
    if ((*H)->endianness==ENDIAN_BIG) {
        value=(uint16_t)(
                ((value&0xFF00)>>8)|
                ((value&0x00FF)<<8));
    }
    return value;
}
inline uint32_t make_little_endian_dword(emma_handle* H,uint32_t value)
{
    if ((*H)->endianness==ENDIAN_BIG) {
        value=((value&0xFF000000)>>24)|
            ((value&0x00FF0000)>>8)|
            ((value&0x0000FF00)<<8)|
            ((value&0x000000FF)<<24);
    }
    return value;
}
inline uint64_t make_little_endian_quad(emma_handle* H,uint64_t value)
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
    const char* mm=(*H)->memmap;

    /* check ELF version */
    if (mm[EI_VERSION]!=EV_CURRENT) {
        /* TODO:perhaps a 'problem' entry? */
        fprintf(stderr,"WARN: Elf Header EI_VERSION (%d) != EV_CURRENT (%d)\n",
                mm[EI_VERSION],
                EV_CURRENT);
    }

    /* check ABI in use, or zero value */
    if ((mm[EI_OSABI]!=ELFOSABI_GNU)&&(mm[EI_OSABI]!=0)) {
        /* TODO:perhaps a 'problem' entry? */
        fprintf(stderr,"WARN: Elf Header EI_OSABI (%d) != ELFOSABI_GNU (%d)\n",
                mm[EI_OSABI],
                ELFOSABI_GNU);
    }

    /* Check value of EI_ABIVERSION, should be zero */
    if (mm[EI_ABIVERSION]!=0) {
        /* TODO:perhaps a 'problem' entry? */
        fprintf(stderr,"INFO: Elf Header EI_ABIVERSION (%d) != 0x00\n",
                mm[EI_ABIVERSION]);
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
    (*H)->elf_class=0;
    if (mm[EI_CLASS]==ELFCLASS32) {
        (*H)->elf_class=BITS_32;
    } else if (mm[EI_CLASS]==ELFCLASS64) {
        (*H)->elf_class=BITS_64;
    }

    assert(((*H)->elf_class==BITS_32)||((*H)->elf_class==BITS_64));

    /* my own storage unit */
    Elf64_Ehdr elf;

    if ((*H)->elf_class==BITS_32) {
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
    switch (elf.e_machine) {
        /* very limited here, perhaps more later? */
        case EM_386: /* 32 bit i386 */
            (*H)->bits=BITS_32;
            break;
        case EM_X86_64: /* 64 bit x86_64 */
            (*H)->bits=BITS_64;
            break;
        case EM_ARM: /* 32 bit ARM */
            (*H)->bits=BITS_32;
            break;
        default: /* uh oh */
            (*H)->bits=0;
    }
    /* elf bits match program bits? */
    /* a difference would cause havoc in make_little_endian_* routines */
    if ((*H)->bits!=(*H)->elf_class) {
        /* TODO:perhaps a 'problem' entry? */
        fprintf(stderr,"WARN: Elf Header elf_class (%s) != bits (%s)\n",
                estr((*H)->elf_class),
                estr((*H)->bits));
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
    if (elf.e_version!=EV_CURRENT) {
        /* TODO:perhaps a 'problem' entry? */
        fprintf(stderr,"WARN: Elf Header e_version (%d) != EV_CURRENT (%d)\n",
                elf.e_version,
                EV_CURRENT);
    }
    (*H)->startaddress=elf.e_entry;
    (*H)->programheader=elf.e_phoff;
    (*H)->sectionheader=elf.e_shoff;
    (*H)->elfflags=elf.e_flags;
    (*H)->elfheadersize=elf.e_ehsize;
    (*H)->phentsize=elf.e_phentsize;
    (*H)->phnum=elf.e_phnum;
    (*H)->shentsize=elf.e_shentsize;
    (*H)->shnum=elf.e_shnum;
    (*H)->section_string_index=elf.e_shstrndx;
}
static void parse_program_header(emma_handle* H)
{
    if ((*H)->programheader==0) {
        /* well, that was easy... no program header! */
        return;
    }

    if ((*H)->bits==BITS_32) {
        /* parse 32bit program header */
        for (unsigned int i=0; i<(*H)->phnum; ++i) {
            Elf32_Phdr* elf32=(void*)(*H)->memmap+(*H)->programheader+(i*(*H)->phentsize);
            create_segment(H,
                    make_little_endian_dword(H,elf32->p_type),
                    make_little_endian_dword(H,elf32->p_flags),
                    make_little_endian_dword(H,elf32->p_offset),
                    make_little_endian_dword(H,elf32->p_vaddr),
                    make_little_endian_dword(H,elf32->p_paddr),
                    make_little_endian_dword(H,elf32->p_filesz),
                    make_little_endian_dword(H,elf32->p_memsz),
                    make_little_endian_dword(H,elf32->p_align));
        }
    } else {
        /* parse 64bit program header */
        for (unsigned int i=0; i<(*H)->phnum; ++i) {
            Elf64_Phdr* elf64=(void*)(*H)->memmap+(*H)->programheader+(i*(*H)->phentsize);
            create_segment(H,
                    make_little_endian_dword(H,elf64->p_type),
                    make_little_endian_dword(H,elf64->p_flags),
                    make_little_endian_quad(H,elf64->p_offset),
                    make_little_endian_quad(H,elf64->p_vaddr),
                    make_little_endian_quad(H,elf64->p_paddr),
                    make_little_endian_quad(H,elf64->p_filesz),
                    make_little_endian_quad(H,elf64->p_memsz),
                    make_little_endian_quad(H,elf64->p_align));
        }
    }
    /* find segment containing entry point */
    unsigned int segcount=(*H)->segment_count;
    for (unsigned int i=0; i<segcount; ++i) {
        segment_t* segment=(*H)->segments[i];
        if ((segment->vaddr<=(*H)->startaddress)&&
                ((*H)->startaddress<(segment->vaddr+segment->sizemem))&&
                (segment->sizemem>0)) {
            (*H)->baseaddress=segment->vaddr;
            break;
        }
    }
}
static void parse_section_header(emma_handle* H)
{
    if ((*H)->sectionheader==0) {
        /* well, that was easy... no section header! */
        return;
    }

    if ((*H)->bits==BITS_32) {
        /* parse 32bit section header */
        for (unsigned int i=0; i<(*H)->shnum; ++i) {
            Elf32_Shdr* elf32=(void*)(*H)->memmap+(*H)->sectionheader+(i*(*H)->shentsize);
            create_section(H,
                    /* grrr, double cast, booo! */
                    (char*)(uint64_t)make_little_endian_dword(H,elf32->sh_name),
                    make_little_endian_dword(H,elf32->sh_type),
                    make_little_endian_dword(H,elf32->sh_flags),
                    make_little_endian_dword(H,elf32->sh_addr),
                    make_little_endian_dword(H,elf32->sh_offset),
                    make_little_endian_dword(H,elf32->sh_size),
                    make_little_endian_dword(H,elf32->sh_link),
                    make_little_endian_dword(H,elf32->sh_info),
                    make_little_endian_dword(H,elf32->sh_addralign),
                    make_little_endian_dword(H,elf32->sh_entsize));
        }
    } else {
        /* parse 64bit program header */
        for (unsigned int i=0; i<(*H)->shnum; ++i) {
            Elf64_Shdr* elf64=(void*)(*H)->memmap+(*H)->sectionheader+(i*(*H)->shentsize);
            create_section(H,
                    /* again! with the double casting! ick! */
                    (char*)(uint64_t)make_little_endian_dword(H,elf64->sh_name),
                    make_little_endian_dword(H,elf64->sh_type),
                    make_little_endian_quad(H,elf64->sh_flags),
                    make_little_endian_quad(H,elf64->sh_addr),
                    make_little_endian_quad(H,elf64->sh_offset),
                    make_little_endian_quad(H,elf64->sh_size),
                    make_little_endian_dword(H,elf64->sh_link),
                    make_little_endian_dword(H,elf64->sh_info),
                    make_little_endian_quad(H,elf64->sh_addralign),
                    make_little_endian_quad(H,elf64->sh_entsize));
        }
    }
    /* set up section name pointers */
    unsigned int strindex=(*H)->section_string_index;
    unsigned int seccount=(*H)->section_count;
    if ((strindex==0)||(strindex>=seccount)) {
        /* TODO:perhaps a 'problem' entry? */
        fprintf(stderr,"WARN: Program Header SHSTRINDEX not valid (%d of %d)\n",
                strindex,seccount);
        /* TODO: perhaps an option to search for proper segment? */
        /* it's USUALLY the FIRST type 3 section, with no flags set */
        for (unsigned int i=0; i<seccount; ++i) {
            section_t* section=(*H)->sections[i];
            if ((section->type==SHT_STRTAB)&&       /* it's a STRTAB (3) */
                    (section->flags==0)&&           /* with no flags set */
                    (*((*H)->memmap+section->offset)==0)&& /* and *0==0 */
                    (*((*H)->memmap+section->offset+1)=='.')) { /* and *1=='.' */
                /* TODO:perhaps a 'problem' entry? */
                strindex=i;
                (*H)->section_string_index=i;
                fprintf(stderr,"WARN: Program Header SHSTRINDEX fixed, set to %d\n",
                        strindex);
                break;
            }

        }
    }
    for (unsigned int i=0; i<seccount; ++i) {
        section_t* section=(*H)->sections[i];
        if ((strindex>0)&&(strindex<seccount)) {
            section->name+=(uint64_t)(*H)->memmap+(*H)->sections[strindex]->offset;
        } else {
            section->name=".bad_shstrtab_index";
        }
    }
}
static void parse_symbol_section(emma_handle* H)
{
}

emma_handle emma_init(void)
{
    emma_handle H=calloc(1,sizeof(struct_t));
    if (H==0) {
        /* calloc failed?!? */
        return 0;
    }

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

    /* might be more we need to copy over, but nothing stands out */
    (*H)->length=(size_t)stats.st_size;

    const char* mm;
#if _POSIX_MAPPED_FILES > 0
    /* yay! mmaping is available, easy-peasy! */
    mm=mmap(0x0,(size_t)((*H)->length),PROT_READ,MAP_SHARED,(*H)->fd,0);
    if (mm==MAP_FAILED) {
        /* mmap failed? */
        close((*H)->fd);
        return 1;
    }
#else
    /* uh oh, file mmaping isn't available... a'loading we shall go! */
    mm=calloc(1,(*H)->length);
    if (mm==0) {
        /* calloc failed? */
        close((*H)->fd);
        return 1;
    }
    ssize_t mmoffset=0;
    ssize_t err;
    do {
        /* read a chunk of file (hopefully all of it!) */
        err=read((*H)->fd,(void*)mm+mmoffset,(*H)->length);
        if (err<0) {
            /* read failed */
            close((*H)->fd);
            return 1;
        }
        /* bump pointer to load in proper place */
        mmoffset+=err;
    } while (err>0);

#endif

    /* we're done with the file */
    close((*H)->fd);

    (*H)->memmap=mm;

    if (memcmp(mm,ELFMAG,SELFMAG)!=0) {
        /* not an ELF file of any sort */
        /* at some point, we'll handle PE/COFF/etc */
        /* we need to create a raw section to hold contents*/
        (*H)->filetype=FT_RAW;
        /* TODO: option to specify endianness */
        (*H)->endianness=ENDIAN_LITTLE;
        /* TODO: option to specify bit size */
        (*H)->bits=BITS_64;
        (*H)->elf_class=BITS_64;
        /* TODO: option to specify base address */
        create_section(H,".section.raw",0,0,0x0,0x0,(*H)->length,0,0,1,0);
        /* TODO: option to specify start address */
        create_symbol(H, "_start.raw",0x0,0x0,0x0);
        return 0;
    }

    parse_elf_header(H);
    parse_program_header(H);
    parse_section_header(H);
    parse_symbol_section(H);

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
section_t* emma_get_section(emma_handle* H,unsigned int which)
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
unsigned int emma_segment_count(emma_handle* H)
{
    if ((H==0)||(*H==0)) {
        /* don't deref 0 */
        return 0;
    }
    return (*H)->segment_count;
}
segment_t* emma_get_segment(emma_handle* H,unsigned int which)
{
    if ((H==0)||(*H==0)) {
        /* don't deref 0 */
        return 0;
    }
    if (which>=(*H)->segment_count) {
        return 0;
    }
    return (*H)->segments[which];
}
unsigned int emma_symbol_count(emma_handle* H)
{
    if ((H==0)||(*H==0)) {
        /* don't deref 0 */
        return 0;
    }

    return (*H)->symbol_count;
}
symbol_t* emma_get_symbol(emma_handle* H,unsigned int which)
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
    munmap((void*)(*H)->memmap,(size_t)((*H)->length));

    /* all done, close file */
    close((*H)->fd);

    /* all done, clean up */
    if ((*H)->symbol_count>0) {
        /* empty the symbols vector */
        for (unsigned int i=0; i<(*H)->symbol_count; ++i) {
            free((void*)(*H)->symbols[i]->name);
            free((*H)->symbols[i]);
        }
        free((*H)->symbols);
        (*H)->symbol_count=0;
    }
    if ((*H)->section_count>0) {
        /* empty the section array */
        for (unsigned int i=0; i<(*H)->section_count; ++i) {
            free((*H)->sections[i]);
        }
        free((*H)->sections);
        (*H)->section_count=0;
    }
    if ((*H)->segment_count>0) {
        /* empty the segment array */
        for (unsigned int i=0; i<(*H)->segment_count; ++i) {
            free((*H)->segments[i]);
        }
        free((*H)->segments);
        (*H)->segment_count=0;
    }
    free((void*)(*H)->filename);
    free(*H);
    *H=0;
    return 0;
}
static int create_symbol(emma_handle* H,
        char* name,
        uint64_t value,
        uint64_t flags,
        uint64_t symtype)
{
    if ((H==0)||(*H==0)) {
        /* don't deref 0 */
        return 1;
    }
    symbol_t* savesymbol=calloc(1,sizeof(symbol_t));
    if (savesymbol==0) {
        /* calloc failed?!? */
        return 1;
    }

    savesymbol->name=demangle(name);
    savesymbol->value=value;
    savesymbol->flags=flags;
    savesymbol->type=symtype;

    (*H)->symbols=realloc((*H)->symbols,sizeof(symbol_t*)*((*H)->symbol_count+1));
    if ((*H)->symbols==0) {
        /* realloc failed?!? */
        return 1;
    }
    (*H)->symbols[(*H)->symbol_count]=savesymbol;
    (*H)->symbol_count++;
    return 0;
}
static int create_section(emma_handle* H,
        char* name,
        unsigned int type,
        uint64_t flags,
        uint64_t addr,
        uint64_t offset,
        uint64_t size,
        unsigned int link,
        unsigned int info,
        uint64_t align,
        uint64_t entsize)
{
    if ((H==0)||(*H==0)) {
        /* don't deref 0 */
        return 1;
    }
    section_t* savesection=calloc(1,sizeof(section_t));
    if (savesection==0) {
        /* calloc failed?!? */
        return 1;
    }

    savesection->name=name;
    savesection->type=type;
    savesection->flags=flags;
    savesection->addr=addr;
    savesection->offset=offset;
    savesection->size=size;
    savesection->link=link;
    savesection->info=info;
    savesection->align=align;
    savesection->entsize=entsize;

    (*H)->sections=realloc((*H)->sections,sizeof(section_t*)*((*H)->section_count+1));
    if ((*H)->sections==0) {
        /* realloc failed?!? */
        return 1;
    }
    (*H)->sections[(*H)->section_count]=savesection;
    (*H)->section_count++;
    return 0;
}
static int create_segment(emma_handle* H,
        unsigned int type,
        unsigned int flags,
        uint64_t offset,
        uint64_t vaddr,
        uint64_t paddr,
        uint64_t sizefile,
        uint64_t sizemem,
        uint64_t alignment)
{
    if ((H==0)||(*H==0)) {
        /* don't deref 0 */
        return 1;
    }
    segment_t* savesegment=calloc(1,sizeof(segment_t));
    if (savesegment==0) {
        /* calloc failed?!? */
        return 1;
    }

    savesegment->type=type;
    savesegment->flags=flags;
    savesegment->offset=offset;
    savesegment->vaddr=vaddr;
    savesegment->paddr=paddr;
    savesegment->sizefile=sizefile;
    savesegment->sizemem=sizemem;
    savesegment->alignment=alignment;

    (*H)->segments=realloc((*H)->segments,sizeof(segment_t*)*((*H)->segment_count+1));
    if ((*H)->segments==0) {
        /* realloc failed?!? */
        return 1;
    }
    (*H)->segments[(*H)->segment_count]=savesegment;
    (*H)->segment_count++;
    return 0;
}
