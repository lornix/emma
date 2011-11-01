/* emma - readelf32.cpp */

// for memcpy
#include <cstring>
// makes demangle NOT redefine basename
#define HAVE_DECL_BASENAME 1
#include <demangle.h>

#include "readelf32.h"
#include "utils.h"

readelf32::readelf32(std::string fname)
{
    // initialize to null
    fdata=0;

    std::cerr << "Reading '" << fname << "'\n";

    // save filename for later use
    filename=fname;

    try {
        // see if file exists
        struct stat statbuf;
        if (stat(fname.c_str(),&statbuf)) {
            throw std::string("File does not exist");
        }
        // yes, but is it a REAL file?
        if (!S_ISREG(statbuf.st_mode)) {
            throw std::string("Not a regular file");
        }
        // whew! attempt to open this file
        FILE* ifile=fopen(fname.c_str(),"rb");
        if (!ifile) {
            throw std::string("Unable to open file");
        }
        // create storage for ELF header
        Elf32_Ehdr elf_hdr;
        // attempt to read in ELF header
        if (fread(&elf_hdr,sizeof(elf_hdr),1,ifile)!=1) {
            throw std::string("Error reading ELF header");
        }
        if (memcmp(elf_hdr.e_ident,ELFMAG,SELFMAG)!=0) {
            throw std::string("Bad magic, not an ELF based file");
        }
        // this 'module' supports ELF32 i386, make sure that's what we have
        if (elf_hdr.e_ident[EI_CLASS]!=ELFCLASS32) {
            throw std::string("I only understand i386 32-bit");
        }
        // look for little-endian structure
        if (elf_hdr.e_ident[EI_DATA]!=ELFDATA2LSB) {
            throw std::string("I only know little-endian");
        }
        // is ELF 'version' current?
        if (elf_hdr.e_ident[EI_VERSION]!=EV_CURRENT) {
            throw std::string("ELF version not current");
        }
        // OSABI = linux or gnu?
        if ( (elf_hdr.e_ident[EI_OSABI]!=ELFOSABI_GNU)      &&
                (elf_hdr.e_ident[EI_OSABI]!=ELFOSABI_LINUX) &&
                (elf_hdr.e_ident[EI_OSABI]!=ELFOSABI_SYSV)  &&
                (elf_hdr.e_ident[EI_OSABI]!=ELFOSABI_NONE)  ) {
            throw std::string("OSABI not GNU/LINUX/SYSV/NONE");
        }

        // header is valid, pull data from file

        // save the starting point of program
        exec_addr=elf_hdr.e_entry;
        proc_flags=elf_hdr.e_flags;

        // get size in bytes
        fsize=statbuf.st_size;
        // reserve space
        fdata=new char[fsize];
        // rewind to start of file again
        if (fseek(ifile,0,SEEK_SET)) {
            throw std::string("Can't rewind file");
        }
        // read in whole file
        if (fread(fdata,fsize,1,ifile)!=1) {
            throw std::string("Error reading file");
        }
        // done with file
        fclose(ifile);

        // set up all the section header pointers
        // section[0] is special, containing additional info for phnum and shnum
        sec_headers.push_back(reinterpret_cast<Elf32_Shdr*>(fdata+elf_hdr.e_shoff));
        //
        unsigned int shcount=elf_hdr.e_shnum;
        if (shcount==0) {
            shcount=sec_headers[0]->sh_size;
        }
        // already set up [0]
        for (unsigned int i=1; i<shcount; i++) {
            Elf32_Shdr* ptr=reinterpret_cast<Elf32_Shdr*>(fdata+elf_hdr.e_shoff+(i*elf_hdr.e_shentsize));
            sec_headers.push_back(ptr);
        }
        //
        unsigned int phcount=elf_hdr.e_phnum;
        if (elf_hdr.e_phnum==PN_XNUM) { // header count > 0xffff
            phcount=sec_headers[0]->sh_info;
        }
        // set up all the program header pointers
        for (unsigned int i=0; i<phcount; i++) {
            Elf32_Phdr* ptr=reinterpret_cast<Elf32_Phdr*>(fdata+elf_hdr.e_phoff+(i*elf_hdr.e_phentsize));
            prg_headers.push_back(ptr);
        }
        // set up section name string table
        sec_name_table=elf_hdr.e_shstrndx;
        if (sec_name_table!=SHN_UNDEF) {
            if (sec_name_table==SHN_XINDEX) {
                sec_name_table=sec_headers[0]->sh_link;
            }
        }

        show_sections();

        parse_symbols();

    } catch (std::string e) {
        if (errno!=0) {
            // if error was from function call?
            perror(e.c_str());
        } else {
            // nope, just display the error
            std::cerr << "Error: " << e << "\n";
        }
        // exit program (one of few exit points)
        exit(1);
    }
}

readelf32::~readelf32()
{
    if (fdata)
        delete[] fdata;
    if (!prg_headers.empty())
        prg_headers.clear();
    if (!sec_headers.empty())
        sec_headers.clear();
    if (!sec_names.empty())
        sec_names.clear();
    while (!symbols.empty()) {
        delete symbols.back();
        symbols.pop_back();
    }
    // sec_headers and prg_headers are destroyed when
    // the class is destroyed.
}

const char* readelf32::sec_name(unsigned int section_num)
{
    // return const char* based on sh_name offset into
    // str_table_index section
    const char* ptr=fdata;
    if (section_num>sec_headers.size()) {
        ptr=NULL;
    } else {
        ptr+=sec_headers[sec_name_table]->sh_offset;
        ptr+=sec_headers[section_num]->sh_name;
    }
    return ptr;
}

unsigned int readelf32::find_section_name(std::string name)
{
    // return section number of section with 'name'
    for (unsigned int i=1; i<sec_headers.size(); i++) {
        if (std::string(sec_name(i))==name) {
            return i;
        }
    }
    // not found, return empty section (0)
    return 0;
}

std::string readelf32::show_sec_flags(unsigned int section_num)
{
    std::string retval;
    unsigned int flags=sec_headers[section_num]->sh_flags;
    if (flags&SHF_WRITE) retval+=",Write";
    if (flags&SHF_ALLOC) retval+=",Alloc";
    if (flags&SHF_EXECINSTR) retval+=",Exec";
    if (flags&SHF_MASKPROC) retval+=",Rsvd";
    if (flags&SHF_MERGE) retval+=",Merge";
    if (flags&SHF_STRINGS) retval+=",Strings";
    if (flags&SHF_INFO_LINK) retval+=",Info_Link";
    if (flags&SHF_LINK_ORDER) retval+=",Link_Order";
    if (flags&SHF_OS_NONCONFORMING) retval+=",OS_Noncon";
    if (flags&SHF_GROUP) retval+=",Group";
    if (flags&SHF_TLS) retval+=",Thread_Local_Data";
    if (flags&SHF_ORDERED) retval+=",Ordered";
    if (flags&SHF_EXCLUDE) retval+=",Exclude";
    if (retval.length()>0) {
        retval=" ("+retval.substr(1)+")";
    }
    return retval;
}
std::string readelf32::show_sec_type(unsigned int section_num)
{
    std::string retval;
    unsigned int types=sec_headers[section_num]->sh_type;
    switch (types) {
        case SHT_NULL:
            retval="Null"; break;
        case SHT_PROGBITS:
            retval="Program_Data"; break;
        case SHT_SYMTAB:
            retval="Symbol_Table"; break;
        case SHT_STRTAB:
            retval="String_Table"; break;
        case SHT_RELA:
            retval="Relocation_Entries_(addends)"; break;
        case SHT_HASH:
            retval="Symbol_Hash_Table"; break;
        case SHT_DYNAMIC:
            retval="Dynamic_Linking"; break;
        case SHT_NOTE:
            retval="Notes"; break;
        case SHT_NOBITS:
            retval="NoBits"; break;
        case SHT_REL:
            retval="Relocation_Entries_(no addends)"; break;
        case SHT_SHLIB:
            retval="Reserved_(SHLIB)"; break;
        case SHT_DYNSYM:
            retval="Dynamic_Linker_Symbol_Table"; break;
        case SHT_INIT_ARRAY:
            retval="Array_of_Constructors"; break;
        case SHT_FINI_ARRAY:
            retval="Array_of_Destructors"; break;
        case SHT_PREINIT_ARRAY:
            retval="Array_of_Pre-Constructors"; break;
        case SHT_GROUP:
            retval="Section_Group"; break;
        case SHT_SYMTAB_SHNDX:
            retval="Extended_Section_Indices"; break;
        case SHT_GNU_ATTRIBUTES:
            retval="Object_Attributes"; break;
        case SHT_GNU_HASH:
            retval="GNU-style_Hash_Table"; break;
        case SHT_GNU_LIBLIST:
            retval="Prelink_Library_List"; break;
        case SHT_CHECKSUM:
            retval="Checksum_for_DSO_Content"; break;
        case SHT_GNU_verdef:
            retval="Version_Definition_Section"; break;
        case SHT_GNU_verneed:
            retval="Version_Needs_Section"; break;
        case SHT_GNU_versym:
            retval="Version_Symbol_Table"; break;
        default:
            retval="Not_a_clue: "+types; break;
    }
    return retval;
}
std::string readelf32::show_prg_flags(unsigned int prg_section)
{
    std::string retval;
    unsigned int flags=prg_headers[prg_section]->p_flags;
    if (flags&PF_R) retval+="R";
    if (flags&PF_W) retval+="W";
    if (flags&PF_X) retval+="X";
    if (retval.length()>0) {
        retval=" ("+retval+")";
    }
    return retval;
}
std::string readelf32::show_prg_type(unsigned int prg_section)
{
    std::string retval;
    unsigned int types=prg_headers[prg_section]->p_type;
    switch (types) {
        case PT_NULL:
            retval="Program_Header_Table_Entry"; break;
        case PT_LOAD:
            retval="Loadable_Program_Segment"; break;
        case PT_DYNAMIC:
            retval="Dynamic_Linking_Information"; break;
        case PT_INTERP:
            retval="Program_Interpreter"; break;
        case PT_NOTE:
            retval="Auxiliary_Information"; break;
        case PT_SHLIB:
            retval="Reserved_(SHLIB)"; break;
        case PT_PHDR:
            retval="Entry_for_header_table_itself"; break;
        case PT_TLS:
            retval="Thread-local_storage_segment"; break;
        case PT_GNU_EH_FRAME:
            retval="GCC_.eh_frame_hdr_segment"; break;
        case PT_GNU_STACK:
            retval="Stack_executable"; break;
        case PT_GNU_RELRO:
            retval="Read-Only_after_relocation"; break;
        default:
            retval="Not_a_clue:_"+types; break;
    }
    return retval;
}

unsigned int readelf32::scan_symbol_table(unsigned int symtab)
{
    // get name of current symbol table
    std::string tabname=sec_name(symtab);
    // do I need to also look for UPPERCASE versions?
    unsigned int place=tabname.find("sym");
    if (place==std::string::npos) {
        // 'sym' not found in string
        throw("Unable to find sym in section name");
    }
    tabname=tabname.replace(place,3,"str");
    unsigned int strtab=find_section_name(tabname);
    if (strtab==0) {
        // couldn't find strtab for symtab
        throw("Unable to find str tab for sym tab");
    }
    // get initial count of symbols
    unsigned int delta=symbols.size();
    // skip dummy record 0
    for (unsigned int i=sizeof(Elf32_Sym);
            i<sec_headers[symtab]->sh_size;
            i+=sizeof(Elf32_Sym)) {
        Elf32_Sym* sym=reinterpret_cast<Elf32_Sym*>
            (fdata+sec_headers[symtab]->sh_offset+i);
        syms_s* tmpsym=new syms_s;
        tmpsym->val=sym->st_value;
        tmpsym->len=0;
        tmpsym->align=0;
        tmpsym->scope=ELF32_ST_BIND(sym->st_info);
        tmpsym->type=ELF32_ST_TYPE(sym->st_info);
        tmpsym->section=sym->st_shndx;

        // determine WHICH common tag to watch
        // st_shndx (SHN_COMMON) or st_info (STT_COMMON)
        tmpsym->len=sym->st_size; // for functions, length
        tmpsym->align=sym->st_size; // for objects, alignment

        tmpsym->mangled=std::string(fdata+sec_headers[strtab]->sh_offset+sym->st_name);
        char* tstr=cplus_demangle(tmpsym->mangled.c_str(),DMGL_PARAMS);
        if (tstr==NULL) {
            tmpsym->name=tmpsym->mangled;
        } else {
            tmpsym->name=std::string(tstr);
            // release memory allocated by cplus_demangle
            free(tstr);
        }
        symbols.push_back(tmpsym);
    }
    delta=symbols.size()-delta;
    return delta;
}
unsigned int readelf32::parse_symbols()
{
    std::cout << "Entry Point: " << hexval0x(exec_addr,8) << "\n\n";
    std::cout << "Symbols (";
    unsigned int section=0;
    unsigned int total=0;
    while (section<sec_headers.size()) {
        if ((sec_headers[section]->sh_type==SHT_SYMTAB)||
                (sec_headers[section]->sh_type==SHT_DYNSYM)) {
            if (total>0) std::cout << "+";
            total=scan_symbol_table(section);
            std::cout << total;
        }
        section++;
    }
    std::cout << ")\n";
    std::cout << "=================\n";
    // remove duplicates
    // dynsym always subset of symtab (if symtab present)
    bool changed=true;
    while (changed) {
        changed=false;
        for (std::vector<syms_s*>::iterator i=symbols.begin(); i<symbols.end(); i++) {
            for (std::vector<syms_s*>::iterator j=symbols.begin(); j<symbols.end(); j++) {
                if (i==j)
                    continue;
                if (((*i)->val    ==(*j)->val)&&
                        ((*i)->len    ==(*j)->len)&&
                        ((*i)->scope  ==(*j)->scope)&&
                        ((*i)->type   ==(*j)->type)&&
                        ((*i)->section==(*j)->section)&&
                        ((*i)->name   ==(*j)->name)
                   ) {
                    symbols.erase(i);
                    changed=true;
                    break;
                }
            }
        }
    }

    return symbols.size();
}

void readelf32::show_sections()
{
    //
    // show program sections
    std::cout << "Program Sections (" << prg_headers.size() << ")\n";
    std::cout << "=================\n";
    for (unsigned int i=0; i<prg_headers.size(); i++) {
        std::cout.width(3);
        std::cout << i << ": ";
        std::cout << hexval0x(prg_headers[i]->p_type) << " " << show_prg_type(i);
        std::cout << show_prg_flags(i) << "\n";
        std::cout << "Off: " << hexval0x(prg_headers[i]->p_offset,8);
        std::cout << " VMA: " << hexval0x(prg_headers[i]->p_vaddr,8);
        std::cout << " PMA: " << hexval0x(prg_headers[i]->p_paddr,8);
        std::cout << " Pfs: " << hexval0x(prg_headers[i]->p_filesz);
        std::cout << " Pms: " << hexval0x(prg_headers[i]->p_memsz);
        for (unsigned int j=1; j<sec_headers.size(); j++) {
            if (sec_headers[j]->sh_offset>=prg_headers[i]->p_offset) {
                if (sec_headers[j]->sh_offset<(prg_headers[i]->p_offset+prg_headers[i]->p_memsz)) {
                    std::cout << " (" << sec_name(j) << "=" << hexval0x(sec_headers[j]->sh_offset) << ")";
                }
            }
        }
        std::cout <<"\n\n";
    }
    std::cout << "\n";
    //
    // show sections
    unsigned int namelen=1;
    for (unsigned int i=0; i<sec_headers.size(); i++) {
        if (std::string(sec_name(i)).length()>namelen)
            namelen=std::string(sec_name(i)).length();
    }
    std::cout << "Sections (" << sec_headers.size() << "-1)\n";
    std::cout << "=================\n";
    for (unsigned int i=1; i<sec_headers.size(); i++) {
        std::cout.width(3);
        std::cout << i << ": ";
        std::cout.width(namelen);
        std::cout << std::right;
        std::cout << sec_name(i) << " ";
        std::cout << "Off: " << hexval0x(sec_headers[i]->sh_offset,8) << " ";
        std::cout << "VMA: " << hexval0x(sec_headers[i]->sh_addr,8) << " ";
        std::cout << "Sz: " << hexval0x(sec_headers[i]->sh_size,8) << " ";
        std::cout << "T: " << hexval0x(sec_headers[i]->sh_type,8) << " (" << show_sec_type(i) << ") ";
        std::cout << show_sec_flags(i);
        std::cout << "\n";
    }
    std::cout << "\n";
}
