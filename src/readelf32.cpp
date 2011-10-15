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

readelf32::readelf32(std::string fname) throw(std::string)
{
    // allows use of a string to specify filename
    readelf32(fname.c_str());
}

readelf32::readelf32(const char* fname) throw(std::string)
{
    std::cerr << "readelf32: " << fname << "\n";

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
        // save filename
        filename=std::string(fname);
        // begin reading in file data, a structure at a time
        int err=fread(&elf_hdr,sizeof(elf_hdr),1,ifile);
        if (err!=1) {
            throw std::string("Error reading file (e1)");
        }
        if (memcmp(elf_hdr.e_ident,ELFMAG,SELFMAG)!=0) {
            throw std::string("Not an ELF based file");
        }


        // all done!
        fclose(ifile);
    } catch (std::string e) {
        if (errno!=0) {
            perror(e.c_str());
        } else {
            std::cerr << "Error: " << e << "\n";
        }
        exit(1);
    }
}

readelf32::~readelf32()
{
}
