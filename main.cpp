/* emma - main.cpp */

#include <iostream>
// for exit
#include <cstdlib>

// #include <elf.h>
// #include <bfd.h>
// #include <dis-asm.h>

#include "emma.h"
#include "parsefile.h"

int main(int argc,const char* argv[])
{
    for (int i=1; i<argc; i++) {
        try {
            parsefile(std::string(argv[i]));
        }
        catch (parsefile::NotValidFile e) {
            // NIX - commented out to reduce clutter
            // std::cerr << e.what() << "\n";
        }
        catch (parsefile::CantOpenFile e) {
            // NIX - commented out to reduce clutter
            // std::cerr << e.what() << "\n";
        };
    }

    return 0;
}
