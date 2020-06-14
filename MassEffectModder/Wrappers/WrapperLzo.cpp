/*
 * Wrapper LZO2
 *
 * Copyright (C) 2014-2020 Pawel Kolodziejski
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
#include <memory>

#include <lzo/lzo1x.h>

int LzoDecompress(unsigned char *src, unsigned int src_len, unsigned char *dst, unsigned int *dst_len)
{
    lzo_uint len = *dst_len;

    int status = lzo_init();
    if (status != LZO_E_OK)
        return status;

    status = lzo1x_decompress_safe(src, src_len, dst, &len, nullptr);
    if (status != LZO_E_OK)
    {
        printf("lzo1x_decompress_safe failed - error: %d\n", status);
    }
    *dst_len = (unsigned int)len;

    return status;
}

int LzoCompress(unsigned char *src, unsigned int src_len, unsigned char **dst, unsigned int *dst_len)
{
    lzo_uint len = 0;

    int status = lzo_init();
    if (status != LZO_E_OK)
        return status;

    auto *wrkmem = new unsigned char[LZO1X_1_15_MEM_COMPRESS];
    if (wrkmem == nullptr)
        return -100;
    memset(wrkmem, 0, LZO1X_1_15_MEM_COMPRESS);

    auto *tmpBuffer = new unsigned char[src_len + LZO1X_1_15_MEM_COMPRESS];
    if (tmpBuffer == nullptr)
    {
        delete[] wrkmem;
        return -100;
    }
    memset(tmpBuffer, 0, src_len + LZO1X_1_15_MEM_COMPRESS);

    status = lzo1x_1_15_compress(src, src_len, tmpBuffer, &len, wrkmem);
    if (status == LZO_E_OK)
    {
        *dst = new unsigned char[len];
        if (*dst != nullptr)
        {
            memcpy(*dst, tmpBuffer, len);
            *dst_len = static_cast<unsigned int>(len);
        }
        else
        {
            status = -100;
        }
    }
    else
    {
        printf("lzo1x_1_15_compress failed - error: %d\n", status);
    }

    delete[] tmpBuffer;
    delete[] wrkmem;

    return status;
}
