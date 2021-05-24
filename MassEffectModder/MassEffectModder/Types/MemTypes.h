/*
 * MassEffectModder
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

#ifndef MEM_TYPES_H
#define MEM_TYPES_H

#define APP_NAME "MEM"

typedef enum
{
    UNKNOWN_TYPE = 0,
    ME1_TYPE,
    ME2_TYPE,
    ME3_TYPE
} MeType;

typedef enum
{
    UnknownPixelFormat, Internal, DXT1, DXT3, DXT5, ATI2, V8U8, ARGB, RGBA, RGB, G8, BC5, BC7
} PixelFormat;

typedef enum
{
    UnknownImageFormat, DDS, PNG, BMP, TGA
} ImageFormat;

typedef enum
{
    Diffuse = 0,
    Normalmap,
    OneBitAlpha,
    GreyScale,
    Displacementmap,
    Movie,
} TextureType;

typedef enum
{
    None = 0,
    MarkToConvert = 1,
} TextureFlags;

typedef enum
{
    LZO = 1,
    Zlib = 2,
    LZMA = 3,
    ZSTD = 4
} CompressionDataType;

#define textureMapBinTag      0x5054454D
#define textureMapBinVersion  1
#define TextureModTag         0x444F4D54
#define TextureModVersion     3
#define FileTextureTag        0x53444446
#define FileMovieTextureTag   0x53494246
#define MEMI_TAG              0x494D454D
#define MEMendFileMarker      "ThisIsMEMEndOfFileMarker"
#define MEMMarkerLength       (sizeof(MEMendFileMarker) - 1)

#define BIK_TAG               0x6A32424B // 'HB2j'

#define PERCENT_OF_SIZE(x, y) (((x) * (y)) / 100)
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif


#endif
