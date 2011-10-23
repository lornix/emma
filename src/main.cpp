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
    readelf32 f(opts.filename);

    // extract symbols
    // read symbols from .symtab/.strtab
    unsigned int symtab=f.find_section_name(".symtab");
    unsigned int strtab=f.find_section_name(".strtab");
    if ((symtab==0)||(strtab==0)) {
        throw("That's weird, symbol table not found");
    }
    cout << "symtab = " << symtab << "\n";
    cout << "strtab = " << strtab << "\n";
    // skip record 0, it's a dummy
    for (unsigned int i=sizeof(Elf32_Sym);
            i<f.sec_headers[symtab]->sh_size;
            i+=sizeof(Elf32_Sym)) {
        Elf32_Sym* sym=reinterpret_cast<Elf32_Sym*>(f.fdata+f.sec_headers[symtab]->sh_offset+i);
        cout << hexval0x(sym->st_value,8) << "  ";
        cout << hexval(sym->st_size,8) << "  ";
        cout << "-";
        unsigned char stinfo_b=ELF32_ST_BIND(sym->st_info);
        cout << ((stinfo_b==STB_LOCAL     )?"Loc":"");
        cout << ((stinfo_b==STB_GLOBAL    )?"Glo":"");
        cout << ((stinfo_b==STB_WEAK      )?"Wea":"");
        cout << ((stinfo_b==STB_NUM       )?"Num":"");
        cout << ((stinfo_b==STB_GNU_UNIQUE)?"Uni":"");
        cout << "-";
        unsigned char stother=ELF32_ST_VISIBILITY(sym->st_other);
        cout << ((stother==STV_DEFAULT  )?"   ":"");
        cout << ((stother==STV_INTERNAL )?"Int":"");
        cout << ((stother==STV_HIDDEN   )?"Hid":"");
        cout << ((stother==STV_PROTECTED)?"Pro":"");
        cout << "-";
        unsigned char stinfo_t=ELF32_ST_TYPE(sym->st_info);
        cout << ((stinfo_t==STT_NOTYPE   )?"   ":"");
        cout << ((stinfo_t==STT_OBJECT   )?"Obj":"");
        cout << ((stinfo_t==STT_FUNC     )?"Fun":"");
        cout << ((stinfo_t==STT_SECTION  )?"Sec":"");
        cout << ((stinfo_t==STT_FILE     )?"Fil":"");
        cout << ((stinfo_t==STT_COMMON   )?"Com":"");
        cout << ((stinfo_t==STT_TLS      )?"TLD":"");
        cout << ((stinfo_t==STT_NUM      )?"Num":"");
        cout << ((stinfo_t==STT_GNU_IFUNC)?"Gnu":"");
        cout << "-";
        cout << "  ";
        unsigned int shndx=sym->st_shndx;
        switch (shndx) {
            case 0:
                cout << "*Und*"; break;
            case SHN_ABS:
                cout << "*Abs*"; break;
            case SHN_COMMON:
                cout << "*Com*"; break;
            default:
                cout << f.sec_name(shndx); break;
        }
        cout << "\t";
        cout << f.fdata+f.sec_headers[strtab]->sh_offset+sym->st_name;
        cout << "\n";
    }

    return 0;
}
