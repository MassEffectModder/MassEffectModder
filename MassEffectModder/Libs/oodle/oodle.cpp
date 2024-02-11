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

#include "oodle.h"

#include <cstdint>
#include <cstddef>

#ifdef WIN32
#include <windows.h>
#else
extern "C" {
#define WINAPI __attribute__((ms_abi))
void *LoadLibrary(const char *filename);
bool FreeLibrary(void *handle);
bool NativeLibrary(void *handle);
void *GetProcAddress(void *pe, const char *name);
}
#endif

typedef int64_t WINAPI OodleLZDecompressFuncWinApi(uint8_t *src, int64_t srcLen, uint8_t *dst, int64_t dstLen, int,
                                                   int, int, void *, int64_t, void *, void *, void *, int64_t, int);
typedef int64_t WINAPI OodleLZCompressFuncWinApi(int codecId, uint8_t *src, int64_t srcLen, uint8_t *dst,
                                                 int compressionLevel, void *, void *, void *, void *, int64_t);
typedef int64_t OodleLZDecompressFunc(uint8_t *src, int64_t srcLen, uint8_t *dst, int64_t dstLen, int,
                                      int, int, void *, int64_t, void *, void *, void *, int64_t, int);
typedef int64_t OodleLZCompressFunc(int codecId, uint8_t *src, int64_t srcLen, uint8_t *dst,
                                    int compressionLevel, void *, void *, void *, void *, int64_t);

namespace {
OodleLZDecompressFuncWinApi *OodleLZDecompressWinApi = nullptr;
OodleLZCompressFuncWinApi *OodleLZCompressWinApi = nullptr;
OodleLZDecompressFunc *OodleLZDecompress = nullptr;
OodleLZCompressFunc *OodleLZCompress = nullptr;
#ifdef WIN32
static HINSTANCE libInst = nullptr;
#else
void *libInst = nullptr;
#endif
}

int Kraken_Decompress(const uint8_t *src, size_t src_len, uint8_t *dst, size_t dst_len);

int64_t OodleLZDecompressNative(uint8_t *src, int64_t srcLen, uint8_t *dst, int64_t dstLen, int,
                                int, int, void *, int64_t, void *, void *, void *, int64_t, int)
{
    uint8_t b1 = src[0];
    uint8_t b2 = src[1];
    int64_t outputSize;

    if ((b1 == 0x8C || b1 == 0xCC) && (b2 == 12))
    {
        outputSize = Kraken_Decompress((const uint8_t *)src, srcLen, dst, dstLen);
        if (outputSize == -1)
            return -1;
        if (outputSize == dstLen)
            return 0;
    }

    return -10;
}

#ifdef WIN32
bool OodleLoadLib(const wchar_t *libPath)
#else
bool OodleLoadLib(const char *libPath)
#endif
{
    if (!libPath)
    {
        OodleLZDecompress = OodleLZDecompressNative;
        return true;
    }

    if (libInst)
        return true;

    libInst = LoadLibrary(libPath);
    if (!libInst)
        return false;

#ifndef WIN32
    if (NativeLibrary(libInst)) {
        OodleLZDecompress = (OodleLZDecompressFunc *)GetProcAddress(libInst, "OodleLZ_Decompress");
        OodleLZCompress = (OodleLZCompressFunc *)GetProcAddress(libInst, "OodleLZ_Compress");
        if (OodleLZDecompress && OodleLZCompress)
        {
            return true;
        }
    }
#endif
    OodleLZDecompressWinApi = (OodleLZDecompressFuncWinApi *)GetProcAddress(libInst, "OodleLZ_Decompress");
    OodleLZCompressWinApi = (OodleLZCompressFuncWinApi *)GetProcAddress(libInst, "OodleLZ_Compress");
    if (OodleLZDecompressWinApi && OodleLZCompressWinApi)
    {
        return true;
    }

    FreeLibrary(libInst);
    libInst = nullptr;
    return false;
}

void OodleUnloadLib()
{
    if (!libInst)
        return;

    FreeLibrary(libInst);
    libInst = nullptr;
}

bool OodleSupportCompression()
{
    return OodleLZCompressWinApi || OodleLZCompress;
}

int OodleCompressData(unsigned char *src, unsigned int srcLen,
                      unsigned char *dst, unsigned int *dstLen)
{
    if (!OodleLZCompressWinApi && !OodleLZCompress)
        return -5;

    unsigned int outputSize = -1;
    if (OodleLZCompressWinApi)
        outputSize = OodleLZCompressWinApi(13, src, srcLen, dst, 4, nullptr, nullptr, nullptr, nullptr, 0);
    else if (OodleLZCompress)
        outputSize = OodleLZCompress(13, src, srcLen, dst, 4, nullptr, nullptr, nullptr, nullptr, 0);
    if (outputSize < 0)
        return -1;

    *dstLen = outputSize;

    return 0;
}

int OodleDecompressData(unsigned char *src, unsigned int srcLen,
                        unsigned char *dst, unsigned int dstLen)
{
    if (!OodleLZDecompressWinApi && !OodleLZDecompress)
        return -5;

    unsigned int outputSize = -1;
    if (OodleLZDecompressWinApi)
        outputSize = OodleLZDecompressWinApi(src, srcLen, dst, dstLen, 0, 0, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, 0);
    else if (OodleLZDecompress)
        outputSize = OodleLZDecompress(src, srcLen, dst, dstLen, 0, 0, 0, nullptr, 0, nullptr, nullptr, nullptr, 0, 0);
    if (outputSize < 0)
        return -1;

    return 0;
}
