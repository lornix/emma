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

typedef enum {
    little_endian,
    big_endian
} endianness;

typedef struct section_t {
    const char* name;
    unsigned char* contents;
    unsigned long vma_start;
    unsigned long int length;
    unsigned int alignment; /* chaotic evil? */
    unsigned long int flags;
    // something to store REL relocations
    // something to store RELA relocations
    // look at reloc_cache_entry in bfd.h
    // line number info?
} section_t;

typedef section_t EMMA_SECTION;

typedef struct symbol_t {
    const char* name;
    unsigned long int value;
    unsigned long int flags;
    unsigned long int type;
} symbol_t;

typedef symbol_t EMMA_SYMBOL;

typedef struct {
    bfd* abfd;
    char* filename;
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
    endianness whichendian;
} EMMA_STRUCT;

typedef EMMA_STRUCT* EMMA_HANDLE;

EMMA_HANDLE emma_init();
int emma_open(EMMA_HANDLE* handle,const char* fname);
int emma_close(EMMA_HANDLE* handle);
unsigned int emma_section_count(EMMA_HANDLE* handle);
EMMA_SECTION* emma_section(EMMA_HANDLE* handle,unsigned int which);
unsigned int emma_symbol_count(EMMA_HANDLE* handle);
EMMA_SYMBOL* emma_symbol(EMMA_HANDLE* handle,unsigned int which);

#endif
