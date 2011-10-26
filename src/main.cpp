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

    for (unsigned int i=0; i<f.symbols.size(); i++) {
        cout << hexval0x(f.symbols[i]->val,8) << "  ";
        if (f.symbols[i]->len>0) {
            cout << hexval(f.symbols[i]->len,8);
        } else if (f.symbols[i]->align>0) {
            cout << hexval(f.symbols[i]->align,8);
        } else {
            cout << "        ";
        }
        cout << "  ";
        cout << "-";
        unsigned char stype=f.symbols[i]->type;
        cout << ((stype==STT_NOTYPE   )?"   ":"");
        cout << ((stype==STT_OBJECT   )?"Obj":"");
        cout << ((stype==STT_FUNC     )?"Fun":"");
        cout << ((stype==STT_SECTION  )?"Sec":"");
        cout << ((stype==STT_FILE     )?"Fil":"");
        cout << ((stype==STT_COMMON   )?"Com":"");
        cout << ((stype==STT_TLS      )?"TLD":"");
        cout << ((stype==STT_GNU_IFUNC)?"Gnu":"");
        cout << "-";
        cout << "  ";
        unsigned int shndx=f.symbols[i]->section;
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
        cout << "\t" << f.symbols[i]->name;
        cout << "\n";
    }

    return 0;
}
