/* emma - utils.cpp */

#include "utils.h"

std::string hexval(unsigned long val,unsigned int len)
{
    // force len into 2/4/8 sizes (byte/word/long)
    if (len<2) len=2;
    if (len==3) len=4;
    if (len>4) len=8;
    // check size of val against len, expand len if necessary
    if ((val>0xff)&&(len<4)) len=4;   //     0x0100 -     0xffff
    if ((val>0xffff)&&(len<8)) len=8; // 0x00010000 - 0xffffffff
    // build the string
    std::string retval;
    for (unsigned int i=0; i<len; i++) {
        // use nybble as offset into array
        retval="0123456789abcdef"[val&0xf]+retval;
        // shift nybble out
        val>>=4;
    }
    return retval;
}
std::string hexval0x(unsigned long val,unsigned int len)
{
    return "0x"+hexval(val,len);
}
