/*
 * Wrapper Zstd
 *
 * Copyright (C) 2019-2020 Pawel Kolodziejski
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

#define ZSTD_STATIC_LINKING_ONLY
#include <zstd.h>
#include <cstring>
#include <memory>

int ZstdDecompress(unsigned char *src, unsigned int src_len, unsigned char *dst, unsigned int *dst_len)
{
    size_t len = *dst_len;
    unsigned long long const rSize = ZSTD_findDecompressedSize(src, src_len);
    if (rSize == ZSTD_CONTENTSIZE_ERROR ||
        rSize == ZSTD_CONTENTSIZE_UNKNOWN)
    {
        *dst_len = 0;
        printf("zstd uncompress failed - error: not known size!\n");
        return -1;
    }
    if (rSize > len)
    {
        *dst_len = 0;
        printf("zstd uncompress failed - error: buffer too small!\n");
        return -1;
    }

    size_t const dSize = ZSTD_decompress(dst, len, src, src_len);
    if (dSize == rSize)
        *dst_len = static_cast<unsigned int>(dSize);
    else
    {
        *dst_len = 0;
        printf("zstd uncompress failed - Error: %s\n", ZSTD_getErrorName(dSize));
        return -1;
    }

    return 0;
}

int ZstdCompress(unsigned char *src, unsigned int src_len,
                 unsigned char **dst, unsigned int *dst_len, int compression_level)
{
    size_t const tmpBufLen = ZSTD_compressBound(src_len);
    auto *tmpbuf = new unsigned char[tmpBufLen];
    if (tmpbuf == nullptr)
        return -100;

    size_t const cSize = ZSTD_compress(tmpbuf, tmpBufLen, src, src_len, compression_level);
    if (ZSTD_isError(cSize))
    {
        *dst_len = 0;
        printf("zstd compress failed - Error: %s\n", ZSTD_getErrorName(cSize));
    }
    else
    {
        *dst = new unsigned char[cSize];
        if (*dst == nullptr)
        {
            delete[] tmpbuf;
            return -100;
        }
        memcpy(*dst, tmpbuf, cSize);
        *dst_len = static_cast<unsigned int>(cSize);
    }

    delete[] tmpbuf;

    return 0;
}
