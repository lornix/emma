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

typedef enum estr_enum {
    ZERO_NOT_VALID=0,
    ENDIAN_LITTLE,
    ENDIAN_BIG,
    BITS_32,
    BITS_64,
    FT_RAW,
    FT_RELOC,
    FT_EXEC,
    FT_DYNLIB,
    FT_CORE,
} estr_enum;

typedef struct symbol_t {
    const    char* name;
    unsigned long  value;
    unsigned long  flags;
    unsigned long  type;
} symbol_t;

typedef struct section_t {
    const    char* name;
    size_t   vma_start;
    size_t   length;
    unsigned int   alignment; /* chaotic evil? */
    unsigned long  flags;
    const    char* contents;
} section_t;

typedef struct segment_t {
    unsigned int type;
    unsigned int flags;
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    size_t   sizefile;
    size_t   sizemem;
    uint64_t alignment;
} segment_t;

typedef struct {
    int fd;
    const char* filename;
    const char* memmap;
    estr_enum elf_class;
    estr_enum bits;
    estr_enum filetype;
    estr_enum endianness;
    uint64_t baseaddress;
    uint64_t startaddress;
    size_t length;
    section_t** sections;
    unsigned int section_count;
    symbol_t** symbols;
    unsigned int symbol_count;
    segment_t** segments;
    unsigned int segment_count;
    uint64_t programheader;
    uint64_t sectionheader;
    unsigned int elfflags;
    unsigned int elfheadersize;
    unsigned int phentsize;
    unsigned int phnum;
    unsigned int shentsize;
    unsigned int shnum;
    unsigned int strindex;
} struct_t;

typedef struct_t* emma_handle;

emma_handle emma_init();
int emma_open(emma_handle* H,const char* fname);
int emma_close(emma_handle* H);
unsigned int emma_symbol_count(emma_handle* H);
unsigned int emma_section_count(emma_handle* H);
unsigned int emma_segment_count(emma_handle* H);
symbol_t* emma_symbol(emma_handle* H,unsigned int which);
section_t* emma_section(emma_handle* H,unsigned int which);
segment_t* emma_segment(emma_handle* H,unsigned int which);
const char* estr(int value);

#endif
