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
#include "utils.h"

int main(int argc,const char* argv[])
{
    // parse options, does not return if no filename given
    options opts(argc,&argv[0]);

    return 0;
}
