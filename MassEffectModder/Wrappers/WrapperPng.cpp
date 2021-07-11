/*
 * Wrapper PNG
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

#define PNG_DEBUG 3
#include <png.h>
#include <cstdlib>
#include <cstring>
#include <cmath>

#define PNG_SIGN_LEN 8

typedef struct {
    unsigned char *bufferPtr;
    png_size_t bufferOffset;
    png_size_t bufferSize;
} IoHandle;

static void ReadFunction(png_structp pngStruct, png_bytep buffer, png_size_t count)
{
    auto *handle = static_cast<IoHandle *>(png_get_io_ptr(pngStruct));
    if ((count + handle->bufferOffset) <= handle->bufferSize)
        memcpy(buffer, handle->bufferPtr + handle->bufferOffset, count);
    else
        memset(buffer, 0, count);
    handle->bufferOffset += count;
}

static void WriteFunction(png_structp pngStruct, png_bytep buffer, png_size_t count)
{
    auto *handle = static_cast<IoHandle *>(png_get_io_ptr(pngStruct));
    if (!handle->bufferPtr)
        handle->bufferPtr = static_cast<unsigned char *>(malloc(count));
    else
    {
        png_size_t newPosition = handle->bufferOffset + count;
        if (newPosition > handle->bufferSize)
        {
            handle->bufferSize = newPosition + handle->bufferSize * 2;
            handle->bufferPtr = static_cast<unsigned char *>(realloc(handle->bufferPtr, handle->bufferSize));
        }
    }
    memcpy(handle->bufferPtr + handle->bufferOffset, buffer, count);
    handle->bufferOffset += count;
}

int PngRead(unsigned char *src, unsigned int srcSize,
             float **dst, unsigned int *dstSize,
             unsigned int *width, unsigned int *height)
{
    IoHandle handle;
    png_structp pngStruct;
    png_infop pngInfo;
    png_uint_32 pngWidth, pngHeight;
    int bits, colorType;

    if (!png_check_sig(src, PNG_SIGN_LEN) || srcSize < PNG_SIGN_LEN)
        return -1;

    handle.bufferOffset = PNG_SIGN_LEN;
    handle.bufferSize = srcSize;
    handle.bufferPtr = src;

    pngStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!pngStruct)
        return -1;

    pngInfo = png_create_info_struct(pngStruct);
    if (!pngInfo)
    {
        png_destroy_read_struct(&pngStruct, nullptr, nullptr);
        return -1;
    }

    png_set_read_fn(pngStruct, &handle, ReadFunction);

    png_set_sig_bytes(pngStruct, PNG_SIGN_LEN);

    png_read_info(pngStruct, pngInfo);

    if (png_get_IHDR(pngStruct, pngInfo, &pngWidth, &pngHeight, &bits, &colorType, nullptr, nullptr, nullptr) == 0)
    {
        png_destroy_read_struct(&pngStruct, &pngInfo, nullptr);
        return -1;
    }
    *width = pngWidth;
    *height = pngHeight;

    png_set_packing(pngStruct);
    if (colorType == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(pngStruct);
    if (colorType == PNG_COLOR_TYPE_GRAY && bits < 8)
        png_set_expand_gray_1_2_4_to_8(pngStruct);
    png_set_invert_mono(pngStruct);
    if (png_get_valid(pngStruct, pngInfo, PNG_INFO_tRNS) != 0)
        png_set_tRNS_to_alpha(pngStruct);
    if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(pngStruct);
    png_set_swap(pngStruct);
    png_set_filler(pngStruct, 0xff, PNG_FILLER_BEFORE);
    png_read_update_info(pngStruct, pngInfo);

    *dstSize = pngWidth * pngHeight * 4 * sizeof(float);
    float *dstPtr = *dst = new float[*dstSize];
    if (*dst == nullptr)
    {
        png_destroy_read_struct(&pngStruct, &pngInfo, nullptr);
        return -1;
    }

    unsigned char lineData8[png_get_rowbytes(pngStruct, pngInfo)];
    unsigned short lineData16[png_get_rowbytes(pngStruct, pngInfo)];
    int dstOffset = 0;
    for (png_uint_32 y = 0; y < pngHeight; y++)
    {
        int lineOffset = 0;
        if (bits == 16)
        {
            png_read_row(pngStruct, (png_bytep)lineData16, nullptr);
            for (png_uint_32 x = 0; x < pngWidth; x++)
            {
                dstPtr[dstOffset + 0] = lineData16[lineOffset++] / 65535.0f;
                dstPtr[dstOffset + 1] = lineData16[lineOffset++] / 65535.0f;
                dstPtr[dstOffset + 2] = lineData16[lineOffset++] / 65535.0f;
                dstPtr[dstOffset + 3] = lineData16[lineOffset++] / 65535.0f;
                dstOffset += 4;
            }
        }
        else
        {
            png_read_row(pngStruct, (png_bytep)lineData8, nullptr);
            for (png_uint_32 x = 0; x < pngWidth; x++)
            {
                dstPtr[dstOffset + 0] = lineData8[lineOffset++] / 255.0f;
                dstPtr[dstOffset + 1] = lineData8[lineOffset++] / 255.0f;
                dstPtr[dstOffset + 2] = lineData8[lineOffset++] / 255.0f;
                dstPtr[dstOffset + 3] = lineData8[lineOffset++] / 255.0f;
                dstOffset += 4;
            }
        }
    }
    png_read_end(pngStruct, pngInfo);

    png_destroy_read_struct(&pngStruct, &pngInfo, nullptr);

    return 0;
}

int PngWrite(const float *src, unsigned char **dst, unsigned int *dstSize,
             unsigned int width, unsigned int height, bool storeAs8bits)
{
    IoHandle handle;
    png_structp pngStruct;
    png_infop pngInfo;

    handle.bufferOffset = 0;
    handle.bufferSize = 0;
    handle.bufferPtr = nullptr;

    pngStruct = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!pngStruct)
        return -1;

    pngInfo = png_create_info_struct(pngStruct);
    if (!pngInfo)
    {
        png_destroy_write_struct(&pngStruct, &pngInfo);
        return -1;
    }

    int bits = 16;
    float scale = 65535.0f;
    if (storeAs8bits)
    {
        bits = 8;
        scale = 255.0f;
    }
    png_set_IHDR(pngStruct, pngInfo, width, height,
                 bits, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_set_write_fn(pngStruct, &handle, WriteFunction, nullptr);

    png_write_info(pngStruct, pngInfo);

    png_set_swap(pngStruct);

    unsigned char lineData8[width * 4];
    unsigned short lineData16[width * 4];
    int srcOffset = 0;
    for (png_uint_32 y = 0; y < height; y++)
    {
        if (bits == 8)
        {
            int lineOffset = 0;
            for (png_uint_32 x = 0; x < width; x++)
            {
                lineData8[lineOffset++] = roundf(src[srcOffset + 0] * scale);
                lineData8[lineOffset++] = roundf(src[srcOffset + 1] * scale);
                lineData8[lineOffset++] = roundf(src[srcOffset + 2] * scale);
                lineData8[lineOffset++] = roundf(src[srcOffset + 3] * scale);
                srcOffset += 4;
            }
            png_write_row(pngStruct, (png_bytep)lineData8);
        }
        else
        {
            int lineOffset = 0;
            for (png_uint_32 x = 0; x < width; x++)
            {
                lineData16[lineOffset++] = roundf(src[srcOffset + 0] * scale);
                lineData16[lineOffset++] = roundf(src[srcOffset + 1] * scale);
                lineData16[lineOffset++] = roundf(src[srcOffset + 2] * scale);
                lineData16[lineOffset++] = roundf(src[srcOffset + 3] * scale);
                srcOffset += 4;
            }
            png_write_row(pngStruct, (png_bytep)lineData16);
        }
    }

    png_write_end(pngStruct, nullptr);

    png_destroy_write_struct(&pngStruct, &pngInfo);

    *dst = handle.bufferPtr;
    *dstSize = handle.bufferOffset;

    return 0;
}
