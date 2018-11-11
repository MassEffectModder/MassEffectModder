/*

 C++ port of C# OptimizedCRC
 Copyright (c) 2018 Pawel Kolodziejski

*/

/*

 Copyright (c) 2012-2015 Eugene Larchenko (spct@mail.ru)

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.

*/

#ifndef OPTIMIZED_CRC_H
#define OPTIMIZED_CRC_H

static const uint kCrcPoly = 0xEDB88320;
static const uint kInitial = 0xFFFFFFFF;
static const int CRC_NUM_TABLES = 8;
static uint Table[256 * CRC_NUM_TABLES];

static bool initialized = false;

class OptimizedCRC
{
private:

    uint value{};

public:

    OptimizedCRC()
    {
        Init();
    }

    ~OptimizedCRC() = default;

    void Init()
    {
        if (!initialized)
        {
            int i;
            for (i = 0; i < 256; i++)
            {
                auto r = (uint)i;
                for (int j = 0; j < 8; j++)
                    r = (r >> 1) ^ (kCrcPoly & ~((r & 1) - 1));
                Table[i] = r;
            }
            for (; i < 256 * CRC_NUM_TABLES; i++)
            {
                uint r = Table[i - 256];
                Table[i] = Table[r & 0xFF] ^ (r >> 8);
            }

            initialized = true;
        }

        value = kInitial;
    }

    int getValue()
    {
        return (int)~value;
    }

    void Update(const unsigned char *data, int offset, int count)
    {
        value = ProcessBlock(value, data, offset, count);
    }

    static int Compute(const unsigned char *data, int offset, int count)
    {
        auto crc = OptimizedCRC();
        crc.Update(data, offset, count);
        int crcValue = crc.getValue();
        return crcValue;
    }

    static int Compute(const unsigned char *data, uint length)
    {
        return Compute(data, 0, length);
    }

private:

    static uint ProcessBlock(uint crc, const unsigned char *data, int offset, int count)
    {
        if (count < 0) throw;
        if (count == 0) return crc;

        for (; (offset & 7) != 0 && count != 0; count--)
            crc = (crc >> 8) ^ Table[(unsigned char)crc ^ data[offset++]];

        if (count >= 8)
        {
            int end = (count - 8) & ~7;
            count -= end;
            end += offset;

            while (offset != end)
            {
                crc ^= (uint)(data[offset] + (data[offset + 1] << 8) + (data[offset + 2] << 16) + (data[offset + 3] << 24));
                auto high = (uint)(data[offset + 4] + (data[offset + 5] << 8) + (data[offset + 6] << 16) + (data[offset + 7] << 24));
                offset += 8;

                crc = Table[(unsigned char)crc + 0x700]
                    ^ Table[(unsigned char)(crc >> 8) + 0x600]
                    ^ Table[(unsigned char)(crc >> 16) + 0x500]
                    ^ Table[(unsigned char)(crc >> 24) + 0x400]
                    ^ Table[(unsigned char)(high) + 0x300]
                    ^ Table[(unsigned char)(high >> 8) + 0x200]
                    ^ Table[(unsigned char)(high >> 16) + 0x100]
                    ^ Table[(unsigned char)(high >> 24) + 0x000];
            }
        }

        while (count-- != 0)
            crc = (crc >> 8) ^ Table[(unsigned char)crc ^ data[offset++]];

        return crc;
    }
};

#endif
