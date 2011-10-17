/* emma - options.h */

#ifndef OPTIONS_H
#define OPTIONS_H

#include <iostream>

// for getopt_long
#include <unistd.h>
#include <getopt.h>
// for exit
#include <cstdlib>

#include "../config.h"

class options
{
 public:
    options(int argc,const char* argv[]);
    ~options();
 public:
    bool verbose;
    std::string filename;
 private:
    void help_usage();
    void show_version();
    void parse_options(unsigned int argc,const char* argv[]);
};

#endif
