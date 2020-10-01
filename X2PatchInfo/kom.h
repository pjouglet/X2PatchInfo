#pragma once

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <map>
#include <sstream>
#include "adler32.h"
#include <windows.h>
#define HEADEROFFSET 72

extern bool DeleteFileForce(std::string filename);

class KSubfile
{
public:
    enum EAlgorithm
    {
        infilate = 0,
        lzma = 1
    };

    KSubfile(std::string filename, std::ifstream& stream, int headersize);
    KSubfile(std::string filename, xmlNode* node, int* offset);


    KSubfile(const KSubfile& src)
        :filename(src.filename), filetime(src.filetime), adler32(src.adler32), size(src.size), compressedsize(src.compressedsize), algorithm(src.algorithm), offset(src.offset), iscalcadler(src.iscalcadler)
    {}
    ~KSubfile() = default;;

    bool operator == (const KSubfile& r)
    {
        //if(r.filetime==0)
        return (adler32 == r.adler32 && algorithm == r.algorithm && compressedsize == r.compressedsize);

        //return (adler32 == r.adler32 && algorithm == r.algorithm &&	compressedsize == r.compressedsize && filetime == r.filetime);
    }

    KSubfile& operator= (const KSubfile& r)
    {
        adler32 = r.adler32;
        algorithm = r.algorithm;
        compressedsize = r.compressedsize;
        filename = r.filename;
        size = r.size;
        offset = r.offset;
        iscalcadler = r.iscalcadler;
        if (r.filetime != 0)
            filetime = r.filetime;
        return *this;
    }

    bool WriteCompressed(std::ostream& stream);
    bool Verify();


    unsigned int GetFileTime() const { return filetime; }
    void SetFileTime(unsigned int ft) { filetime = ft; }

    unsigned int GetAdler32();

    unsigned int GetSize() const { return size; }
    unsigned int GetCompressedSize() const { return compressedsize; }
    int GetAlgorithm() const { return algorithm; }

private:

    std::string filename;
    unsigned int filetime;
    unsigned int adler32;
    unsigned int size;
    unsigned int compressedsize;
    int algorithm;

    unsigned int offset;

    bool iscalcadler;
};


class Komfile
{
public:

    enum KOM_TYPE
    {
        NOT_KOM = -1,
        OLD_KOM = 0,
        CUR_KOM = 1
    };

    static KOM_TYPE CheckKom(std::string filename);
    static bool Verify(std::string filename);


    Komfile() { Close(); };
    Komfile(const Komfile& r)
        :subfiles(r.subfiles), filetime(r.filetime), adler32(r.adler32), headersize(r.headersize)
    {};

    ~Komfile() { Close(); };



    Komfile& operator = (const Komfile& r)
    {
        subfiles = r.subfiles;
        filetime = r.filetime;
        adler32 = r.adler32;
        headersize = r.headersize;

        return *this;
    }

    bool LeftOuterJoin(Komfile& left, Komfile& right);

    bool Open(std::string filename);
    void Close();

    unsigned int GetFileTime();
    unsigned int GetAdler32() const { return adler32; }
    unsigned int GetHeaderSize() const { return headersize; }

    bool Verify();
private:

    std::map<std::string, KSubfile> subfiles;
    unsigned int filetime;
    unsigned int adler32;
    unsigned int headersize;
};