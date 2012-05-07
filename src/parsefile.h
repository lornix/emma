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
struct s_symbol {
    std::string name;
    unsigned long int value;
    unsigned long int length;
    unsigned int sectionid;
    unsigned long int flags;
};

class parsefile
{
public:
    parsefile(std::string fname);
    ~parsefile();
private:
    bfd* abfd;
    std::vector <struct s_section> sections;
    std::vector <struct s_symbol> symbols;

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
