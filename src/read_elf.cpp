#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>

#include <bfd.h>
#include <dis-asm.h>

#include "../config.h"

#include "emma.h"
#include "read_elf.h"

using namespace std;

read_elf::read_elf(string fname)
{
    int count=load_sections(fname);
    if (count<0)
        throw count;
    section_count=count;
    cerr << "Loaded " << count << " sections\n";
}

read_elf::~read_elf()
{
    // not sure what to do for destroying the filedata vector,
    // it's built from mallocs by bfd_..., icky.
}

unsigned int read_elf::size()
{
    return section_count;
}

const read_elf::filedata_s* read_elf::operator[](unsigned int index)
{
    // unsigned .. can't be negative, just check upper bound
    if (index<section_count) {
        return &filedata[index];
    }
    return 0;
}

const read_elf::filedata_s* read_elf::section(string name)
{
    for (unsigned int i=0; i<section_count; i++) {
        if (name==string(filedata[i].a.name))
            return &filedata[i];
    }
    return 0;
}

// grab_section NOT in read_elf class due to weird pointer
// mutations involved that I don't know how to fix yet
void grab_section(bfd* b,asection* s,void* vv)
{
    // convert pointer from (void*) to (filedata_s*)
    vector<read_elf::filedata_s>*v=reinterpret_cast<vector<read_elf::filedata_s>*>(vv);

    // create a new 'filedata_s' unit
    read_elf::filedata_s fd;
    // save the section data
    fd.a=*s;
    // retrieve and save the section data contents
    bfd_malloc_and_get_section(b,s,&fd.b);

    // stuff it into storage
    v->push_back(fd);
}

int read_elf::load_sections(string fname)
{
    const char* target="elf32-i386";

    // initialize bfd systems
    bfd_init();

    if (!bfd_set_default_target(target)) {
        bfd_perror("Can't set BFD default target");
        return -1;
    }

    // open desired file
    bfd* bfile=bfd_openr(fname.c_str(),target);
    if (!bfile) {
        cerr << "Error opening " << fname << "\n";
        return -2;
    }
    if (!bfd_check_format(bfile,bfd_object)) {
        bfd_perror(NULL);
        return -3;
    }

    int elf_header_size=bfd_get_elf_phdr_upper_bound(bfile);
    unsigned char elf_header[elf_header_size];
    // load the header data, discard the count retval
    // int elf_count=bfd_get_elf_phdrs(bfile,&elf_header);
    bfd_get_elf_phdrs(bfile,&elf_header);

    bfd_map_over_sections(bfile,grab_section,&filedata);
    bfd_close(bfile);

    // return count of sections loaded
    return filedata.size();
}
