/*
 * Wrapper
 *
 * Copyright (C) 2017-2022 Pawel Kolodziejski
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

class BC7BlockEncoder;
class BC7BlockDecoder;

int LzmaDecompress(BYTE *src, UINT32 src_len, BYTE *dst, UINT32 *dst_len);
int LzmaCompress(BYTE *src, UINT32 src_len, BYTE **dst, UINT32 *dst_len, int compress_level = 5);

void *ZipOpenFromFile(const void *path, int *numEntries, int tpf);
void *ZipOpenFromMem(BYTE *src, UINT64 srcLen, int *numEntries, int tpf);
int ZipGetCurrentFileInfo(void *handle, char *fileName,
                          int sizeOfFileName, unsigned long long *dstLen,
                          unsigned long *fileFlags);
int ZipGoToFirstFile(void *handle);
int ZipGoToNextFile(void *handle);
int ZipLocateFile(void *handle, const char *filename);
int ZipReadCurrentFile(void *handle, BYTE *dst, UINT64 dst_len, const BYTE *pass);
void ZipClose(void *handle);
int ZipUnpack(const void *path, const void *output_path, const void *filter,
              bool full_path, bool ipc = false);
int ZipList(const void *path, bool ipc = false);

int SevenZipUnpack(const void *path, const void *output_path, const void *filter,
                   bool full_path, bool ipc = false);
int SevenZipList(const void *path, bool ipc = false);

int RarUnpack(const void *path, const void *output_path, const void *filter, bool full_path, bool ipc = false);
int RarList(const void *path, bool ipc = false);

int ZlibDecompress(BYTE *src, UINT32 src_len, BYTE *dst, UINT32 *dst_len);
int ZlibCompress(BYTE *src, UINT32 src_len, BYTE **dst, UINT32 *dst_len, int compression_level = -1);

#ifdef WIN32
bool OodleInitLib(const wchar_t *libPath);
#else
bool OodleInitLib(const char *libPath);
#endif
void OodleUninitLib();
bool OodleIsCompressionSupported();
int OodleDecompress(BYTE *src, UINT32 src_len, BYTE *dst, UINT32 dst_len);
int OodleCompress(BYTE *src, UINT32 src_len, BYTE **dst, UINT32 *dst_len);

int ZstdDecompress(BYTE *src, UINT32 src_len, BYTE *dst, UINT32 *dst_len);
int ZstdCompress(BYTE *src, UINT32 src_len, BYTE **dst, UINT32 *dst_len, int compression_level = 3);

int LzxDecompress(BYTE *src, UINT32 src_len, BYTE *dst, const UINT32 *dst_len);

int PngRead(BYTE *src, UINT32 srcSize,
             float **dst, UINT32 *dstSize,
             UINT32 *width, UINT32 *height, bool &source8Bits);
int PngWrite(const float *src, BYTE **dst, UINT32 *dstSize,
              UINT32 width, UINT32 height, bool storeAs8bits = true);

#define BLOCK_SIZE_4X4        16
#define BLOCK_SIZE_4X4X4      64
#define BLOCK_SIZE_4X4X2      32

void DxtcCompressRGBABlock(float rgbaBlock[BLOCK_SIZE_4X4X4], UINT32 compressedBlock[4]);
void DxtcDecompressRGBABlock(float rgbaBlock[BLOCK_SIZE_4X4X4], UINT32 compressedBlock[4]);
void DxtcCompressRGBABlock_ExplicitAlpha(float rgbaBlock[BLOCK_SIZE_4X4X4], UINT32 compressedBlock[4]);
void DxtcDecompressRGBABlock_ExplicitAlpha(float rgbaBlock[BLOCK_SIZE_4X4X4], UINT32 compressedBlock[4]);
void DxtcCompressRGBBlock(float rgbBlock[BLOCK_SIZE_4X4X4], UINT32 compressedBlock[2], bool bDXT1 = false,
                      bool bDXT1UseAlpha = false, float bDXT1UseAlphaThreshold = 0);
void DxtcDecompressRGBBlock(float rgbBlock[BLOCK_SIZE_4X4X4], const UINT32 compressedBlock[2], bool bDXT1);
void DxtcCompressAlphaBlock(float alphaBlock[BLOCK_SIZE_4X4], UINT32 compressedBlock[2]);
void DxtcDecompressAlphaBlock(float alphaBlock[BLOCK_SIZE_4X4], UINT32 compressedBlock[2]);

int BC7InitializeLibrary();
int BC7ShutdownLibrary();
int BC7CreateEncoder(double quality, bool restrictColour, bool restrictAlpha, UINT32 modeMask, double performance, BC7BlockEncoder **encoder);
int BC7CreateDecoder(BC7BlockDecoder **decoder);
int BC7DestoyEncoder(BC7BlockEncoder *encoder);
int BC7DestoyDecoder(BC7BlockDecoder *decoder);
int BC7CompressBlock(BC7BlockEncoder *encoder, double in[BLOCK_SIZE_4X4][4], BYTE *out);
int BC7DecompressBlock(BC7BlockDecoder *decoder, BYTE *in, double out[BLOCK_SIZE_4X4][4]);

void BacktraceGetFilename(char *dst, const char *src, int maxLen);
int BacktraceGetInfoFromModule(char *moduleFilePath, UINT64 offset, char *sourceFile,
    char *sourceFunc, UINT32 *sourceLine);
bool GetBackTrace(std::string &output, bool exceptionMode, bool crashMode);

#endif
