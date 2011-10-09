/* emma - read_elf.h */

#ifndef READ_ELF_H
#define READ_ELF_H

struct alldata {
    asection a;
    bfd_byte* b;
};

void grab_section(bfd* b,asection* s,void* vv);
void show_sections();
int load_sections(const char* fname);

extern std::vector<alldata> sections;

#endif
