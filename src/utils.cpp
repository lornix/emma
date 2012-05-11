/* utils.cpp */

/*
 * collection of utility functions
 */

#include "utils.h"

#include <sstream>

std::string tohex(unsigned long int value,int length,std::string prefix)
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
    std::ostringstream ss;
    if (prefix.length()>0) {
        ss << prefix;
    }
    ss << std::setfill('0') << std::setw(length) << std::hex << value << std::dec;
    return ss.str();
}
