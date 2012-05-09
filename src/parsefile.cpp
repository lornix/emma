/* parsefile.cpp */

/*
 * file for the file parsing class, it populates arrays of sections and their
 * contents, symbols and their values, file information, start address,
 * whatever else I can obtain from the bfd backend for ELF/PE/a.out... type
 * files.  Eventually to have support for raw data blocks too
 */

#include "parsefile.h"
#include "utils.h"

parsefile::parsefile(std::string fname)
{
    bfd* abfd;
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
    startaddress=abfd->start_address;
    std::cout << "Start Address: 0x" << std::hex << startaddress << std::dec << "\n";

    load_sections(abfd);
    load_symbols(abfd);

    /* all done, close file */
    bfd_close(abfd);

    std::cout << "\n";
    std::vector<section_t>::iterator ptr1=sections.begin();
    while (ptr1!=sections.end()) {
        tohex(ptr1->vma_start,8,"");
        std::cout << " ";
        tohex(ptr1->length,8,"");
        std::cout << " " << ptr1->name << "\n";
        ptr1++;
    }
    std::cout << "\n";
    std::vector<symbol_t>::iterator ptr2=symbols.begin();
    while (ptr2!=symbols.end()) {
        tohex(ptr2->value,8,"");
        std::cout << " " << (char)ptr2->type << " " << ptr2->name << "\n";
        ptr2++;
    }
}

parsefile::~parsefile()
{
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
        /* empty the symbols vector */
        symbols.clear();
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
        savesection.vma_start=sec->vma;
        savesection.length=sec->size;
        /* compute end of usage.  (start,end] */
        savesection.vma_end=sec->vma+sec->size+1;
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
void parsefile::load_symbols(bfd* abfd)
{
    /* ask how big storage for symbols needs to be */
    long int datasize=bfd_get_symtab_upper_bound(abfd);
    /* negative return value indicates no symbols */

    if (datasize>0) {
        /* allocate memory to hold symbols */
        asymbol** symtable=(asymbol**)malloc(datasize);

        if (!symtable) {
            /* couldn't allocate memory? very odd */
            throw OutOfMemory(std::string("Error allocating memory for symtable"));
        }

        long numsymbols=bfd_canonicalize_symtab(abfd,symtable);
        if (numsymbols<0) {
            throw GeneralError(std::string("Error while canonicalizing symbols"));
        }

        /* process regular symbols */
        for (long i=0; i<numsymbols; i++) {
            symbol_t sym;

            sym.name=demangle(abfd,symtable[i]->name);
            sym.value=symtable[i]->value;
            sym.flags=symtable[i]->flags;
            sym.type=bfd_decode_symclass(symtable[i]);

            symbols.push_back(sym);
        }
        free(symtable);
    }

    long int dyndatasize=bfd_get_dynamic_symtab_upper_bound(abfd);
    /* negative return value indicates no symbols */

    if (dyndatasize>0) {
        /* allocate memory to hold symbols */
        asymbol** dynsymtable=(asymbol**)malloc(dyndatasize);

        if (!dynsymtable) {
            /* couldn't allocate memory? very odd */
            throw OutOfMemory(std::string("Error allocating memory for dynamic symtable"));
        }

        long dynnumsymbols=bfd_canonicalize_dynamic_symtab(abfd,dynsymtable);
        /* process dynamic symbols */
        for (long i=0; i<dynnumsymbols; i++) {
            symbol_t dynsym;

            dynsym.name=demangle(abfd,dynsymtable[i]->name);
            dynsym.value=dynsymtable[i]->value;
            dynsym.flags=dynsymtable[i]->flags;
            dynsym.type=bfd_decode_symclass(dynsymtable[i]);

            symbols.push_back(dynsym);
        }
        free(dynsymtable);
    }

    std::cout << "Loaded " << symbols.size() << " symbols.\n";
}
std::string parsefile::demangle(bfd* abfd,const char* name)
{
    std::string retval;
    char* str=bfd_demangle(abfd,name,0);
    if (str) {
        retval=std::string(str);
        /* bfd malloc's mem for demangling */
        free(str);
    } else {
        retval=std::string(name);
    }
    return retval;
}
