#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <unistd.h>
#include <getopt.h>

#include <bfd.h>
#include <dis-asm.h>

#include "../config.h"

#include "emma.h"
#include "read_file.h"

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
    int verbose_flag=0;
    int option_index=0;
    static struct option long_options[]={
        {"verbose",no_argument,0,'v'},
        {"help"   ,no_argument,0,'h'},
    };

    int opt;
    while ((opt=getopt_long(argc,argv,"vh",
                    long_options,&option_index))>=0) {
        switch (opt) {
            case 'v': /* verbose */
                verbose_flag=1; break;
            case 'h': /* help */
                help_usage(); break; /* never returns */
            case '?':
            default: /* unknown */
                break;
        }

    }

    cout << "Verbose=" << verbose_flag << "\n";
    cout << "argc=" << argc << "\n";
    cout << "optind=" << optind << "\n";
    cout << "argv[optind]=" << argv[optind] << "\n";
    cout << "\n";

    if (argc<=optind) {
        cerr << "Please provide a filename to process\n";
        return 1;
    }

    load_sections(argv[optind]);

    cout << "\nshowing sections here\n";
    // show_sections();

    return 0;
}
