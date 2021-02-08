/*
 * Wrapper LZX
 *
 * Copyright (C) 2020-2021 Pawel Kolodziejski
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

#include <cstdlib>
#include <cstring>
#include <cstdio>

extern "C" {
    int lzx_decompress(unsigned char *CompressedBuffer, int CompressedSize, unsigned char *UncompressedBuffer, int UncompressedSize);
}

#ifndef EXPORT_LIBS

int LzxDecompress(unsigned char *src, unsigned int src_len, unsigned char *dst, unsigned int *dst_len)
{
    int status = lzx_decompress(src, src_len, dst, *dst_len);
    if (status != 0)
    {
        printf("lzx_decompress failed - error: %d\n", status);
    }

    return status;
}

#else

#ifdef _WIN32
#define LIB_EXPORT extern "C" __declspec(dllexport)
#else
#define LIB_EXPORT extern "C"
#endif

LIB_EXPORT int LZXDecompress(unsigned char *src, unsigned int src_len, unsigned char *dst, unsigned int *dst_len)
{
    return lzx_decompress(src, src_len, dst, *dst_len);
}

#endif
