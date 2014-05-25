/* parsefile.h */

/*
 * header file for the file parsing code, it populates arrays of sections and
 * their contents, symbols and their values, file information, start address
 * whatever else I can obtain. Eventually to have support for raw data blocks
 */

#ifndef PARSEFILE_H
#define PARSEFILE_H

#include "emma.h"

/* for uint??_t defines */
#include <stdint.h>

enum {
    ZERO_FILLER=0,
    ENDIAN_LITTLE,
    ENDIAN_BIG,
    BITS_32,
    BITS_64,
    FT_RAW,
    FT_RELOC,
    FT_EXEC,
    FT_DYNLIB,
    FT_CORE,
};

static const uint64_t DEFAULT_BASE_32bits __attribute__((used)) =0x8048000;
static const uint64_t DEFAULT_BASE_64bits __attribute__((used)) =0x0400000;

typedef struct section_t {
    const char* name;
    unsigned long vma_start;
    size_t length;
    unsigned int alignment; /* chaotic evil? */
    unsigned long flags;
    char* contents;
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
    char* memmap;
    int bits;
    int filetype;
    uint64_t baseaddress;
    uint64_t startaddress;
    size_t length;
    unsigned int section_count;
    section_t** sections;
    unsigned int symbol_count;
    symbol_t** symbols;
    int endianness;
    char* programheader;
    char* sectionheader;
    unsigned long elfflags;
    unsigned long elfheadersize;
    unsigned long phentsize;
    unsigned long phnum;
    unsigned long shentsize;
    unsigned long shnum;
    unsigned long strindex;
} struct_t;

typedef struct_t* emma_handle;

emma_handle emma_init();
int emma_open(emma_handle* H,const char* fname);
int emma_close(emma_handle* H);
unsigned int emma_section_count(emma_handle* H);
unsigned int emma_symbol_count(emma_handle* H);
section_t* emma_section(emma_handle* H,unsigned int which);
symbol_t* emma_symbol(emma_handle* H,unsigned int which);
char* estr(int value);

#endif
