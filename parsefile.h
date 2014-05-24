/* parsefile.h */

/*
 * header file for the file parsing code, it populates arrays of sections and
 * their contents, symbols and their values, file information, start address
 * whatever else I can obtain. Eventually to have support for raw data blocks
 */

#ifndef PARSEFILE_H
#define PARSEFILE_H

#include "emma.h"

#include <stdio.h>

typedef enum {
    LITEND=0,
    BIGEND
} endianness;

typedef enum {
    FILETYPE_UNKNOWN=0,
    FILETYPE_RAW32,
    FILETYPE_RAW64,
    FILETYPE_ELF32,
    FILETYPE_ELF64,
    FILETYPE_LIB32,
    FILETYPE_LIB64
} filetype_t;

typedef struct section_t {
    const char* name;
    unsigned long vma_start;
    unsigned long length;
    unsigned int alignment; /* chaotic evil? */
    unsigned long flags;
    char* contents;
    // something to store REL relocations
    // something to store RELA relocations
    // line number info?
} section_t;

typedef struct symbol_t {
    const    char* name;
    unsigned long  value;
    unsigned long  flags;
    unsigned long  type;
} symbol_t;

typedef struct {
    int fd;
    char* filename;
    char* mmap;
    int arch;
    int mach;
    filetype_t filetype;
    unsigned long baseaddress;
    unsigned long startaddress;
    unsigned long length;
    unsigned int section_count;
    section_t** sections;
    unsigned int symbol_count;
    symbol_t** symbols;
    endianness whichendian;
} struct_t;

typedef struct_t* emma_handle;

emma_handle emma_init();
int emma_open(emma_handle* H,const char* fname);
int emma_close(emma_handle* H);
unsigned int emma_section_count(emma_handle* H);
unsigned int emma_symbol_count(emma_handle* H);
section_t* emma_section(emma_handle* H,unsigned int which);
symbol_t* emma_symbol(emma_handle* H,unsigned int which);
char* filetype_str(filetype_t filetype);

#endif
