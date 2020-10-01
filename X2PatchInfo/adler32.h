#pragma once

#include <iostream>
#include <fstream>
#include <windows.h>

namespace AdlerCheckSum
{
    unsigned int adler32(unsigned int adler, const unsigned char* buf, unsigned int len);
    unsigned int adler32_combine(unsigned int adler1, unsigned int adler2, unsigned int len2);

    unsigned int adler32(const char* filename, unsigned int* size = NULL);
    unsigned int adler32(std::istream& stream, int offset, int length, bool resetposition = true);
}