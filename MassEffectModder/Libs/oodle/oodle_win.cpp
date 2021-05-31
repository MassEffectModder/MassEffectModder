/*
 * MassEffectModder
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

#include "oodle.h"

#include <cstdint>
#include <windows.h>

typedef int64_t WINAPI OodleLZDecompressFunc(uint8_t *src, int64_t srcLen, uint8_t *dst, int64_t dstLen,
                                            int64_t, int64_t, int64_t, int64_t, size_t, int64_t,
                                            int64_t, int64_t, int64_t, int64_t);
typedef int64_t WINAPI OodleLZCompressFunc(int codecId, uint8_t *src, int64_t srcLen, uint8_t *dst, int64_t,
                                           int64_t, int64_t, int64_t, int64_t, int64_t);

static OodleLZDecompressFunc *OodleLZDecompress = nullptr;
static OodleLZCompressFunc *OodleLZCompress = nullptr;
static HINSTANCE libInst = nullptr;

bool OodleLoadLib(const wchar_t *libPath)
{
    if (!libPath)
        return false;

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

    return true;
}

void OodleUnloadLib()
{
    if (!libInst)
        return;

    FreeLibrary(libInst);
    libInst = nullptr;
}

int OodleCompressData(unsigned char *src, unsigned int srcLen,
                      unsigned char *dst, unsigned int *dstLen)
{
    if (!OodleLZCompress)
        return -5;

    unsigned int outputSize = OodleLZCompress(13, src, srcLen, dst, 4, 0, 0, 0, 0, 0);
    if (outputSize < 0)
        return -1;

    *dstLen = outputSize;

    return 0;
}

int OodleDecompressData(unsigned char *src, unsigned int srcLen,
                        unsigned char *dst, unsigned int dstLen)
{
    byte b1 = src[0];
    byte b2 = src[1];
    unsigned int outputSize;

    if ((b1 == 0x8C || b1 == 0xCC) && (b2 == 12))
    {
        if (!OodleLZDecompress)
            return -5;
        outputSize = OodleLZDecompress(src, srcLen, dst, dstLen, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        if (outputSize < 0)
            return -1;
        return 0;
    }

    return -10;
}
