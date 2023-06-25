/*
 * Wrapper BC7
 *
 * Copyright (C) 2021 Pawel Kolodziejski
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

enum {
    BLOCK_SIZE_4X4 = 16
};

// Number of image components
enum {
    BC7_COMPONENT_COUNT = 4
};

typedef unsigned int        CODEC_DWORD;         ///< A 32-bit integer format.
typedef unsigned char       CODEC_BYTE;          ///< An 8-bit integer format.

class BC7BlockEncoder;
class BC7BlockDecoder;

int CMP_InitializeBC7Library();
int CMP_ShutdownBC7Library();
int CMP_CreateBC7Encoder(double quality, bool restrictColour, bool restrictAlpha, CODEC_DWORD modeMask, double performance, BC7BlockEncoder **encoder);
int CMP_CreateBC7Decoder(BC7BlockDecoder **decoder);
int CMP_DestroyBC7Encoder(BC7BlockEncoder *encoder);
int CMP_DestroyBC7Decoder(BC7BlockDecoder *decoder);
int CMP_EncodeBC7Block(BC7BlockEncoder *encoder, double in[BLOCK_SIZE_4X4][BC7_COMPONENT_COUNT], CODEC_BYTE *out);
int CMP_DecodeBC7Block(BC7BlockDecoder *decoder, CODEC_BYTE *in, double out[BLOCK_SIZE_4X4][BC7_COMPONENT_COUNT]);

LIB_EXPORT int BC7InitializeLibrary()
{
    return CMP_InitializeBC7Library();
}

LIB_EXPORT int BC7ShutdownLibrary()
{
    return CMP_ShutdownBC7Library();
}

LIB_EXPORT int BC7CreateEncoder(double quality, bool restrictColour, bool restrictAlpha, CODEC_DWORD modeMask, double performance, BC7BlockEncoder **encoder)
{
    return CMP_CreateBC7Encoder(quality, restrictColour, restrictAlpha, modeMask, performance, encoder);
}

LIB_EXPORT int BC7CreateDecoder(BC7BlockDecoder **decoder)
{
    return CMP_CreateBC7Decoder(decoder);
}

LIB_EXPORT int BC7DestoyEncoder(BC7BlockEncoder *encoder)
{
    return CMP_DestroyBC7Encoder(encoder);
}

LIB_EXPORT int BC7DestoyDecoder(BC7BlockDecoder *decoder)
{
    return CMP_DestroyBC7Decoder(decoder);
}

LIB_EXPORT int BC7CompressBlock(BC7BlockEncoder *encoder, double in[BLOCK_SIZE_4X4][BC7_COMPONENT_COUNT], CODEC_BYTE *out)
{
    return CMP_EncodeBC7Block(encoder, in, out);
}

LIB_EXPORT int BC7DecompressBlock(BC7BlockDecoder *decoder, CODEC_BYTE *in, double out[BLOCK_SIZE_4X4][BC7_COMPONENT_COUNT])
{
    return CMP_DecodeBC7Block(decoder, in, out);
}
