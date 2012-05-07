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

int main(int argc,const char* argv[])
{
    for (int i=0; i<argc; i++) {
        cout << i << "\t" << argv[i] << "\n";
    }
    parsefile(std::string(argv[1]));

    return 0;
}
