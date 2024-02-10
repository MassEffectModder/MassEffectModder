/*
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

#if defined(__x86_64__)

#include "oodle.h"

#include <cstdint>
#include <cstddef>

#ifdef WIN32
#include <windows.h>
#else
extern "C" {
#include "winnt_types.h"
#include "pe_linker.h"
}
#endif

typedef int64_t WINAPI OodleLZDecompressFunc(uint8_t *src, int64_t srcLen, uint8_t *dst, int64_t dstLen, int,
                                             int, int, void *, int64_t, void *, void *, void *, int64_t, int);
typedef int64_t WINAPI OodleLZCompressFunc(int codecId, uint8_t *src, int64_t srcLen, uint8_t *dst,
                                           int compressionLevel, void *, void *, void *, void *, int64_t);

static OodleLZDecompressFunc *OodleLZDecompress = nullptr;
static OodleLZCompressFunc *OodleLZCompress = nullptr;
#ifdef WIN32
static HINSTANCE libInst = nullptr;
#else
static struct pe_image *libInst = nullptr;
#endif

#endif // __x86_64__

#ifdef WIN32
bool OodleLoadLib(const wchar_t *libPath)
#else
bool OodleLoadLib(const char *libPath)
#endif
{
    if (!libPath)
        return false;

#if defined(__x86_64__)
    if (libInst)
        return true;

    libInst = LoadLibrary(libPath);
    if (!libInst)
        return false;

    OodleLZDecompress = (OodleLZDecompressFunc *)GetProcAddress(libInst, "OodleLZ_Decompress");
    OodleLZCompress = (OodleLZCompressFunc *)GetProcAddress(libInst, "OodleLZ_Compress");
    if (!OodleLZDecompress || !OodleLZCompress)
    {
        FreeLibrary(libInst);
        libInst = nullptr;
        return false;
    }
#endif

    return true;
}

void OodleUnloadLib()
{
#if defined(__x86_64__)
    if (!libInst)
        return;

    FreeLibrary(libInst);
    libInst = nullptr;
#endif
}

int OodleCompressData(unsigned char *src, unsigned int srcLen,
                      unsigned char *dst, unsigned int *dstLen)
{
#if defined(__x86_64__)
    if (!OodleLZCompress)
        return -5;

    unsigned int outputSize = OodleLZCompress(13, src, srcLen, dst, 4, nullptr, nullptr, nullptr, nullptr, 0);
    if (outputSize < 0)
        return -1;

    *dstLen = outputSize;
#endif

    return 0;
}

int OodleDecompressData(unsigned char *src, unsigned int srcLen,
                        unsigned char *dst, unsigned int dstLen)
{
#if defined(__x86_64__)
    if (!OodleLZDecompress)
        return -5;

    unsigned int outputSize = OodleLZDecompress(src, srcLen, dst, dstLen, 0, 0, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, 0);
    if (outputSize < 0)
        return -1;
#endif

    return 0;
}
