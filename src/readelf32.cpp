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
    // initialize to null
    fdata=0;

    std::cerr << "Reading '" << fname << "'\n";

    // save filename for later use
    filename=std::string(fname);

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
        // create storage for ELF header
        Elf32_Ehdr elf_hdr;
        // attempt to read in ELF header
        if (fread(&elf_hdr,sizeof(elf_hdr),1,ifile)!=1) {
            throw std::string("Error reading ELF header");
        }
        if (memcmp(elf_hdr.e_ident,ELFMAG,SELFMAG)!=0) {
            throw std::string("Bad magic, not an ELF based file");
        }
        // this 'module' supports ELF32 i386, make sure that's what we have
        if (elf_hdr.e_ident[EI_CLASS]!=ELFCLASS32) {
            throw std::string("I only understand i386 32-bit");
        }
        // look for little-endian structure
        if (elf_hdr.e_ident[EI_DATA]!=ELFDATA2LSB) {
            throw std::string("I only know little-endian");
        }
        // is ELF 'version' current?
        if (elf_hdr.e_ident[EI_VERSION]!=EV_CURRENT) {
            throw std::string("ELF version not current");
        }
        // OSABI = linux or gnu?
        if ( (elf_hdr.e_ident[EI_OSABI]!=ELFOSABI_GNU)      &&
                (elf_hdr.e_ident[EI_OSABI]!=ELFOSABI_LINUX) &&
                (elf_hdr.e_ident[EI_OSABI]!=ELFOSABI_SYSV)  &&
                (elf_hdr.e_ident[EI_OSABI]!=ELFOSABI_NONE)  ) {
            throw std::string("OSABI not GNU/LINUX/SYSV/NONE");
        }

        // header is valid, pull data from file

        // save the starting point of program
        exec_addr=elf_hdr.e_entry;
        proc_flags=elf_hdr.e_flags;

        // get size in bytes
        fsize=statbuf.st_size;
        // reserve space
        fdata=new unsigned char[fsize];
        // rewind to start of file again
        if (fseek(ifile,0,SEEK_SET)) {
            throw std::string("Can't rewind file");
        }
        // read in whole file
        if (fread(fdata,fsize,1,ifile)!=1) {
            throw std::string("Error reading file");
        }
        // done with file
        fclose(ifile);

        // set up all the section header pointers
        // section[0] is special, containing additional info for phnum and shnum
        sec_headers.push_back(reinterpret_cast<Elf32_Shdr*>(fdata+elf_hdr.e_shoff));
        //
        unsigned int shcount=elf_hdr.e_shnum;
        if (shcount==0) {
            shcount=sec_headers[0]->sh_size;
        }
        // already set up [0]
        for (unsigned int i=1; i<shcount; i++) {
            Elf32_Shdr* ptr=reinterpret_cast<Elf32_Shdr*>(fdata+elf_hdr.e_shoff+(i*elf_hdr.e_shentsize));
            sec_headers.push_back(ptr);
        }
        //
        unsigned int phcount=elf_hdr.e_phnum;
        if (elf_hdr.e_phnum==PN_XNUM) { // header count > 0xffff
            phcount=sec_headers[0]->sh_info;
        }
        // set up all the program header pointers
        for (unsigned int i=0; i<phcount; i++) {
            Elf32_Phdr* ptr=reinterpret_cast<Elf32_Phdr*>(fdata+elf_hdr.e_phoff+(i*elf_hdr.e_phentsize));
            prg_headers.push_back(ptr);
        }
        // set up section name string table
        sec_name_table=elf_hdr.e_shstrndx;
        if (sec_name_table!=SHN_UNDEF) {
            if (sec_name_table==SHN_XINDEX) {
                sec_name_table=sec_headers[0]->sh_link;
            }
        }

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
    if (fdata) {
        delete fdata;
    }
    // sec_headers and prg_headers are destroyed when
    // the class is destroyed.
}

const unsigned char* readelf32::sec_name(unsigned int section_num)
{
    // return const char* based on sh_name offset into
    // str_table_index section
    const unsigned char* ptr=fdata;
    ptr+=sec_headers[sec_name_table]->sh_offset;
    ptr+=sec_headers[section_num]->sh_name;
    return ptr;
}

std::string readelf32::show_sec_flags(unsigned int section_num)
{
    std::string retval;
    unsigned int flags=sec_headers[section_num]->sh_flags;
    if (flags&SHF_WRITE) retval+=" Write";
    if (flags&SHF_ALLOC) retval+=" Alloc";
    if (flags&SHF_EXECINSTR) retval+=" Exec";
    if (flags&SHF_MASKPROC) retval+=" Rsvd";
    if (flags&SHF_MERGE) retval+=" Merge";
    if (flags&SHF_STRINGS) retval+=" Strings";
    if (flags&SHF_INFO_LINK) retval+=" Info_Link";
    if (flags&SHF_LINK_ORDER) retval+=" Link_Order";
    if (flags&SHF_OS_NONCONFORMING) retval+=" OS_Noncon";
    if (flags&SHF_GROUP) retval+=" Group";
    if (flags&SHF_TLS) retval+=" Thread_Local_Data";
    if (flags&SHF_ORDERED) retval+=" Ordered";
    if (flags&SHF_EXCLUDE) retval+=" Exclude";
    return retval;
}
std::string readelf32::show_sec_type(unsigned int section_num)
{
    std::string retval;
    unsigned int types=sec_headers[section_num]->sh_type;
    switch (types) {
        case SHT_NULL:
            retval="Null"; break;
        case SHT_PROGBITS:
            retval="Program Data"; break;
        case SHT_SYMTAB:
            retval="Symbol Table"; break;
        case SHT_STRTAB:
            retval="String Table"; break;
        case SHT_RELA:
            retval="Relocation Entries (addends)"; break;
        case SHT_HASH:
            retval="Symbol Hash Table"; break;
        case SHT_DYNAMIC:
            retval="Dynamic Linking"; break;
        case SHT_NOTE:
            retval="Notes"; break;
        case SHT_NOBITS:
            retval="NoBits"; break;
        case SHT_REL:
            retval="Relocation Entries (no addends)"; break;
        case SHT_SHLIB:
            retval="Reserved (SHLIB)"; break;
        case SHT_DYNSYM:
            retval="Dynamic Linker Symbol Table"; break;
        case SHT_INIT_ARRAY:
            retval="Array of Constructors"; break;
        case SHT_FINI_ARRAY:
            retval="Array of Destructors"; break;
        case SHT_PREINIT_ARRAY:
            retval="Array of Pre-Constructors"; break;
        case SHT_GROUP:
            retval="Section Group"; break;
        case SHT_SYMTAB_SHNDX:
            retval="Extended Section Indices"; break;
        case SHT_NUM:
            retval="Number of defined types"; break;
        case SHT_GNU_ATTRIBUTES:
            retval="Object Attributes"; break;
        case SHT_GNU_HASH:
            retval="GNU-style Hash Table"; break;
        case SHT_GNU_LIBLIST:
            retval="Prelink Library List"; break;
        case SHT_CHECKSUM:
            retval="Checksum for DSO Content"; break;
        case SHT_GNU_verdef:
            retval="Version Definition Section"; break;
        case SHT_GNU_verneed:
            retval="Version Needs Section"; break;
        case SHT_GNU_versym:
            retval="Version Symbol Table"; break;
    }
    return retval;
}
