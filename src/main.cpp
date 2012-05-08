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
    parsefile(std::string(argv[1]));

    return 0;
}
