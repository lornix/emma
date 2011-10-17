/* emma - utils.h */

#ifndef UTILS_H
#define UTILS_H

#include <string>

#include "../config.h"

#include "emma.h"

std::string hexval(unsigned long val,unsigned int len=2);
std::string hexval0x(unsigned long val,unsigned int len=2);

#endif
