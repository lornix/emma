#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>

#include <elf.h>
#include <bfd.h>
#include <dis-asm.h>

#include "../config.h"

#include "emma.h"
#include "read_elf.h"
#include "readelf32.h"

using namespace std;

string show_sec_flags(int flags)
{
#define ssf(x) if (flags&SEC_ ## x) retval+=" " #x;
    string retval;
    // if (flags==SEC_NO_FLAGS) retval+=" None";
    ssf(ALLOC);
    ssf(LOAD);
    ssf(RELOC);
    ssf(READONLY);
    ssf(CODE);
    ssf(DATA);
    ssf(ROM);
    ssf(CONSTRUCTOR);
    ssf(HAS_CONTENTS);
    ssf(NEVER_LOAD);
    ssf(THREAD_LOCAL);
    ssf(HAS_GOT_REF);
    ssf(IS_COMMON);
    ssf(DEBUGGING);
    ssf(IN_MEMORY);
    ssf(EXCLUDE);
    ssf(SORT_ENTRIES);
    ssf(LINK_ONCE);
    ssf(LINK_DUPLICATES);
    ssf(LINK_DUPLICATES_DISCARD);
    ssf(LINK_DUPLICATES_ONE_ONLY);
    ssf(LINK_DUPLICATES_SAME_SIZE);
    ssf(LINKER_CREATED);
    ssf(KEEP);
    ssf(SMALL_DATA);
    ssf(MERGE);
    ssf(STRINGS);
    ssf(GROUP);
    ssf(COFF_SHARED_LIBRARY);
    ssf(ELF_REVERSE_COPY);
    ssf(COFF_SHARED);
    ssf(TIC54X_BLOCK);
    ssf(TIC54X_CLINK);
    ssf(COFF_NOREAD);

    retval+="\n";
    return retval;
}

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

    read_elf elf(string(argv[optind]));

    for (unsigned int i=0; i<elf.size(); i++) {
        const read_elf::filedata_s* fd;
        fd=elf[i];
        if (fd->a.flags&SEC_DEBUGGING) continue;
        cout << fd->a.index << ": " << fd->a.name << "\n";
        cout << "  VMA: 0x" << hex << fd->a.vma << dec << "\n";
        cout << " Size: 0x" << hex << fd->a.size << dec << "\n";
        cout << "Flags: 0x" << hex << fd->a.flags << dec << show_sec_flags(fd->a.flags);
        cout << "\n";
    }
    cout << "\n";
    // show debugging sections
    for (unsigned int i=0; i<elf.size(); i++) {
        const read_elf::filedata_s* fd;
        fd=elf[i];
        if (!(fd->a.flags&SEC_DEBUGGING)) continue;
        cout << fd->a.index << ": " << fd->a.name << "\n";
        cout << "  VMA: 0x" << hex << fd->a.vma << dec << "\n";
        cout << " Size: 0x" << hex << fd->a.size << dec << "\n";
        cout << "Flags: 0x" << hex << fd->a.flags << dec << show_sec_flags(fd->a.flags);
        cout << "\n";
    }

    return 0;
}
