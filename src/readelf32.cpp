/* emma - readelf32.cpp */

#include "readelf32.h"

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
        fdata=new unsigned char[fsize];
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
    if (fdata) {
        delete fdata;
    }
    // sec_headers and prg_headers are destroyed when
    // the class is destroyed.
}

const unsigned char* readelf32::sec_name(unsigned int section_num)
{
    // return const char* based on sh_name offset into
    // str_table_index section
    const unsigned char* ptr=fdata;
    ptr+=sec_headers[sec_name_table]->sh_offset;
    ptr+=sec_headers[section_num]->sh_name;
    return ptr;
}

std::string readelf32::show_sec_flags(unsigned int section_num)
{
    std::string retval;
    unsigned int flags=sec_headers[section_num]->sh_flags;
    if (flags&SHF_WRITE) retval+=" Write";
    if (flags&SHF_ALLOC) retval+=" Alloc";
    if (flags&SHF_EXECINSTR) retval+=" Exec";
    if (flags&SHF_MASKPROC) retval+=" Rsvd";
    if (flags&SHF_MERGE) retval+=" Merge";
    if (flags&SHF_STRINGS) retval+=" Strings";
    if (flags&SHF_INFO_LINK) retval+=" Info_Link";
    if (flags&SHF_LINK_ORDER) retval+=" Link_Order";
    if (flags&SHF_OS_NONCONFORMING) retval+=" OS_Noncon";
    if (flags&SHF_GROUP) retval+=" Group";
    if (flags&SHF_TLS) retval+=" Thread_Local_Data";
    if (flags&SHF_ORDERED) retval+=" Ordered";
    if (flags&SHF_EXCLUDE) retval+=" Exclude";
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
    return " "+retval;
}
std::string readelf32::show_prg_flags(unsigned int prg_section)
{
    std::string retval;
    unsigned int flags=prg_headers[prg_section]->p_flags;
    if (flags&PF_R) retval+=" R";
    if (flags&PF_W) retval+=" W";
    if (flags&PF_X) retval+=" X";
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
    return " "+retval;
}
