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
    cout << "Sections\n";
    cout << "=================\n";
    for (unsigned int i=0; i<elf32.sec_headers.size(); i++) {
        cout << i << ": " << elf32.sec_name(i) << "\n";
        cout << "  VMA: 0x" << hex << elf32.sec_headers[i]->sh_addr << dec << "\n";
        cout << " Size: 0x" << hex << elf32.sec_headers[i]->sh_size << dec << "\n";
        cout << " Type: 0x" << hex << elf32.sec_headers[i]->sh_type << dec << " " << elf32.show_sec_type(i) << "\n";
        cout << "Flags: 0x" << hex << elf32.sec_headers[i]->sh_flags << dec << elf32.show_sec_flags(i) << "\n";
        cout << "\n";
    }

    return 0;
}
