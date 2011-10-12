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
    readelf32(std::string fname);
    ~readelf32();
 private:
};

#endif
