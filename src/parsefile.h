/* parsefile.h */

/*
 * header file for the file parsing class, it populates arrays of sections and
 * their contents, symbols and their values, file information, start address
 * whatever else I can obtain from the bfd backend for ELF/PE/a.out... type
 * files.  Eventually to have support for raw data blocks too
 */

#ifndef PARSEFILE_H
#define PARSEFILE_H

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <vector>
#include <cstdlib>

#include <bfd.h>

#include "../config.h"
#include "emma.h"

typedef struct s_section {
    std::string name;
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

typedef struct s_symbol {
    std::string name;
    unsigned long int value;
    unsigned long int length;
    unsigned long int flags;
    unsigned long int type;
    unsigned int sectionid;
} symbol_t;

// linenumber storage (eventually)
typedef struct s_linenum {
    unsigned int line_number;
    union {
        symbol_t* sym;
        vma_t offset;
    } u;
} linenum_t;

class parsefile
{
public:
    parsefile(std::string fname);
    ~parsefile();
private: /* variables */
    std::string filename;
    unsigned long int startaddress;
    std::vector <section_t> sections;
    std::vector <symbol_t> symbols;
    bool flag_has_reloc;
    bool flag_has_linenums;
    bool flag_has_debug;
    bool flag_has_symbols;
    bool flag_has_locals;
    bool flag_has_dynamic;
    bool flag_is_relaxable;

private: /* functions */
    void elf_load_sections(bfd* abfd);
    void elf_load_symbols(bfd* abfd);
    std::string demangle(bfd* abfd,const char* name);

    /* exceptions used by parsefile */
public:
    class CantOpenFile : public std::runtime_error
    {
     public:
        CantOpenFile(std::string const& msg) : runtime_error(msg) { };
    };
    class NotValidFile : public std::runtime_error
    {
     public:
        NotValidFile(std::string const& msg) : runtime_error(msg) { };
    };
    class OutOfMemory : public std::runtime_error
    {
     public:
        OutOfMemory(std::string const& msg) : runtime_error(msg) { };
    };
    class GeneralError : public std::runtime_error
    {
     public:
        GeneralError(std::string const& msg) : runtime_error(msg) { };
    };
};

#endif
