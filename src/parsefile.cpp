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
    load_sections(abfd);
}

parsefile::~parsefile()
{
    if (abfd) {
        if (!sections.empty()) {
            /* empty the section vector */
            std::vector<section_t>::iterator ptr=sections.begin();
            while (ptr!=sections.end()) {
                /* sigh, bfd used malloc to create chunk for contents */
                if (ptr->contents) {
                    free(ptr->contents);
                }
                ptr++;
            }
            sections.clear();
        }
        if (!symbols.empty()) {
            symbols.clear();
        }
        bfd_close(abfd);
        abfd=NULL;
    }
}
void parsefile::load_sections(bfd* abfd)
{
    /* I couldn't determine a clean method to use bfd_map_over... */
    /* load the sections, follow the linked list built by bfd */
    bfd_section* sec=abfd->sections;
    while (sec) {
        section_t savesection;

        savesection.name=std::string(sec->name);
        savesection.vma=sec->vma;
        savesection.length=sec->size;
        savesection.alignment=sec->alignment_power;
        /* TODO determine flags used */
        savesection.flags=sec->flags;
        savesection.contents=NULL;

        unsigned long int datasize=bfd_get_section_size(sec);
        if (datasize) {
            bfd_malloc_and_get_section(abfd,sec,&(savesection.contents));
        }
        sections.push_back(savesection);
        sec=sec->next;
    }
    std::cout << "Loaded " << sections.size() << " sections.\n";
}
