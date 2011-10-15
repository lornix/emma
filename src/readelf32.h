/* emma - readelf32.h */

#ifndef READELF32_H
#define READELF32_H

class readelf32
{
 public:
    //
    struct filedata_s {
        asection a;
        bfd_byte* b;
    };
    //
    readelf32(const char* fname);
    readelf32(std::string fname);
    ~readelf32();
    const unsigned char* sec_name(unsigned int section_num);
    std::string show_sec_flags(unsigned int section_num);
    std::string show_sec_type(unsigned int section_num);
 private:
    std::string filename;
    // information from the elf_hdr
    unsigned int exec_addr;
    unsigned int proc_flags;
    unsigned int sec_name_table;
    //
    unsigned long int fsize;
 public:
    // points to array holding file data
    unsigned char* fdata;
    // pointers into the fdata array
    std::vector<Elf32_Phdr*> prg_headers;
    std::vector<Elf32_Shdr*> sec_headers;
    std::vector<const unsigned char*> sec_names;
};

#endif
