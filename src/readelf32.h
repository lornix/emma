/* emma - readelf32.h */

#ifndef READELF32_H
#define READELF32_H

#include <iostream>
#include <vector>
#include <string>
// fopen/fread/fclose
#include <cstdio>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
// for memcmp
 #include <cstring>
// for errno
#include <cerrno>

#include <elf.h>

class readelf32
{
 public: /* functions */
    readelf32(std::string fname);
    ~readelf32();
    //
    const char* sec_name(unsigned int section_num);
    unsigned int find_section_name(std::string name);
    std::string show_sec_flags(unsigned int section_num);
    std::string show_sec_type(unsigned int section_num);
    std::string show_prg_flags(unsigned int prg_section);
    std::string show_prg_type(unsigned int prg_section);
    //
 public: /* variables */
    // points to array holding file data
    char* fdata;
    // pointers into the fdata array
    std::vector<Elf32_Phdr*> prg_headers;
    std::vector<Elf32_Shdr*> sec_headers;
    std::vector<const char*> sec_names;
    std::vector<Elf32_Sym*> symbols;
    //
 private:
    std::string filename;
    // information from the elf_hdr
    unsigned int exec_addr;
    unsigned int proc_flags;
    unsigned int sec_name_table;
    //
    unsigned long int fsize;
};

#endif
