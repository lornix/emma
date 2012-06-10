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
        throw CantOpenFile("Unable to open file: "+fname);
    }
    /* make sure file is a known object type, maybe add code
       to parse bfd_archive and bfd_core, along with raw data */
    if (!bfd_check_format(abfd,bfd_object)) {
        bfd_close(abfd);
        throw NotValidFile("Unknown/Not Handled File Type: "+fname);
    }

    filename=fname;

    arch=bfd_get_arch(abfd);
    mach=bfd_get_mach(abfd);
    machstr=std::string(bfd_printable_arch_mach(arch,mach));
    fileflags=bfd_get_file_flags(abfd);
    switch (arch) {
        case bfd_arch_i386:

            flag_has_reloc=fileflags&HAS_RELOC;
            flag_has_exec=fileflags&EXEC_P;
            flag_has_linenums=fileflags&HAS_LINENO;
            flag_has_debug=fileflags&HAS_DEBUG;
            flag_has_symbols=fileflags&HAS_SYMS;
            flag_has_locals=fileflags&HAS_LOCALS;
            flag_has_dynamic=fileflags&DYNAMIC;
            flag_is_relaxable=fileflags&BFD_IS_RELAXABLE;

            startaddress=abfd->start_address;
            elf_load_sections(abfd);
            elf_load_symbols(abfd);

            break;
        default:
            bfd_close(abfd);
            throw NotValidFile("Unknown/Unhandled Architecture: "+machstr);
    }

    flavor=bfd_get_flavour(abfd);
    std::ostringstream tmpstr;
    switch (flavor) {
        case bfd_target_elf_flavour:
            tmpstr << "ELF";
            break;
        default:
            tmpstr << "#" << flavor << " - FIXME";
    }
    if (flag_has_exec) {
        tmpstr << " (exec)";
    } else {
        tmpstr << " (lib)";
    }
    flavorstr=tmpstr.str();
    filetypestr=std::string(bfd_get_target(abfd));
    std::cout << "File: " << filename << "\n";
    std::cout << "Filetype: " << filetypestr << "\n";
    std::cout << "Flavor: " << flavorstr << "\n";
    std::cout << "Architecture: " << machstr << "\n";
    std::cout << "Start Address: " << tohex(startaddress,0) << "\n";
    std::cout << "Loaded " << sections.size() << " sections.\n";
    std::cout << "Loaded " << symbols.size() << " symbols.\n";
    std::cout << "\n";

    std::vector<section_t>::iterator ptr1=sections.begin();
    while (ptr1!=sections.end()) {
        std::cout << tohex(ptr1->vma_start,8,"") << " " << tohex(ptr1->length,8,"") << " " << ptr1->name << "\n";
        ptr1++;
    }
    std::cout << "\n";
    std::vector<symbol_t>::iterator ptr2=symbols.begin();
    while (ptr2!=symbols.end()) {
        std::cout << tohex(ptr2->value,8,"") << " " << (char)ptr2->type << " " << ptr2->name << "\n";
        ptr2++;
    }
    std::cout << "\n";

    /* all done, close file */
    bfd_close(abfd);
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
void parsefile::elf_load_sections(bfd* abfd)
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
}
void parsefile::elf_load_symbols(bfd* abfd)
{
    /* ask how big storage for symbols needs to be */
    long int datasize=bfd_get_symtab_upper_bound(abfd);
    /* negative return value indicates no symbols */

    if (datasize>0) {
        /* allocate memory to hold symbols */
        asymbol** symtable=(asymbol**)malloc(datasize);

        if (!symtable) {
            /* couldn't allocate memory? very odd */
            throw OutOfMemory("Error allocating memory for symtable");
        }

        long numsymbols=bfd_canonicalize_symtab(abfd,symtable);
        if (numsymbols<0) {
            throw GeneralError("Error while canonicalizing symbols");
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
            throw OutOfMemory("Error allocating memory for dynamic symtable");
        }

        long dynnumsymbols=bfd_canonicalize_dynamic_symtab(abfd,dynsymtable);
        if (dynnumsymbols<0) {
            throw GeneralError("Error while canonicalizing dynamic symbols");
        }
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
