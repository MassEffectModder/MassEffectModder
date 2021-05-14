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
#include <memory>
#include <oodle.h>

#ifndef EXPORT_LIBS

int OodleDecompress(unsigned char *src, unsigned int srcLen, unsigned char *dst, unsigned int *dstLen)
{
    return OodleDecompressData(src, srcLen, dst, dstLen);
}

int OodleCompress(unsigned char *src, unsigned int srcLen,
                  unsigned char **dst, unsigned int *dstLen, int compressionLevel)
{
    return OodleCompressData(src, srcLen, dst, dstLen, compressionLevel);
}

#else

#ifdef _WIN32
#define LIB_EXPORT extern "C" __declspec(dllexport)
#else
#define LIB_EXPORT extern "C"
#endif

LIB_EXPORT int OodleDecompress(unsigned char *src, unsigned int srcLen, unsigned char *dst, unsigned int *dstLen)
{
    return OodleDecompressData(src, srcLen, dst, dstLen);
}

LIB_EXPORT int OodleCompress(unsigned char *src, unsigned int srcLen, unsigned char *dst, unsigned int *dstLen, int compressionLevel)
{
    return OodleCompress(src, srcLen, dst, dstLen, compressionLevel);
}

#endif
