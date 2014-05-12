/* parsefile.h */

/*
 * header file for the file parsing code, it populates arrays of sections and
 * their contents, symbols and their values, file information, start address
 * whatever else I can obtain from the bfd backend for ELF/PE/a.out... type
 * files.  Eventually to have support for raw data blocks too
 */

#ifndef PARSEFILE_H
#define PARSEFILE_H

#include <bfd.h>

#include "emma.h"

typedef struct section_t {
    const char* name;
    unsigned char* contents;
    vma_t vma_start;
    vma_t vma_end;
    unsigned long int length;
    unsigned int alignment; /* chaotic evil? */
    unsigned long int flags;
    // something to store REL relocations
    // something to store RELA relocations
    // look at reloc_cache_entry in bfd.h
    // line number info?
} section_t;

typedef struct symbol_t {
    const char* name;
    unsigned long int value;
    unsigned long int length;
    unsigned long int flags;
    unsigned long int type;
    unsigned int sectionid;
} symbol_t;

// linenumber storage (eventually)
typedef struct linenum_t {
    unsigned int line_number;
    union {
        symbol_t* sym;
        vma_t offset;
    } u;
} linenum_t;

// formfeed 

typedef struct parsefile_info_t
{
    const char* filename;
    enum bfd_architecture arch;
    unsigned long mach;
    const char* machstr;
    const char* filetypestr;
    const char* flavorstr;
    enum bfd_flavour flavor;
    flagword fileflags;
    unsigned long int startaddress;
    section_t** sections;
    unsigned int sections_num;
    symbol_t** symbols;
    unsigned int symbols_num;
    unsigned int flag_has_reloc;
    unsigned int flag_has_exec;
    unsigned int flag_has_linenums;
    unsigned int flag_has_debug;
    unsigned int flag_has_symbols;
    unsigned int flag_has_locals;
    unsigned int flag_has_dynamic;
    unsigned int flag_is_relaxable;

} parsefile_info_t;

void parsefile(const char* fname);
void elf_load_sections(parsefile_info_t* pi,bfd* abfd);
void elf_load_symbols(parsefile_info_t* pi,bfd* abfd);
char* demangle(bfd* abfd,const char* name);

#endif
