#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>

#include <bfd.h>
#include <dis-asm.h>

#include "../config.h"

#include "emma.h"
#include "read_elf.h"

using namespace std;

void help_usage()
{
    cout << "usage: emma [option]... [file]\n";
    cout << "\n";
    cout << "-h, --help\tShow this help.\n";
    cout << "-v, --verbose\tBe verbose.\n";
    exit(0);
}

int main(int argc,char* argv[])
{
    static struct option long_options[]={
        {"help"   ,no_argument,0,'h'},
    };

    int opt;
    int option_index=0;
    while ((opt=getopt_long(argc,argv,"vh",
                    long_options,&option_index))>=0) {
        switch (opt) {
            case 'h': /* help */
                help_usage(); break; /* never returns */
            case '?':
            default: /* unknown */
                break;
        }

    }

    // are we out of parameters?
    if (argc<=optind) {
        cerr << "Please provide a filename to process\n";
        return 1;
    }
    // no, see if file exists
    struct stat statbuf;
    if (stat(argv[optind],&statbuf)) {
        perror("Unable to open file");
        return 1;
    }
    // but is it a REAL file?
    if (!S_ISREG(statbuf.st_mode)) {
        cerr << "Not a regular file\n";
        return 1;
    }

    cout << "loading sections here\n";
    // new sections(argv[optind]);

    // load_sections(argv[optind]);

    cout << "showing sections here\n";
    // show_sections();

    return 0;
}
