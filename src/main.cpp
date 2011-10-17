/* emma - main.cpp */

#include <iostream>
// for exit
#include <cstdlib>

// #include <elf.h>
// #include <bfd.h>
// #include <dis-asm.h>

using namespace std;

#include "../config.h"

#include "emma.h"

#include "options.h"
#include "readelf32.h"
#include "utils.h"


int main(int argc,const char* argv[])
{
    // parse options, does not return if no filename given
    options opts(argc,&argv[0]);

    // load in file data
    readelf32 elf32(opts.filename);

    cout << "\n";
    cout << "Program Sections (" << elf32.prg_headers.size() << ")\n";
    cout << "=================\n";
    for (unsigned int i=0; i<elf32.prg_headers.size(); i++) {
        cout << i << ": " << "\n";
        cout << "  Type: " << hexval0x(elf32.prg_headers[i]->p_type) << elf32.show_prg_type(i) << "\n";
        cout << "Offset: " << hexval0x(elf32.prg_headers[i]->p_offset,8);
        int count=0;
        for (unsigned int j=1; j<elf32.sec_headers.size(); j++) {
            if (elf32.sec_headers[j]->sh_offset>=elf32.prg_headers[i]->p_offset) {
                if (elf32.sec_headers[j]->sh_offset<(elf32.prg_headers[i]->p_offset+elf32.prg_headers[i]->p_memsz)) {
                    if (count==0) cout << "\n\t\t";
                    cout << " (" << elf32.sec_name(j) << "=" << hexval0x(elf32.sec_headers[j]->sh_offset) << ")";
                    count=(count+1)%4;
                }
            }
        }
        cout <<"\n";
        cout << " Vaddr: " << hexval0x(elf32.prg_headers[i]->p_vaddr,8) << "\n";
        cout << " Paddr: " << hexval0x(elf32.prg_headers[i]->p_paddr,8) << "\n";
        cout << "Pfsize: " << hexval0x(elf32.prg_headers[i]->p_filesz)  << "\n";
        cout << "Pmemsz: " << hexval0x(elf32.prg_headers[i]->p_memsz)   << "\n";
        cout << " Flags: " << hexval0x(elf32.prg_headers[i]->p_flags)   << elf32.show_prg_flags(i) << "\n";
        cout << " Align: " << hexval0x(elf32.prg_headers[i]->p_align)   << "\n";
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
