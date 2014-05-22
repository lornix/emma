/* parsefile.c */

/*
 * file for the file parsing code, it populates arrays of sections and their
 * contents, symbols and their values, file information, start address, etc.
 */

#define _GNU_SOURCE

#include "emma.h"
#include "parsefile.h"

#include <stdio.h>
#include <assert.h>

static void elf_load_sections(EMMA_HANDLE* handle,bfd* abfd);
static void elf_load_symbols(EMMA_HANDLE* handle,bfd* abfd);

EMMA_HANDLE emma_init()
{
    EMMA_HANDLE handle=malloc(sizeof(EMMA_STRUCT));

    assert(handle!=0);

    bfd_init();
    bfd_set_default_target("default");
    /* zero out structure by default */
    memset(handle,0,sizeof(EMMA_STRUCT));
    /* a few things need default values */
    handle->whichendian=little_endian;

    return handle;
}
int emma_open(EMMA_HANDLE* handle,const char* fname)
{
    assert(*handle!=0);

    (*handle)->filename=strdup(fname);
    (*handle)->abfd=bfd_openr(fname,0);
    if (!(*handle)->abfd) {
        EXITERROR("Unable to open file: %s",fname);
    }
    /* make sure file is a known object type, maybe add code
       to parse bfd_archive and bfd_core, along with raw data */
    if (!bfd_check_format((*handle)->abfd,bfd_object)) {
        bfd_close((*handle)->abfd);
        EXITERROR("Unknown/Not Handled File Type: %s",fname);
    }

    (*handle)->arch=bfd_get_arch((*handle)->abfd);
    (*handle)->mach=bfd_get_mach((*handle)->abfd);
    (*handle)->machstr=bfd_printable_arch_mach((*handle)->arch,(*handle)->mach);
    (*handle)->fileflags=bfd_get_file_flags((*handle)->abfd);
    switch ((*handle)->arch) {
        case bfd_arch_i386:
            (*handle)->flag_has_reloc=(*handle)->fileflags&HAS_RELOC;
            (*handle)->flag_has_exec=(*handle)->fileflags&EXEC_P;
            (*handle)->flag_has_linenums=(*handle)->fileflags&HAS_LINENO;
            (*handle)->flag_has_debug=(*handle)->fileflags&HAS_DEBUG;
            (*handle)->flag_has_symbols=(*handle)->fileflags&HAS_SYMS;
            (*handle)->flag_has_locals=(*handle)->fileflags&HAS_LOCALS;
            (*handle)->flag_has_dynamic=(*handle)->fileflags&DYNAMIC;
            (*handle)->flag_is_relaxable=(*handle)->fileflags&BFD_IS_RELAXABLE;
            (*handle)->startaddress=(*handle)->abfd->start_address;
            elf_load_sections(handle,(*handle)->abfd);
            elf_load_symbols(handle,(*handle)->abfd);
            break;
        default:
            bfd_close((*handle)->abfd);
            EXITERROR("Unknown/Unhandled Architecture: %s",(*handle)->machstr);
    }

    (*handle)->filetypestr=bfd_get_target((*handle)->abfd);
    (*handle)->flavor=bfd_get_flavour((*handle)->abfd);

    return 0;
}
unsigned int emma_section_count(EMMA_HANDLE* handle)
{
    assert(*handle!=0);
    return (*handle)->sections_num;
}
EMMA_SECTION* emma_section(EMMA_HANDLE* handle,unsigned int which)
{
    assert(*handle!=0);
    if (which>=(*handle)->sections_num) {
        return NULL;
    }
    return (*handle)->sections[which];
}
unsigned int emma_symbol_count(EMMA_HANDLE* handle)
{
    assert(*handle!=0);

    return (*handle)->symbols_num;
}
EMMA_SYMBOL* emma_symbol(EMMA_HANDLE* handle,unsigned int which)
{
    assert(*handle!=0);

    if (which>=(*handle)->symbols_num) {
        return NULL;
    }
    return (*handle)->symbols[which];
}
int emma_close(EMMA_HANDLE* handle)
{
    assert(*handle!=0);

    /* all done, close file */
    bfd_close((*handle)->abfd);

    /* all done, clean up */
    if ((*handle)->sections_num>0) {
        /* empty the section array */
        for (unsigned int i=0; i<(*handle)->sections_num; ++i) {
            /* sigh, bfd used malloc to create chunk for contents */
            if ((*handle)->sections[i]->contents) {
                free((*handle)->sections[i]->contents);
            }
            free((*handle)->sections[i]);
        }
        free((*handle)->sections);
        (*handle)->sections_num=0;
    }
    if ((*handle)->symbols_num>0) {
        /* empty the symbols vector */
        for (unsigned int i=0; i<(*handle)->symbols_num; ++i) {
            free((*handle)->symbols[i]);
        }
        free((*handle)->symbols);
        (*handle)->symbols_num=0;
    }
    free((*handle)->filename);
    free(*handle);
    *handle=0;
    return 0;
}

static char* emma_demangle(bfd* abfd,const char* name)
{
    /* remember! bfd malloc's mem for demangling */
    char* retval=bfd_demangle(abfd,name,0);
    if (retval==0) {
        retval=(char*)name;
    }
    return retval;
}

static void elf_load_sections(EMMA_HANDLE* handle,bfd* abfd)
{
    /* I couldn't determine a clean method to use bfd_map_over... */
    /* load the sections, follow the linked list built by bfd */
    struct bfd_section* sec=abfd->sections;

    (*handle)->sections=0;
    (*handle)->sections_num=0;

    while (sec) {
        section_t* savesection=malloc(sizeof(section_t));

        savesection->name=sec->name;
        savesection->vma_start=sec->vma;
        savesection->length=sec->size;
        savesection->alignment=sec->alignment_power;
        /* TODO determine flags used */
        savesection->flags=sec->flags;
        savesection->contents=NULL;

        unsigned long int datasize=bfd_get_section_size(sec);
        if (datasize) {
            bfd_malloc_and_get_section(abfd,sec,&(savesection->contents));
        }

        (*handle)->sections=realloc((*handle)->sections,sizeof(section_t*)*((*handle)->sections_num+1));
        (*handle)->sections[(*handle)->sections_num]=savesection;
        (*handle)->sections_num++;

        sec=sec->next;
    }
}
static void elf_load_symbols(EMMA_HANDLE* handle,bfd* abfd)
{
    /* ask how big storage for symbols needs to be */
    long int datasize=bfd_get_symtab_upper_bound(abfd);
    /* negative return value indicates no symbols */

    (*handle)->symbols=0;
    (*handle)->symbols_num=0;

    if (datasize>0) {
        /* allocate memory to hold symbols */
        asymbol** symtable=(asymbol**)malloc(datasize);

        if (!symtable) {
            /* couldn't allocate memory? very odd */
            EXITERROR("Error allocating memory for symtable");
        }

        long numsymbols=bfd_canonicalize_symtab(abfd,symtable);
        if (numsymbols<0) {
            EXITERROR("Error while canonicalizing symbols");
        }

        /* process regular symbols */
        for (long i=0; i<numsymbols; i++) {
            symbol_t* sym=malloc(sizeof(symbol_t));

            sym->name=emma_demangle(abfd,symtable[i]->name);
            sym->value=symtable[i]->value;
            sym->flags=symtable[i]->flags;
            sym->type=bfd_decode_symclass(symtable[i]);

            (*handle)->symbols=realloc((*handle)->symbols,sizeof(symbol_t*)*((*handle)->symbols_num+1));
            (*handle)->symbols[(*handle)->symbols_num]=sym;
            (*handle)->symbols_num++;
        }
        free(symtable);
    }

    long int dyndatasize=bfd_get_dynamic_symtab_upper_bound(abfd);
    /* negative return value indicates no symbols */

    if (dyndatasize>0) {
        /* allocate memory to hold symbols */
        asymbol** dynsymtable=(asymbol**)malloc(dyndatasize);

        if (!dynsymtable) {
            /* couldn't allocate memory? very odd */
            EXITERROR("Error allocating memory for dynamic symtable");
        }

        long dynnumsymbols=bfd_canonicalize_dynamic_symtab(abfd,dynsymtable);
        if (dynnumsymbols<0) {
            EXITERROR("Error while canonicalizing dynamic symbols");
        }
        /* process dynamic symbols */
        for (long i=0; i<dynnumsymbols; i++) {
            symbol_t* dynsym=malloc(sizeof(symbol_t));

            dynsym->name=emma_demangle(abfd,dynsymtable[i]->name);
            dynsym->value=dynsymtable[i]->value;
            dynsym->flags=dynsymtable[i]->flags;
            dynsym->type=bfd_decode_symclass(dynsymtable[i]);

            (*handle)->symbols=realloc((*handle)->symbols,sizeof(symbol_t*)*((*handle)->symbols_num+1));
            (*handle)->symbols[(*handle)->symbols_num]=dynsym;
            (*handle)->symbols_num++;
        }
        free(dynsymtable);
    }
}
