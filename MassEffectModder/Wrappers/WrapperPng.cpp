/*
 * Wrapper PNG
 *
 * Copyright (C) 2018 Pawel Kolodziejski <aquadran at users.sourceforge.net>
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
             unsigned char **dst, unsigned int *dstSize,
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

    if (png_get_IHDR(pngStruct, pngInfo, &pngWidth, &pngHeight, &bits,
                     &colorType, nullptr, nullptr, nullptr) == 0)
    {
        png_destroy_read_struct(&pngStruct, &pngInfo, nullptr);
        return -1;
    }
    *width = pngWidth;
    *height = pngHeight;

    if (colorType != PNG_COLOR_TYPE_RGB &&
        colorType != PNG_COLOR_TYPE_RGB_ALPHA &&
        bits != 8)
    {
        png_destroy_read_struct(&pngStruct, &pngInfo, nullptr);
        return -1;
    }

    *dstSize = pngWidth * pngHeight * 4;
    unsigned char *dstPtr = *dst = new unsigned char[*dstSize];
    if (*dst == nullptr)
    {
        png_destroy_read_struct(&pngStruct, &pngInfo, nullptr);
        return -1;
    }

    unsigned char lineData[png_get_rowbytes(pngStruct, pngInfo)];
    int dstOffset = 0;
    for (png_uint_32 y = 0; y < pngHeight; y++)
    {
        png_read_row(pngStruct, (png_bytep)lineData, nullptr);
        int lineOffset = 0;
        if (colorType == PNG_COLOR_TYPE_RGB_ALPHA)
        {
            for (png_uint_32 x = 0; x < pngWidth; x++)
            {
                dstPtr[dstOffset + 2] = lineData[lineOffset++];
                dstPtr[dstOffset + 1] = lineData[lineOffset++];
                dstPtr[dstOffset + 0] = lineData[lineOffset++];
                dstPtr[dstOffset + 3] = lineData[lineOffset++];
                dstOffset += 4;
            }
        }
        else
        {
            for (png_uint_32 x = 0; x < pngWidth; x++)
            {
                dstPtr[dstOffset + 2] = lineData[lineOffset++];
                dstPtr[dstOffset + 1] = lineData[lineOffset++];
                dstPtr[dstOffset + 0] = lineData[lineOffset++];
                dstPtr[dstOffset + 3] = 255;
                dstOffset += 4;
            }
        }
    }

    png_destroy_read_struct(&pngStruct, &pngInfo, nullptr);

    return 0;
}

int PngWrite(const unsigned char *src, unsigned char **dst, unsigned int *dstSize,
               unsigned int width, unsigned int height)
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

    png_set_IHDR(pngStruct, pngInfo, width, height,
                 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_set_write_fn(pngStruct, &handle, WriteFunction, nullptr);

    png_write_info(pngStruct, pngInfo);

    unsigned char lineData[width * 4];
    int srcOffset = 0;
    for (png_uint_32 y = 0; y < height; y++)
    {
        int lineOffset = 0;
        for (png_uint_32 x = 0; x < width; x++)
        {
            lineData[lineOffset++] = src[srcOffset + 2];
            lineData[lineOffset++] = src[srcOffset + 1];
            lineData[lineOffset++] = src[srcOffset + 0];
            lineData[lineOffset++] = src[srcOffset + 3];
            srcOffset += 4;
        }
        png_write_row(pngStruct, (png_bytep)lineData);
    }

    png_write_end(pngStruct, nullptr);

    png_destroy_write_struct(&pngStruct, &pngInfo);

    *dst = handle.bufferPtr;
    *dstSize = handle.bufferOffset;

    return 0;
}
