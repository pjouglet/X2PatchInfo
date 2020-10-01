#include "adler32.h"

namespace AdlerCheckSum
{

#define BASE 0xFFF1
#define NMAX 5552
#define DO1(buf,i)  {adler += (buf)[i]; sum2 += adler;}
#define DO2(buf,i)  DO1(buf,i); DO1(buf,i+1);
#define DO4(buf,i)  DO2(buf,i); DO2(buf,i+2);
#define DO8(buf,i)  DO4(buf,i); DO4(buf,i+4);
#define DO16(buf)   DO8(buf,0); DO8(buf,8);
#define MOD(a) a %= BASE
#define MOD4(a) a %= BASE


    unsigned int adler32(unsigned int adler, const unsigned char* buf, unsigned int len)
    {
        unsigned int sum2;
        unsigned int n;

        sum2 = (adler >> 16) & 0xffff;
        adler &= 0xffff;

        if (len == 1) {
            adler += buf[0];
            if (adler >= BASE)
                adler -= BASE;
            sum2 += adler;
            if (sum2 >= BASE)
                sum2 -= BASE;
            return adler | (sum2 << 16);
        }

        if (buf == NULL)
            return 1L;

        if (len < 16) {
            while (len--) {
                adler += *buf++;
                sum2 += adler;
            }
            if (adler >= BASE)
                adler -= BASE;
            MOD4(sum2);
            return adler | (sum2 << 16);
        }

        while (len >= NMAX) {
            len -= NMAX;
            n = NMAX / 16;
            do {
                DO16(buf);
                buf += 16;
            } while (--n);
            MOD(adler);
            MOD(sum2);
        }


        if (len) {
            while (len >= 16) {
                len -= 16;
                DO16(buf);
                buf += 16;
            }
            while (len--) {
                adler += *buf++;
                sum2 += adler;
            }
            MOD(adler);
            MOD(sum2);
        }

        return adler | (sum2 << 16);
    }


    unsigned int adler32_combine(unsigned int adler1, unsigned int adler2, unsigned int len2)
    {
        unsigned int sum1;
        unsigned int sum2;
        unsigned rem;

        rem = (unsigned)(len2 % BASE);
        sum1 = adler1 & 0xffff;
        sum2 = rem * sum1;
        MOD(sum2);
        sum1 += (adler2 & 0xffff) + BASE - 1;
        sum2 += ((adler1 >> 16) & 0xffff) + ((adler2 >> 16) & 0xffff) + BASE - rem;
        if (sum1 > BASE) sum1 -= BASE;
        if (sum1 > BASE) sum1 -= BASE;
        if (sum2 > (BASE << 1)) sum2 -= (BASE << 1);
        if (sum2 > BASE) sum2 -= BASE;
        return sum1 | (sum2 << 16);
    }


    unsigned int adler32(std::istream& stream, int offset, int length, bool resetposition)
    {
        unsigned char buffer[2048];
        unsigned int adler = 0x0001;
        long cur = stream.tellg();

        if (length == 0)
            return 0;

        stream.seekg(offset, std::ios_base::beg);

        int totallen = length;
        int len = 2048;
        while (totallen > 0)
        {
            if (totallen < 2048)
            {
                len = totallen;
            }
            else
            {
                len = 2048;
            }

            stream.read((char*)buffer, len);
            adler = adler32(adler, buffer, len);
            totallen -= len;
        }

        if (resetposition)
            stream.seekg(cur, std::ios::beg);

        return adler;
    }

    unsigned int adler32(const char* filename, unsigned int* size)
    {
        unsigned int adler;
        int length;
        std::ifstream file(filename, std::ios_base::binary | std::ios_base::in);
        file.seekg(0, std::ios::end);
        length = (int)file.tellg();
        if (size)
            *size = length;

        adler = adler32(file, 0, length, true);
        file.close();
        return adler;
    }
}
