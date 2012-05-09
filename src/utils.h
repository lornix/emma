/* utils.h */

/*
 * header file for collection of utility functions
 */

#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <iomanip>

#include "../config.h"
#include "emma.h"

void tohex(unsigned long int value,int length=0,std::string prefix="0x");

#endif