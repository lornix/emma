#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>

#include <bfd.h>
#include <dis-asm.h>

#include "../config.h"

using namespace std;

const int maxlen = 40;

struct alldata {
    asection a;
    bfd_byte* b;
};

void grab_section(bfd* b,asection* s,void* vv)
{
    // convert pointer from (void*) to (alldata*)
    vector<alldata>*v=static_cast<vector<alldata>*>(vv);

    // create a new 'alldata' unit
    alldata ad;
    // save the section data
    ad.a=*s;
    // retrieve and save the section data contents
    bfd_malloc_and_get_section(b,s,&ad.b);

    // stuff it into storage
    v->push_back(ad);
}

const char* target="elf32-i386";

int main(int argc,char* argv[])
{
    if (argc<2) {
        cerr << "Need a filename\n";
        return 1;
    }

    // initialize bfd systems
    bfd_init();

    if (!bfd_set_default_target(target)) {
        bfd_perror("Can't set BFD default target");
        return 1;
    }

    // open desired file
    bfd* bfile=bfd_openr(argv[1],target);
    if (!bfile) {
        cerr << "Error opening " << argv[1] << "\n";
        return 1;
    }
    if (!bfd_check_format(bfile,bfd_object)) {
        bfd_perror(NULL);
    }

    cout << "File size: " << bfd_get_size(bfile) << "\n";

    cout << "# Sections: " << bfd_count_sections(bfile) << "\n";

    cout << "\n";

    int elf_header_size=bfd_get_elf_phdr_upper_bound(bfile);
    unsigned char elf_header[elf_header_size];
    int elf_count=bfd_get_elf_phdrs(bfile,&elf_header);

    cout << elf_count << " ELF headers read (" << elf_header_size << " bytes)";
    // {
    //     int linecount=0;
    //     for (int i=0; i<elf_header_size; i++) {
    //         if (linecount==0)
    //             printf("\n%8x: ",i);
    //         printf("%02x ",elf_header[i]);
    //         if ((linecount%4)==3)
    //             printf(" ");
    //         linecount=(linecount+1)%16;
    //     }
    //     cout << "\n";
    // }

    cout << "\n";

    vector<alldata> sections;
    bfd_map_over_sections(bfile,grab_section,&sections);
    bfd_close(bfile);

    for (vector<alldata>::iterator i=sections.begin(); i!=sections.end(); i++) {
        cout << i->a.index << ": " << i->a.name << "\n";
        cout << "\t        VMA: 0x" << hex << i->a.vma << dec << "\n";
        cout << "\tFile Offset: 0x" << hex << i->a.filepos << dec << "\n";
        cout << "\t       size: 0x" << hex << i->a.size << dec << "\n";
    }

    return 0;
}
