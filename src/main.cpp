#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>

#include <bfd.h>
#include <dis-asm.h>

#include "../config.h"

#include "emma.h"
#include "read_file.h"

using namespace std;

int main(int argc,char* argv[])
{
    if (argc<2) {
        cerr << "Need a filename\n";
        return 1;
    }

    load_sections(argv[1]);

    show_sections();

    return 0;
}
