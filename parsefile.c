/* parsefile.c */

/*
 * file for the file parsing code, it populates arrays of sections and their
 * contents, symbols and their values, file information, start address,
 * whatever else I can obtain from the bfd backend for ELF/PE/a.out... type
 * files.  Eventually to have support for raw data blocks too
 */

#include "emma.h"
#include "parsefile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

EMMA_HANDLE emma_open(const char* fname)
{
    EMMA_HANDLE retval=malloc(sizeof(EMMA_STRUCT));
    return retval;
}
int emma_section_count(EMMA_HANDLE* handle)
{
    return 0;
}
EMMA_SECTION* emma_section(EMMA_HANDLE* handle,int which)
{
    return NULL;
}
void emma_close(EMMA_HANDLE* handle)
{
    if (*handle!=0) {
        free(*handle);
        *handle=0;
    }
}

void parsefile(const char* fname)
{
    parsefile_info_t pi;
    bfd* abfd;
    bfd_init();
    bfd_set_default_target("default");
    abfd=bfd_openr(fname,0);
    if (!abfd) {
        EXITERROR("Unable to open file: %s",fname);
    }
    /* make sure file is a known object type, maybe add code
       to parse bfd_archive and bfd_core, along with raw data */
    if (!bfd_check_format(abfd,bfd_object)) {
        bfd_close(abfd);
        EXITERROR("Unknown/Not Handled File Type: %s",fname);
    }

    pi.filename=fname;

    pi.arch=bfd_get_arch(abfd);
    pi.mach=bfd_get_mach(abfd);
    pi.machstr=bfd_printable_arch_mach(pi.arch,pi.mach);
    pi.fileflags=bfd_get_file_flags(abfd);
    switch (pi.arch) {
        case bfd_arch_i386:

            pi.flag_has_reloc=pi.fileflags&HAS_RELOC;
            pi.flag_has_exec=pi.fileflags&EXEC_P;
            pi.flag_has_linenums=pi.fileflags&HAS_LINENO;
            pi.flag_has_debug=pi.fileflags&HAS_DEBUG;
            pi.flag_has_symbols=pi.fileflags&HAS_SYMS;
            pi.flag_has_locals=pi.fileflags&HAS_LOCALS;
            pi.flag_has_dynamic=pi.fileflags&DYNAMIC;
            pi.flag_is_relaxable=pi.fileflags&BFD_IS_RELAXABLE;

            pi.startaddress=abfd->start_address;
            elf_load_sections(&pi,abfd);
            elf_load_symbols(&pi,abfd);

            break;
        default:
            bfd_close(abfd);
            EXITERROR("Unknown/Unhandled Architecture: %s",pi.machstr);
    }

    pi.filetypestr=bfd_get_target(abfd);
    printf("File: %s\n", pi.filename);
    printf("Filetype: %s\n", pi.filetypestr);
    pi.flavor=bfd_get_flavour(abfd);
    printf("Flavor: ");
    switch (pi.flavor) {
        case bfd_target_elf_flavour:
            printf("ELF");
            break;
        default:
            printf("# flavor: %d - FIXME",pi.flavor);
    }
    if (pi.flag_has_exec) {
        printf(" (exec)");
    } else {
        printf(" (lib)");
    }
    printf("\n");
    printf("Architecture: %s\n", pi.machstr);
    printf("Start Address: %08lx\n", pi.startaddress);
    printf("Loaded %d sections\n", pi.sections_num);
    printf("Loaded %d symbols\n", pi.symbols_num);
    printf("\n");

    for (unsigned int i=0; i<pi.sections_num; ++i) {
        printf("%08lx %08lx %s\n",pi.sections[i]->vma_start,pi.sections[i]->length,pi.sections[i]->name);
    }

    printf("\n");

    for (unsigned int i=0; i<pi.symbols_num; ++i) {
        printf("%08lx %c %s\n",pi.symbols[i]->value,(int)pi.symbols[i]->type,pi.symbols[i]->name);
    }

    /* all done, close file */
    bfd_close(abfd);

    /* all done, clean up */
    if (pi.sections_num>0) {
        /* empty the section array */
        for (unsigned int i=0; i<pi.sections_num; ++i) {
            /* sigh, bfd used malloc to create chunk for contents */
            if (pi.sections[i]->contents) {
                free(pi.sections[i]->contents);
            }
            free(pi.sections[i]);
        }
        free(pi.sections);
        pi.sections_num=0;
    }
    if (pi.symbols_num>0) {
        /* empty the symbols vector */
        for (unsigned int i=0; i<pi.symbols_num; ++i) {
            free(pi.symbols[i]);
        }
        free(pi.symbols);
        pi.symbols_num=0;
    }
}

void elf_load_sections(parsefile_info_t* pi,bfd* abfd)
{
    /* I couldn't determine a clean method to use bfd_map_over... */
    /* load the sections, follow the linked list built by bfd */
    struct bfd_section* sec=abfd->sections;

    pi->sections=malloc(0);
    pi->sections_num=0;

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

        pi->sections=realloc(pi->sections,sizeof(section_t*)*(pi->sections_num+1));
        pi->sections[pi->sections_num]=savesection;
        pi->sections_num++;

        sec=sec->next;
    }
}
void elf_load_symbols(parsefile_info_t* pi,bfd* abfd)
{
    /* ask how big storage for symbols needs to be */
    long int datasize=bfd_get_symtab_upper_bound(abfd);
    /* negative return value indicates no symbols */

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

        pi->symbols=malloc(0);
        pi->symbols_num=0;

        /* process regular symbols */
        for (long i=0; i<numsymbols; i++) {
            symbol_t* sym=malloc(sizeof(symbol_t));

            sym->name=demangle(abfd,symtable[i]->name);
            sym->value=symtable[i]->value;
            sym->flags=symtable[i]->flags;
            sym->type=bfd_decode_symclass(symtable[i]);

            pi->symbols=realloc(pi->symbols,sizeof(symbol_t*)*(pi->symbols_num+1));
            pi->symbols[pi->symbols_num]=sym;
            pi->symbols_num++;
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

            dynsym->name=demangle(abfd,dynsymtable[i]->name);
            dynsym->value=dynsymtable[i]->value;
            dynsym->flags=dynsymtable[i]->flags;
            dynsym->type=bfd_decode_symclass(dynsymtable[i]);

            pi->symbols=realloc(pi->symbols,sizeof(symbol_t*)*(pi->symbols_num+1));
            pi->symbols[pi->symbols_num]=dynsym;
            pi->symbols_num++;
        }
        free(dynsymtable);
    }
}
char* demangle(bfd* abfd,const char* name)
{
    /* remember! bfd malloc's mem for demangling */
    char* retval=bfd_demangle(abfd,name,0);
    if (retval==0) {
        retval=(char*)name;
    }
    return retval;
}
