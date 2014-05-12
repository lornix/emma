/* utils.cpp */

/*
 * collection of utility functions
 */

#include "utils.h"

/* strlen */
#include <string.h>
/* malloc */
#include <stdlib.h>
/* snprintf */
#include <stdio.h>

char* tohex(unsigned long int value,int length,const char* prefix)
{
    const char* prefixstr=prefix;
    if (prefix==0) {
        prefixstr="0x";
    }
    if (length==0) {
        if (value>0xffffffffl) {
            length=16;
        } else if (value>0xffff) {
            length=8;
        } else if (value>0xff) {
            length=4;
        } else {
            length=2;
        }
    }
    /* TODO:-1 (and kin) always gives FFFFFFFFF..., probably should mask value
     * to 2<<(length*8)-1
     */
    size_t length_needed=strlen(prefixstr)+length+2;
    char* retstr=(char*)malloc(length_needed);
    if (retstr!=0) {
        snprintf(retstr,length_needed,"%s%0*lx",prefix,length,value);
    }
    return retstr;
}
