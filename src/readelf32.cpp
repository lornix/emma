#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>

#include <elf.h>
#include <dis-asm.h>

#include "../config.h"

#include "emma.h"
#include "readelf32.h"

using namespace std;

readelf32::readelf32(string fname)
{
    cout << "readelf32: " << fname << "\n";
}

readelf32::~readelf32()
{
}
