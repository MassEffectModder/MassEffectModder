/*
 * Wrapper Oodle
 *
 * Copyright (C) 2021 Pawel Kolodziejski
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <cstring>
#include <cstdio>
#include <oodle.h>

#ifdef EXPORT_LIBS
#ifdef _WIN32
#define LIB_EXPORT extern "C" __declspec(dllexport)
#else
#define LIB_EXPORT extern "C"
#endif
#else
#define LIB_EXPORT
#endif

#ifdef EXPORT_LIBS
#ifdef WIN32
bool OodleLoadLib(const wchar_t *libPath);
#else
bool OodleLoadLib(const char *libPath);
#endif
void OodleUnloadLib();
bool OodleSupportCompression();
int OodleCompressData(unsigned char *src, unsigned int srcLen,
                      unsigned char *dst, unsigned int *dstLen);
int OodleDecompressData(unsigned char *src, unsigned int srcLen,
                        unsigned char *dst, unsigned int dstLen);
#endif


LIB_EXPORT
#ifdef WIN32
bool OodleInitLib(const wchar_t *libPath)
#else
bool OodleInitLib(const char *libPath)
#endif
{
    return OodleLoadLib(libPath);
}

LIB_EXPORT
void OodleUninitLib()
{
    OodleUnloadLib();
}

LIB_EXPORT
bool OodleIsCompressionSupported()
{
    return OodleSupportCompression();
}

LIB_EXPORT
int OodleDecompress(unsigned char *src, unsigned int srcLen, unsigned char *dst, unsigned int dstLen)
{
    return OodleDecompressData(src, srcLen, dst, dstLen);
}

LIB_EXPORT
int OodleCompress(unsigned char *src, unsigned int srcLen,
                  unsigned char **dst, unsigned int *dstLen)
{
    unsigned int tmpBufLen = srcLen * 2;
    unsigned int len = tmpBufLen;
    auto *tmpbuf = new unsigned char[tmpBufLen];
    if (tmpbuf == nullptr)
        return -100;

    int status = OodleCompressData(src, srcLen, tmpbuf, &len);
    if (status == 0)
    {
        *dst = new unsigned char[len];
        if (*dst == nullptr)
        {
            delete[] tmpbuf;
            return -100;
        }
        memcpy(*dst, tmpbuf, len);
        *dstLen = static_cast<unsigned int>(len);
    }
    else
    {
        printf("compress2 failed - error: %d\n", status);
        *dstLen = 0;
    }

    delete[] tmpbuf;

    return status;
}
