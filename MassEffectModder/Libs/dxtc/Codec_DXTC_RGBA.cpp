//===============================================================================
// Copyright (c) 2017-2019 Pawel Kolodziejski
// Copyright (c) 2007-2016  Advanced Micro Devices, Inc. All rights reserved.
// Copyright (c) 2004-2006 ATI Technologies Inc.
//===============================================================================
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////////

#include "Common.h"
#include "CompressonatorXCodec.h"

#define DXTC_OFFSET_ALPHA 0
#define DXTC_OFFSET_RGB 2

void DxtcCompressExplicitAlphaBlock(const CODECFLOAT alphaBlock[BLOCK_SIZE_4X4], CODEC_DWORD compressedBlock[2]);
void DxtcDecompressExplicitAlphaBlock(CODECFLOAT alphaBlock[BLOCK_SIZE_4X4], const CODEC_DWORD compressedBlock[2]);

void DxtcCompressAlphaBlock(CODECFLOAT alphaBlock[BLOCK_SIZE_4X4], CODEC_DWORD compressedBlock[2]);
void DxtcDecompressAlphaBlock(CODECFLOAT alphaBlock[BLOCK_SIZE_4X4], CODEC_DWORD compressedBlock[2]);

#define ConstructColour(r, g, b)  (((r) << 11) | ((g) << 5) | (b))

/*
Channel Bits
*/
#define RG 5
#define GG 6
#define BG 5

void DxtcCompressRGBBlock(CODECFLOAT rgbBlock[BLOCK_SIZE_4X4X4], CODEC_DWORD compressedBlock[2],
                          bool bDXT1 = false, bool bDXT1UseAlpha = false, float nDXT1AlphaThreshold = 0)
{
    /*
    ARGB Channel indexes
    */

    int RC = 2, GC = 1, BC = 0;

    if (bDXT1)
    {
        CODEC_BYTE nEndpoints[2][3][2];
        CODEC_BYTE nIndices[2][BLOCK_SIZE_4X4];

        double fError3 = CompRGBBlock(rgbBlock, BLOCK_SIZE_4X4, RG, GG, BG, nEndpoints[0], nIndices[0], 3, true, false, 1, nullptr, bDXT1UseAlpha, nDXT1AlphaThreshold);
        double fError4 = (fError3 == 0.0) ? FLT_MAX : CompRGBBlock(rgbBlock, BLOCK_SIZE_4X4, RG, GG, BG, nEndpoints[1], nIndices[1], 4, true, false, 1, nullptr, bDXT1UseAlpha, nDXT1AlphaThreshold);

        unsigned int nMethod = (fError3 <= fError4) ? 0 : 1;
        unsigned int c0 = ConstructColour((nEndpoints[nMethod][RC][0] >> (8 - RG)), (nEndpoints[nMethod][GC][0] >> (8 - GG)), (nEndpoints[nMethod][BC][0] >> (8 - BG)));
        unsigned int c1 = ConstructColour((nEndpoints[nMethod][RC][1] >> (8 - RG)), (nEndpoints[nMethod][GC][1] >> (8 - GG)), (nEndpoints[nMethod][BC][1] >> (8 - BG)));
        if ((nMethod == 1 && c0 <= c1) || (nMethod == 0 && c0 > c1))
            compressedBlock[0] = c1 | (c0 << 16);
        else
            compressedBlock[0] = c0 | (c1 << 16);

        compressedBlock[1] = 0;
        for (int i = 0; i < 16; i++)
            compressedBlock[1] |= (nIndices[nMethod][i] << (2 * i));
    }
    else
    {
        CODEC_BYTE nEndpoints[3][2];
        CODEC_BYTE nIndices[BLOCK_SIZE_4X4];

        CompRGBBlock(rgbBlock, BLOCK_SIZE_4X4, RG, GG, BG, nEndpoints, nIndices, 4, true, false, 1, nullptr, bDXT1UseAlpha, nDXT1AlphaThreshold);

        unsigned int c0 = ConstructColour((nEndpoints[RC][0] >> (8 - RG)), (nEndpoints[GC][0] >> (8 - GG)), (nEndpoints[BC][0] >> (8 - BG)));
        unsigned int c1 = ConstructColour((nEndpoints[RC][1] >> (8 - RG)), (nEndpoints[GC][1] >> (8 - GG)), (nEndpoints[BC][1] >> (8 - BG)));
        if (c0 <= c1)
            compressedBlock[0] = c1 | (c0 << 16);
        else
            compressedBlock[0] = c0 | (c1 << 16);

        compressedBlock[1] = 0;
        for (int i = 0; i<16; i++)
            compressedBlock[1] |= (nIndices[i] << (2 * i));
    }
}

// This function decompresses a DXT colour block
// The block is decompressed to 8 bits per channel
void DxtcDecompressRGBBlock(CODECFLOAT rgbBlock[BLOCK_SIZE_4X4X4], const CODEC_DWORD compressedBlock[2], bool bDXT1)
{
    CODEC_DWORD n0 = compressedBlock[0] & 0xffff;
    CODEC_DWORD n1 = compressedBlock[0] >> 16;
    CODEC_DWORD r0;
    CODEC_DWORD g0;
    CODEC_DWORD b0;
    CODEC_DWORD r1;
    CODEC_DWORD g1;
    CODEC_DWORD b1;

    r0 = ((n0 & 0xf800) >> 8);
    g0 = ((n0 & 0x07e0) >> 3);
    b0 = ((n0 & 0x001f) << 3);

    r1 = ((n1 & 0xf800) >> 8);
    g1 = ((n1 & 0x07e0) >> 3);
    b1 = ((n1 & 0x001f) << 3);

    // Apply the lower bit replication to give full dynamic range
    r0 += (r0 >> 5); r1 += (r1 >> 5);
    g0 += (g0 >> 6); g1 += (g1 >> 6);
    b0 += (b0 >> 5); b1 += (b1 >> 5);

    CODECFLOAT c0[4], c1[4], c2[4], c3[4];
    c0[RGBA32F_OFFSET_A] = 1.0f;
    c0[RGBA32F_OFFSET_R] = CONVERT_BYTE_TO_FLOAT(r0);
    c0[RGBA32F_OFFSET_G] = CONVERT_BYTE_TO_FLOAT(g0);
    c0[RGBA32F_OFFSET_B] = CONVERT_BYTE_TO_FLOAT(b0);

    c1[RGBA32F_OFFSET_A] = 1.0f;
    c1[RGBA32F_OFFSET_R] = CONVERT_BYTE_TO_FLOAT(r1);
    c1[RGBA32F_OFFSET_G] = CONVERT_BYTE_TO_FLOAT(g1);
    c1[RGBA32F_OFFSET_B] = CONVERT_BYTE_TO_FLOAT(b1);

    if (!bDXT1 || n0 > n1)
    {
        c2[RGBA32F_OFFSET_A] = 1.0f;
        c2[RGBA32F_OFFSET_R] = ((2 * c0[RGBA32F_OFFSET_R] + c1[RGBA32F_OFFSET_R]) / 3);
        c2[RGBA32F_OFFSET_G] = ((2 * c0[RGBA32F_OFFSET_G] + c1[RGBA32F_OFFSET_G]) / 3);
        c2[RGBA32F_OFFSET_B] = ((2 * c0[RGBA32F_OFFSET_B] + c1[RGBA32F_OFFSET_B]) / 3);

        c3[RGBA32F_OFFSET_A] = 1.0f;
        c3[RGBA32F_OFFSET_R] = ((2 * c1[RGBA32F_OFFSET_R] + c0[RGBA32F_OFFSET_R]) / 3);
        c3[RGBA32F_OFFSET_G] = ((2 * c1[RGBA32F_OFFSET_G] + c0[RGBA32F_OFFSET_G]) / 3);
        c3[RGBA32F_OFFSET_B] = ((2 * c1[RGBA32F_OFFSET_B] + c0[RGBA32F_OFFSET_B]) / 3);

        for (int i = 0; i < 16; i++)
        {
            switch ((compressedBlock[1] >> (2 * i)) & 3)
            {
                case 0:
                    memcpy(&rgbBlock[i * 4], c0, 4 * sizeof(CODECFLOAT));
                    break;
                case 1:
                    memcpy(&rgbBlock[i * 4], c1, 4 * sizeof(CODECFLOAT));
                    break;
                case 2:
                    memcpy(&rgbBlock[i * 4], c2, 4 * sizeof(CODECFLOAT));
                    break;
                case 3:
                    memcpy(&rgbBlock[i * 4], c3, 4 * sizeof(CODECFLOAT));
                    break;
            }
        }
    }
    else
    {
        // Transparent decode
        c2[RGBA32F_OFFSET_A] = 1.0f;
        c2[RGBA32F_OFFSET_R] = ((c0[RGBA32F_OFFSET_R] + c1[RGBA32F_OFFSET_R]) / 2);
        c2[RGBA32F_OFFSET_G] = ((c0[RGBA32F_OFFSET_G] + c1[RGBA32F_OFFSET_G]) / 2);
        c2[RGBA32F_OFFSET_B] = ((c0[RGBA32F_OFFSET_B] + c1[RGBA32F_OFFSET_B]) / 2);

        c3[RGBA32F_OFFSET_A] = 0.0f;
        c3[RGBA32F_OFFSET_R] = 0.0f;
        c3[RGBA32F_OFFSET_G] = 0.0f;
        c3[RGBA32F_OFFSET_B] = 0.0f;

        for (int i = 0; i < 16; i++)
        {
            switch ((compressedBlock[1] >> (2 * i)) & 3)
            {
                case 0:
                    memcpy(&rgbBlock[i * 4], c0, 4 * sizeof(CODECFLOAT));
                    break;
                case 1:
                    memcpy(&rgbBlock[i * 4], c1, 4 * sizeof(CODECFLOAT));
                    break;
                case 2:
                    memcpy(&rgbBlock[i * 4], c2, 4 * sizeof(CODECFLOAT));
                    break;
                case 3:
                    memcpy(&rgbBlock[i * 4], c3, 4 * sizeof(CODECFLOAT));
                    break;
            }
        }
    }
}

void DxtcCompressRGBABlock(CODECFLOAT rgbaBlock[BLOCK_SIZE_4X4X4], CODEC_DWORD compressedBlock[4])
{
    CODECFLOAT alphaBlock[BLOCK_SIZE_4X4];
    for (CODEC_DWORD i = 0; i < 16; i++)
        alphaBlock[i] = rgbaBlock[(i * 4) + RGBA32F_OFFSET_A];

    DxtcCompressAlphaBlock(alphaBlock, &compressedBlock[DXTC_OFFSET_ALPHA]);

    DxtcCompressRGBBlock(rgbaBlock, &compressedBlock[DXTC_OFFSET_RGB], false, false);
}

void DxtcDecompressRGBABlock(CODECFLOAT rgbaBlock[BLOCK_SIZE_4X4X4], CODEC_DWORD compressedBlock[4])
{
    CODECFLOAT alphaBlock[BLOCK_SIZE_4X4];

    DxtcDecompressAlphaBlock(alphaBlock, &compressedBlock[DXTC_OFFSET_ALPHA]);
    DxtcDecompressRGBBlock(rgbaBlock, &compressedBlock[DXTC_OFFSET_RGB], false);

    for (CODEC_DWORD i = 0; i < 16; i++)
        rgbaBlock[(i * 4) + RGBA32F_OFFSET_A] = alphaBlock[i];
}

void DxtcCompressRGBABlock_ExplicitAlpha(CODECFLOAT rgbaBlock[BLOCK_SIZE_4X4X4], CODEC_DWORD compressedBlock[4])
{
    CODECFLOAT alphaBlock[BLOCK_SIZE_4X4];
    for (CODEC_DWORD i = 0; i < 16; i++)
        alphaBlock[i] = rgbaBlock[(i * 4) + RGBA32F_OFFSET_A];

    DxtcCompressExplicitAlphaBlock(alphaBlock, &compressedBlock[DXTC_OFFSET_ALPHA]);

    DxtcCompressRGBBlock(rgbaBlock, &compressedBlock[DXTC_OFFSET_RGB], false, false);
}

void DxtcDecompressRGBABlock_ExplicitAlpha(CODECFLOAT rgbaBlock[BLOCK_SIZE_4X4X4], CODEC_DWORD compressedBlock[4])
{
    CODECFLOAT alphaBlock[BLOCK_SIZE_4X4];

    DxtcDecompressExplicitAlphaBlock(alphaBlock, &compressedBlock[DXTC_OFFSET_ALPHA]);
    DxtcDecompressRGBBlock(rgbaBlock, &compressedBlock[DXTC_OFFSET_RGB], false);

    for (CODEC_DWORD i = 0; i < 16; i++)
        rgbaBlock[(i * 4) + RGBA32F_OFFSET_A] = alphaBlock[i];
}
