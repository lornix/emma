/* emma - read_file.h */

#ifndef READ_FILE_H
#define READ_FILE_H

struct alldata {
    asection a;
    bfd_byte* b;
};

void grab_section(bfd* b,asection* s,void* vv);
void show_sections();
int load_sections(const char* fname);

extern std::vector<alldata> sections;

#endif
