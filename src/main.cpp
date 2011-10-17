#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>

// for getopt_long
#include <unistd.h>
#include <getopt.h>

#include <elf.h>
#include <bfd.h>
#include <dis-asm.h>

#include "../config.h"

#include "emma.h"
#include "readelf32.h"
#include "utils.h"

using namespace std;

void help_usage()
{
    cout << "usage: emma [option]... [file]\n";
    cout << "\n";
    cout << "-h, --help\tShow this help.\n";
    cout << "-v, --verbose\tBe verbose.\n";
    exit(0);
}

int main(int argc,char* argv[])
{
    static struct option long_options[]={
        {"help"   ,no_argument,0,'h'},
    };

    int opt;
    int option_index=0;
    while ((opt=getopt_long(argc,argv,"vh",
                    long_options,&option_index))>=0) {
        switch (opt) {
            case 'h': /* help */
                help_usage(); break; /* never returns */
            case '?':
            default: /* unknown */
                break;
        }

    }

    // are we out of parameters?
    if (argc<=optind) {
        cerr << "Please provide a filename to process\n";
        return 1;
    }

    // load in file data
    readelf32 elf32(argv[optind]);

    cout << "\n";
    cout << "Program Sections (" << elf32.prg_headers.size() << ")\n";
    cout << "=================\n";
    for (unsigned int i=0; i<elf32.prg_headers.size(); i++) {
        cout << i << ": " << "\n";
        cout << "  Type: " << hexval0x(elf32.prg_headers[i]->p_type) << elf32.show_prg_type(i) << "\n";
        cout << "Offset: " << hexval0x(elf32.prg_headers[i]->p_offset,8);
        for (unsigned int j=1; j<elf32.sec_headers.size(); j++) {
            if (elf32.sec_headers[j]->sh_offset>=elf32.prg_headers[i]->p_offset) {
                if (elf32.sec_headers[j]->sh_offset<(elf32.prg_headers[i]->p_offset+elf32.prg_headers[i]->p_memsz)) {
                    cout << " (" << elf32.sec_name(j) << "=" << hexval0x(elf32.sec_headers[j]->sh_offset) << ")";
                }
            }
        }
        cout <<"\n";
        cout << " Vaddr: " << hexval0x(elf32.prg_headers[i]->p_vaddr,8) << "\n";
        cout << " Paddr: " << hexval0x(elf32.prg_headers[i]->p_paddr,8) << "\n";
        cout << "Pfsize: " << hexval0x(elf32.prg_headers[i]->p_filesz) << "\n";
        cout << "Pmemsz: " << hexval0x(elf32.prg_headers[i]->p_memsz) << "\n";
        cout << " Flags: " << hexval0x(elf32.prg_headers[i]->p_flags) << elf32.show_prg_flags(i) << "\n";
        cout << " Align: " << hexval0x(elf32.prg_headers[i]->p_align) << "\n";
        cout << "\n";
    }
    cout << "\n";
    cout << "Sections (" << elf32.sec_headers.size() << ")\n";
    cout << "=================\n";
    for (unsigned int i=0; i<elf32.sec_headers.size(); i++) {
        cout << i << ": " << elf32.sec_name(i) << "\n";
        cout << "Offset: " << hexval0x(elf32.sec_headers[i]->sh_offset,8) << "\n";
        cout << "   VMA: " << hexval0x(elf32.sec_headers[i]->sh_addr,8) << "\n";
        cout << "  Size: " << hexval0x(elf32.sec_headers[i]->sh_size) << "\n";
        cout << "  Type: " << hexval0x(elf32.sec_headers[i]->sh_type) << elf32.show_sec_type(i) << "\n";
        cout << " Flags: " << hexval0x(elf32.sec_headers[i]->sh_flags) << elf32.show_sec_flags(i) << "\n";
        cout << "\n";
    }

    return 0;
}
