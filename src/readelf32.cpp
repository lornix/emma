#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
// for strncasecmp
#include <cstring>
// for errno
#include <cerrno>

#include <elf.h>
#include <dis-asm.h>

#include "../config.h"

#include "emma.h"
#include "readelf32.h"

// using namespace std;

readelf32::readelf32(std::string fname)
{
    // allows use of a string to specify filename
    readelf32(fname.c_str());
}

readelf32::readelf32(const char* fname)
{
    std::cerr << "Reading '" << fname << "'\n";

    try {
        // see if file exists
        struct stat statbuf;
        if (stat(fname,&statbuf)) {
            throw std::string("File does not exist");
        }
        // yes, but is it a REAL file?
        if (!S_ISREG(statbuf.st_mode)) {
            throw std::string("Not a regular file");
        }
        // whew! attempt to open this file
        FILE* ifile=fopen(fname,"rb");
        if (!ifile) {
            throw std::string("Unable to open file");
        }
        // save filename for later use
        filename=std::string(fname);
        // begin reading in file data, a structure at a time
        if (fread(&elf_hdr,sizeof(elf_hdr),1,ifile)!=1) {
            throw std::string("Error reading file header (err01)");
        }
        // std::cerr << "    Magic: '\\";
        // std::cerr << std::oct;
        // std::cerr << (unsigned int) elf_hdr.e_ident[0];
        // std::cerr << std::dec;
        // std::cerr << elf_hdr.e_ident[1] << elf_hdr.e_ident[2];
        // std::cerr << elf_hdr.e_ident[3] << "'\n";
        if (memcmp(elf_hdr.e_ident,ELFMAG,SELFMAG)!=0) {
            throw std::string("Bad magic, not an ELF based file");
        }
        // this 'module' supports ELF32 i386, make sure that's what we have
        // std::cerr << "    Class: " << (unsigned int)elf_hdr.e_ident[EI_CLASS] << " (1=32 bit)\n";
        if (elf_hdr.e_ident[EI_CLASS]!=ELFCLASS32) {
            throw std::string("I only understand i386 32-bit");
        }
        // look for little-endian structure
        // std::cerr << "   Endian: " << (unsigned int)elf_hdr.e_ident[EI_DATA] << " (1=little endian)\n";
        if (elf_hdr.e_ident[EI_DATA]!=ELFDATA2LSB) {
            throw std::string("I only know little-endian");
        }
        // is ELF 'version' current?
        // std::cerr << "  ELF-Ver: " << (unsigned int)elf_hdr.e_ident[EI_VERSION] << " (1=current)\n";
        if (elf_hdr.e_ident[EI_VERSION]!=EV_CURRENT) {
            throw std::string("ELF version not current");
        }
        // OSABI = linux or gnu?
        // std::cerr << "    OSABI: " << (unsigned int)elf_hdr.e_ident[EI_OSABI] << " (0=none/sysv,3=gnu/linux)\n";
        if ( (elf_hdr.e_ident[EI_OSABI]!=ELFOSABI_GNU)      &&
                (elf_hdr.e_ident[EI_OSABI]!=ELFOSABI_LINUX) &&
                (elf_hdr.e_ident[EI_OSABI]!=ELFOSABI_SYSV)  &&
                (elf_hdr.e_ident[EI_OSABI]!=ELFOSABI_NONE)  ) {
            throw std::string("OSABI not GNU or LINUX");
        }
        // std::cerr << "  ABI-Ver: " << (unsigned int)elf_hdr.e_ident[EI_ABIVERSION] << "\n";
        // std::cerr << "  Padding: Bytes " << EI_PAD << " through " << (EI_NIDENT-1) << "\n";
        // std::cerr << "File Type: " << elf_hdr.e_type << "\n";
        // std::cerr << "  Machine: " << elf_hdr.e_machine << "\n";
        // std::cerr << "  Version: " << elf_hdr.e_version << "\n";
        // std::cerr << "Exec Addr: 0x" << std::hex << elf_hdr.e_entry << std::dec << "\n";

        // save the starting point of program
        exec_addr=elf_hdr.e_entry;
        unsigned int prg_hdr_offset=elf_hdr.e_phoff;
        unsigned int sec_hdr_offset=elf_hdr.e_shoff;
        unsigned int proc_flags=elf_hdr.e_flags;
        unsigned int elf_hdr_size=elf_hdr.e_ehsize;
        unsigned int prg_hdr_size=elf_hdr.e_phentsize;
        unsigned int prg_hdr_count=elf_hdr.e_phnum;
        unsigned int sec_hdr_size=elf_hdr.e_shentsize;
        unsigned int sec_hdr_count=elf_hdr.e_shnum;
        unsigned int str_table_index=elf_hdr.e_shstrndx;

        prg_hdr_offset=prg_hdr_offset;
        sec_hdr_offset=sec_hdr_offset;
        proc_flags=proc_flags;
        elf_hdr_size=elf_hdr_size;
        prg_hdr_size=prg_hdr_size;
        prg_hdr_count=prg_hdr_count;
        sec_hdr_size=sec_hdr_size;
        sec_hdr_count=sec_hdr_count;
        str_table_index=str_table_index;



        // all done!
        fclose(ifile);
    } catch (std::string e) {
        if (errno!=0) {
            // if error was from function call?
            perror(e.c_str());
        } else {
            // nope, just display the error
            std::cerr << "Error: " << e << "\n";
        }
        // exit program (one of few exit points)
        exit(1);
    }
}

readelf32::~readelf32()
{
}
