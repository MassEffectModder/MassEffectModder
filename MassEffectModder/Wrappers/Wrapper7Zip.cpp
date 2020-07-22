/*
 * Wrapper 7Zip
 *
 * Copyright (C) 2015-2020 Pawel Kolodziejski
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

#include <LzmaLib.h>
#include <cstring>
#include <memory>

int LzmaDecompress(unsigned char *src, unsigned int src_len, unsigned char *dst, unsigned int *dst_len)
{
    size_t len = *dst_len, sLen = src_len - LZMA_PROPS_SIZE;

    int status = LzmaUncompress(dst, &len, &src[LZMA_PROPS_SIZE], &sLen, src, LZMA_PROPS_SIZE);
    if (status == SZ_OK)
        *dst_len = static_cast<unsigned int>(len);

    return status;
}

int LzmaCompress(unsigned char *src, unsigned int src_len,
                 unsigned char **dst, unsigned int *dst_len, int compress_level)
{
    size_t propsSize = LZMA_PROPS_SIZE;
    size_t destLen = (src_len * 2) + 128;
    auto *tmpbuf = new unsigned char[propsSize + destLen];
    if (tmpbuf == nullptr)
        return -100;

    int status = LzmaCompressData(&tmpbuf[LZMA_PROPS_SIZE],
                              &destLen,
                              (const unsigned char *)src,
                              (size_t)src_len,
                              tmpbuf,
                              &propsSize,
                              compress_level, 0, -1, -1, -1, -1, 1);
    if (status == SZ_OK)
    {
        *dst = new unsigned char[destLen + LZMA_PROPS_SIZE];
        if (*dst == nullptr)
        {
            delete[] tmpbuf;
            return -100;
        }
        memcpy(*dst, tmpbuf, destLen + LZMA_PROPS_SIZE);
        *dst_len = static_cast<unsigned int>(destLen + LZMA_PROPS_SIZE);
    }
    else
    {
        *dst_len = 0;
    }

    delete[] tmpbuf;

    return status;
}

#ifdef _WIN32
#include <windows.h>
BOOL WINAPI DllMain(HINSTANCE /*hin*/, DWORD /*reason*/, LPVOID /*lpvReserved*/) { return TRUE; }
#define SEVENZIP_EXPORT __declspec(dllexport)
#else
#define SEVENZIP_EXPORT
#endif

extern "C" {
SEVENZIP_EXPORT int SevenZipDecompress(unsigned char *src, unsigned int src_len, unsigned char *dst, unsigned int *dst_len)
{
    size_t len = *dst_len, sLen = src_len - LZMA_PROPS_SIZE;

    int status = LzmaUncompress(dst, &len, &src[LZMA_PROPS_SIZE], &sLen, src, LZMA_PROPS_SIZE);
    if (status == SZ_OK)
        *dst_len = len;

    return status;
}

SEVENZIP_EXPORT int SevenZipCompress(int compression_level, unsigned char *src, unsigned int src_len,
                                     unsigned char *dst, unsigned int *dst_len)
{
    size_t len = *dst_len, propsSize = LZMA_PROPS_SIZE;
    int status = LzmaCompressData(&dst[LZMA_PROPS_SIZE], &len, src, src_len, dst, &propsSize, compression_level,
                               1 << 16, -1, -1, -1, -1, -1);
    if (status == SZ_OK)
        *dst_len = len + LZMA_PROPS_SIZE;

    return status;
}

}

extern "C" {
#if defined(_WIN32)
int sevenzip_unpack(const wchar_t *path, const wchar_t *output_path, int full_path);
#else
int sevenzip_unpack(const char *path, const char *output_path, int full_path);
#endif
}

int SevenZipUnpack(const void *path, const void *output_path, bool full_path)
{
#if defined(_WIN32)
    return sevenzip_unpack(static_cast<const wchar_t *>(path),
                        static_cast<const wchar_t *>(output_path), (int)full_path);
#else
    return sevenzip_unpack(static_cast<const char *>(path),
                        static_cast<const char *>(output_path), (int)full_path);
#endif
}
