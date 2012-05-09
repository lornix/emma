/* utils.cpp */

/*
 * collection of utility functions
 */

#include "utils.h"

void tohex(unsigned long int value,int length,std::string prefix)
{
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
    if (prefix.length()>0) {
        std::cout << prefix;
    }
    std::cout << std::setfill('0') << std::setw(length) << std::hex << value << std::dec;
}
