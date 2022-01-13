/*
 * Wrapper DXTc
 *
 * Copyright (C) 2014-2022 Pawel Kolodziejski
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

#ifdef EXPORT_LIBS
#ifdef _WIN32
#define LIB_EXPORT extern "C" __declspec(dllexport)
#else
#define LIB_EXPORT extern "C"
#endif
#else
#define LIB_EXPORT
#endif

#define BLOCK_SIZE_4X4        16
#define BLOCK_SIZE_4X4X4      64

typedef unsigned int        CODEC_DWORD;         ///< A 32-bit integer format.
typedef unsigned char       CODEC_BYTE;          ///< An 8-bit integer format.

void DxtcCompressRGBBlock(CODEC_BYTE rgbBlock[BLOCK_SIZE_4X4X4], CODEC_DWORD compressedBlock[2], bool bDXT1 = false, bool bDXT1UseAlpha = false, CODEC_BYTE nDXT1AlphaThreshold = 0);
void DxtcDecompressRGBBlock(CODEC_BYTE rgbBlock[BLOCK_SIZE_4X4X4], CODEC_DWORD compressedBlock[2], bool bDXT1);
void DxtcCompressRGBABlock(CODEC_BYTE rgbaBlock[BLOCK_SIZE_4X4X4], CODEC_DWORD compressedBlock[4]);
void DxtcDecompressRGBABlock(CODEC_BYTE rgbaBlock[BLOCK_SIZE_4X4X4], CODEC_DWORD compressedBlock[4]);
void DxtcCompressRGBABlock_ExplicitAlpha(CODEC_BYTE rgbaBlock[BLOCK_SIZE_4X4X4], CODEC_DWORD compressedBlock[4]);
void DxtcDecompressRGBABlock_ExplicitAlpha(CODEC_BYTE rgbaBlock[BLOCK_SIZE_4X4X4], CODEC_DWORD compressedBlock[4]);

void DxtcCompressExplicitAlphaBlock(CODEC_BYTE alphaBlock[BLOCK_SIZE_4X4], CODEC_DWORD compressedBlock[2]);
void DxtcDecompressExplicitAlphaBlock(CODEC_BYTE alphaBlock[BLOCK_SIZE_4X4], CODEC_DWORD compressedBlock[2]);
void DxtcCompressAlphaBlock(CODEC_BYTE alphaBlock[BLOCK_SIZE_4X4], CODEC_DWORD compressedBlock[2]);
void DxtcDecompressAlphaBlock(CODEC_BYTE alphaBlock[BLOCK_SIZE_4X4], CODEC_DWORD compressedBlock[2]);

LIB_EXPORT void CompressAlphaBlock(CODEC_BYTE alphaBlock[BLOCK_SIZE_4X4], CODEC_DWORD compressedBlock[2])
{
    DxtcCompressAlphaBlock(alphaBlock, compressedBlock);
}

LIB_EXPORT void DecompressExplicitAlphaBlock(CODEC_BYTE alphaBlock[BLOCK_SIZE_4X4], CODEC_DWORD compressedBlock[2])
{
    DxtcDecompressExplicitAlphaBlock(alphaBlock, compressedBlock);
}

LIB_EXPORT void DecompressAlphaBlock(CODEC_BYTE alphaBlock[BLOCK_SIZE_4X4], CODEC_DWORD compressedBlock[2])
{
    DxtcDecompressAlphaBlock(alphaBlock, compressedBlock);
}

LIB_EXPORT void CompressExplicitAlphaBlock(CODEC_BYTE alphaBlock[BLOCK_SIZE_4X4], CODEC_DWORD compressedBlock[2])
{
    DxtcCompressExplicitAlphaBlock(alphaBlock, compressedBlock);
}

LIB_EXPORT void CompressRGBBlock(CODEC_BYTE rgbBlock[BLOCK_SIZE_4X4X4], CODEC_DWORD compressedBlock[2], bool bDXT1, bool bDXT1UseAlpha, CODEC_BYTE nDXT1AlphaThreshold)
{
    DxtcCompressRGBBlock(rgbBlock, compressedBlock, bDXT1, bDXT1UseAlpha, nDXT1AlphaThreshold);
}

LIB_EXPORT void DecompressRGBBlock(CODEC_BYTE rgbBlock[BLOCK_SIZE_4X4X4], CODEC_DWORD compressedBlock[2], bool bDXT1)
{
    DxtcDecompressRGBBlock(rgbBlock, compressedBlock, bDXT1);
}

LIB_EXPORT void CompressRGBABlock(CODEC_BYTE rgbaBlock[BLOCK_SIZE_4X4X4], CODEC_DWORD compressedBlock[4])
{
    DxtcCompressRGBABlock(rgbaBlock, compressedBlock);
}

LIB_EXPORT void DecompressRGBABlock(CODEC_BYTE rgbaBlock[BLOCK_SIZE_4X4X4], CODEC_DWORD compressedBlock[4])
{
    DxtcDecompressRGBABlock(rgbaBlock, compressedBlock);
}

LIB_EXPORT void CompressRGBABlock_ExplicitAlpha(CODEC_BYTE rgbaBlock[BLOCK_SIZE_4X4X4], CODEC_DWORD compressedBlock[4])
{
    DxtcCompressRGBABlock_ExplicitAlpha(rgbaBlock, compressedBlock);
}

LIB_EXPORT void DecompressRGBABlock_ExplicitAlpha(CODEC_BYTE rgbaBlock[BLOCK_SIZE_4X4X4], CODEC_DWORD compressedBlock[4])
{
    DxtcDecompressRGBABlock_ExplicitAlpha(rgbaBlock, compressedBlock);
}
