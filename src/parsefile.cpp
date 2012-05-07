/* parsefile.cpp */

/*
 * file for the file parsing class, it populates arrays of sections and their
 * contents, symbols and their values, file information, start address,
 * whatever else I can obtain from the bfd backend for ELF/PE/a.out... type
 * files.  Eventually to have support for raw data blocks too
 */

#include "parsefile.h"

parsefile::parsefile(std::string fname)
{
    bfd_init();
    bfd_set_default_target("default");
    abfd=bfd_openr(fname.c_str(),NULL);
    if (!abfd) {
        throw CantOpenFile(std::string("Unable to open file: ")+fname);
    }
    /* make sure file is a known object type, maybe add code
       to parse bfd_archive and bfd_core, along with raw data */
    if (!bfd_check_format(abfd,bfd_object)) {
        bfd_close(abfd);
        abfd=NULL;
        throw NotValidFile(std::string("Unknown file type: ")+fname);
    }
}
parsefile::~parsefile()
{
    if (abfd) {
        bfd_close(abfd);
        abfd=NULL;
    }
}
