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
    readelf32(const char* fname) throw(std::string);
    readelf32(std::string fname) throw(std::string);
    ~readelf32();
 private:
    std::string filename;
    Elf32_Ehdr elf_hdr;
};

#endif
