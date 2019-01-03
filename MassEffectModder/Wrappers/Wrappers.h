/*
 * Wrapper
 *
 * Copyright (C) 2017-2018 Pawel Kolodziejski <aquadran at users.sourceforge.net>
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

#ifndef WRAPPERS_H
#define WRAPPERS_H

typedef unsigned char      BYTE;
typedef unsigned int       UINT32;
typedef unsigned long long UINT64;

int LzmaDecompress(BYTE *src, UINT32 src_len, BYTE *dst, UINT32 *dst_len);

int LzoDecompress(BYTE *src, UINT32 src_len, BYTE *dst, UINT32 *dst_len);
int LzoCompress(BYTE *src, UINT32 src_len, BYTE **dst, UINT32 *dst_len);

void *ZipOpenFromFile(const void *path, int *numEntries, int tpf);
void *ZipOpenFromMem(BYTE *src, UINT64 srcLen, int *numEntries, int tpf);
int ZipGetCurrentFileInfo(void *handle, char **fileName, int *sizeOfFileName, UINT64 *dstLen);
int ZipGoToFirstFile(void *handle);
int ZipGoToNextFile(void *handle);
int ZipLocateFile(void *handle, const char *filename);
int ZipReadCurrentFile(void *handle, BYTE *dst, UINT64 dst_len, const BYTE *pass);
void ZipClose(void *handle);

int XDelta3Compress(BYTE *src1, BYTE *src2, UINT32 src_len, BYTE *delta, UINT32 *delta_len);
int XDelta3Decompress(BYTE *src, UINT32 src_len, BYTE *delta, UINT32 delta_len, BYTE *dst, UINT32 *dst_len);

int ZlibDecompress(BYTE *src, UINT32 src_len, BYTE *dst, UINT32 *dst_len);
int ZlibCompress(BYTE *src, UINT32 src_len, BYTE **dst, UINT32 *dst_len, int compression_level = -1);

int PngRead(BYTE *src, UINT32 srcSize,
             BYTE **dst, UINT32 *dstSize,
             UINT32 *width, UINT32 *height);
int PngWrite(const BYTE *src, BYTE **dst, UINT32 *dstSize,
              UINT32 width, UINT32 height);

#define BLOCK_SIZE_4X4        16
#define BLOCK_SIZE_4X4X4      64

void CompressRGBABlock(BYTE rgbaBlock[BLOCK_SIZE_4X4X4], UINT32 compressedBlock[4]);
void DecompressRGBABlock(BYTE rgbaBlock[BLOCK_SIZE_4X4X4], UINT32 compressedBlock[4]);
void CompressRGBABlock_ExplicitAlpha(BYTE rgbaBlock[BLOCK_SIZE_4X4X4], UINT32 compressedBlock[4]);
void DecompressRGBABlock_ExplicitAlpha(BYTE rgbaBlock[BLOCK_SIZE_4X4X4], UINT32 compressedBlock[4]);
void CompressRGBBlock(BYTE rgbBlock[BLOCK_SIZE_4X4X4], UINT32 compressedBlock[2], bool bDXT1 = false,
                      bool bDXT1UseAlpha = false, unsigned char bDXT1UseAlphaThreshold = 0);
void DecompressRGBBlock(BYTE rgbBlock[BLOCK_SIZE_4X4X4], UINT32 compressedBlock[2], bool bDXT1);
void CompressAlphaBlock(BYTE alphaBlock[BLOCK_SIZE_4X4], UINT32 compressedBlock[2]);
void DecompressAlphaBlock(BYTE alphaBlock[BLOCK_SIZE_4X4], UINT32 compressedBlock[2]);

#endif
