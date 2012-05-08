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
#include <stdexcept>
#include <vector>
#include <cstdlib>

#include <bfd.h>

#include "../config.h"
#include "emma.h"

struct s_section {
    std::string name;
    unsigned char* contents;
    unsigned long int vma;
    unsigned long int length;
    unsigned int alignment; /* chaotic evil? */
    unsigned long int flags;
    // something to store REL relocations
    // something to store RELA relocations
    // line number info?
};
typedef struct s_section section_t;

struct s_symbol {
    std::string name;
    unsigned long int value;
    unsigned long int length;
    unsigned int sectionid;
    unsigned long int flags;
};
typedef struct s_symbol symbol_t;

class parsefile
{
public:
    parsefile(std::string fname);
    ~parsefile();
private: /* variables */
    bfd* abfd;
    std::vector <section_t> sections;
    std::vector <symbol_t> symbols;
private: /* functions */
    void load_sections(bfd* abfd);

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
};

#endif
