/* emma - options.cpp */

#include "options.h"

options::options(int argc,const char* argv[])
{
    // variable setup
    verbose=false;
    filename="";
    //
    static struct option long_options[]={
        {"help"   ,no_argument,0,'h'},
        {"verbose",no_argument,0,'v'},
        {"version",no_argument,0,'V'},
    };
    static const char* short_options="hvV";

    int opt;
    int option_index=0;
    bool bad_option=false;
    while ((opt=getopt_long(
                    argc,
                    const_cast<char* const*>(argv),
                    short_options,
                    long_options,
                    &option_index))>=0) {
        switch (opt) {
            case 'h': /* help */
                help_usage(); exit(0);
            case 'v': /* verbose */
                verbose=true; break;
            case 'V': /* version */
                show_version(); exit(1);
            case '?':
            default: /* unknown */
                bad_option=true;
                break;
        }
    }
    if (bad_option) {
        // bad option
        exit(1);
    }
    // filename found?
    if (argc<=optind) {
        std::cerr << "Please provide a filename\n";
        exit(1);
    }
    filename=std::string(argv[optind]);
}

options::~options()
{
}

void options::show_version()
{
    std::cout << "emma - Version " << VERSION << "\n";
}

void options::help_usage()
{
    show_version();
    std::cout << "\n";
    std::cout << "usage: emma [option]... [file]\n";
    std::cout << "\n";
    std::cout << "-h, --help\tShow this help.\n";
    std::cout << "-v, --verbose\tBe verbose.\n";
    std::cout << "-V, --version\tShow version.\n";
}

