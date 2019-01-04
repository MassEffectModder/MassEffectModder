/* WrapperZlib.cpp

        Copyright (C) 2017-2019 Pawel Kolodziejski

        ---------------------------------------------------------------------------------

        Condition of use and distribution are the same than zlib :
 
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgement in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  ---------------------------------------------------------------------------------
*/

#include <zlib.h>
#include <cstring>
#include <memory>

int ZlibDecompress(unsigned char *src, unsigned int src_len, unsigned char *dst, unsigned int *dst_len)
{
    uLongf len = *dst_len;

    int status = uncompress(static_cast<Bytef *>(dst), &len, static_cast<Bytef *>(src), static_cast<uLong>(src_len));
    if (status == Z_OK)
        *dst_len = static_cast<unsigned int>(len);
    else
        *dst_len = 0;

    return status;
}

int ZlibCompress(unsigned char *src, unsigned int src_len, unsigned char **dst, unsigned int *dst_len, int compression_level)
{
    int tmpBufLen = (src_len * 2) + 128;
    unsigned char *tmpbuf = new unsigned char[tmpBufLen];
    uLongf len = tmpBufLen;

    int status = compress2(static_cast<Bytef *>(tmpbuf), &len, static_cast<Bytef *>(src), static_cast<uLong>(src_len), compression_level);
    if (status == Z_OK)
    {
        *dst = new unsigned char[len];
        memcpy(*dst, tmpbuf, len);
        *dst_len = static_cast<unsigned int>(len);
    }
    else
        *dst_len = 0;

    delete[] tmpbuf;

    return status;
}
