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
    uint64_t  value;
    uint64_t  flags;
    uint64_t  type;
    char* name;
} symbol_t;

typedef struct section_t {
    const char* name;
    uint64_t addr;
    uint64_t offset;
    uint64_t size;
    uint64_t flags;
    uint64_t align;
    uint64_t entsize;
    unsigned int type;
    unsigned int link;
    unsigned int info;
    unsigned int padding;
} section_t;

typedef struct segment_t {
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    uint64_t sizefile;
    uint64_t sizemem;
    uint64_t alignment;
    unsigned int type;
    unsigned int flags;
} segment_t;

typedef struct {
    const char* filename;
    const char* memmap;
    uint64_t baseaddress;
    uint64_t startaddress;
    uint64_t length;
    section_t** sections;
    symbol_t** symbols;
    segment_t** segments;
    uint64_t programheader;
    uint64_t sectionheader;
    unsigned int section_count;
    unsigned int symbol_count;
    unsigned int segment_count;
    unsigned int elfflags;
    unsigned int elfheadersize;
    unsigned int phentsize;
    unsigned int phnum;
    unsigned int shentsize;
    unsigned int shnum;
    unsigned int section_string_index;
    int fd;
    estr_enum elf_class;
    estr_enum bits;
    estr_enum filetype;
    estr_enum endianness;
    unsigned int padding;
} struct_t;

typedef struct_t* emma_handle;

emma_handle emma_init(void);
int emma_open(emma_handle* H,const char* fname);
int emma_close(emma_handle* H);
unsigned int emma_symbol_count(emma_handle* H);
unsigned int emma_section_count(emma_handle* H);
unsigned int emma_segment_count(emma_handle* H);
symbol_t* emma_get_symbol(emma_handle* H,unsigned int which);
section_t* emma_get_section(emma_handle* H,unsigned int which);
segment_t* emma_get_segment(emma_handle* H,unsigned int which);
const char* estr(int value);
uint16_t make_little_endian_word(emma_handle* H,uint16_t value);
uint32_t make_little_endian_dword(emma_handle* H,uint32_t value);
uint64_t make_little_endian_quad(emma_handle* H,uint64_t value);

#endif
