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
#include "parsefile.h"

int main(int argc __attribute__((unused)),const char* argv[])
{
    for (int i=1; i<argc; i++) {
        try {
            parsefile(std::string(argv[i]));
        }
        catch (parsefile::NotValidFile e) {
            std::cerr << e.what() << "\n";
        }
        catch (parsefile::CantOpenFile e) {
            std::cerr << e.what() << "\n";
        };
    }

    return 0;
}
