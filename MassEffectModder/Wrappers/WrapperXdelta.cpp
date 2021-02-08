/*
 * Wrapper Xdelta3
 *
 * Copyright (C) 2018-2021 Pawel Kolodziejski
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

#define HAVE_CONFIG_H 1
#define REGRESSION_TEST 0
#define SHELL_TESTS 0
#define EXTERNAL_COMPRESSION 0
#define SECONDARY_DJW 1
#define SECONDARY_FGK 1
#define SECONDARY_LZMA 0
#define XD3_DEBUG 0
#define XD3_MAIN 0
#define XD3_ENCODER 1
#define XD3_WIN32 0
#define XD3_STDIO 0
#define XD3_POSIX 0
#define VCDIFF_TOOLS 0

#include <xdelta3.h>

int XDelta3Compress(unsigned char *src1, unsigned char *src2, unsigned int src_len,
        unsigned char *delta, unsigned int *delta_len)
{
    int result;
    usize_t len = 0;

    result = xd3_encode_memory(static_cast<const uint8_t *>(src2), static_cast<usize_t>(src_len),
                               static_cast<const uint8_t *>(src1), static_cast<usize_t>(src_len),
                               static_cast<uint8_t *>(delta), &len, static_cast<usize_t>(src_len),
                               XD3_SEC_DJW | XD3_ADLER32);

    if (result == 0)
        *delta_len = static_cast<unsigned int>(len);

    return result;
}

int XDelta3Decompress(unsigned char *src, unsigned int src_len,
        unsigned char *delta, unsigned int delta_len, unsigned char *dst, unsigned int *dst_len)
{
    int result;
    usize_t len = 0;

    result = xd3_decode_memory(static_cast<const uint8_t *>(delta), static_cast<usize_t>(delta_len),
                               static_cast<const uint8_t *>(src), static_cast<usize_t>(src_len),
                               static_cast<uint8_t *>(dst), &len, static_cast<usize_t>(src_len), 0);

    if (result == 0)
        *dst_len = static_cast<unsigned int>(len);

    return result;
}
