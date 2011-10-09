/* emma - read_elf.h */

#ifndef READ_ELF_H
#define READ_ELF_H

class read_elf
{
 public:
    //
    struct filedata_s {
        asection a;
        bfd_byte* b;
    };
    //
    read_elf(std::string fname);
    ~read_elf();
    const filedata_s* section(std::string name);
    const filedata_s* operator[](unsigned int index);
 private:
    // object types we handle at the moment
    std::vector<filedata_s> filedata;
    int load_sections(std::string fname);
    friend void grab_section(bfd* b,asection* s,void* vv);
};

#endif
