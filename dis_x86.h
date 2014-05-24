/* dis_x86.h */

#ifndef DIS_X86_H
#define DIS_X86_H

#include "parsefile.h"

enum {
    BITS_UNK=0,
    BITS16,
    BITS32,
    BITS64,
    BITS_MAX
};

void dis_x86(emma_handle* H,section_t* section);

#endif
