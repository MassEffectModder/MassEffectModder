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

#ifndef IMAGE_H
#define IMAGE_H

#include <Types/MemTypes.h>
#include <MipMaps/MipMap.h>
#include <Helpers/Stream.h>
#include <Helpers/ByteBuffer.h>

#define DDS_TAG                  0x20534444
#define DDS_HEADER_dwSize        124
#define DDS_PIXELFORMAT_dwSize   32

#define DDPF_ALPHAPIXELS         0x1
#define DDPF_FOURCC              0x4
#define DDPF_RGB                 0x40
#define DDPF_LUMINANCE           0x20000
#define DDPF_SIGNED              0x80000

#define FOURCC_DX10_TAG          0x30315844
#define FOURCC_DXT1_TAG          0x31545844
#define FOURCC_DXT3_TAG          0x33545844
#define FOURCC_DXT5_TAG          0x35545844
#define FOURCC_ATI2_TAG          0x32495441

#define DDSD_CAPS                0x1
#define DDSD_HEIGHT              0x2
#define DDSD_WIDTH               0x4
#define DDSD_PIXELFORMAT         0x1000
#define DDSD_MIPMAPCOUNT         0x20000
#define DDSD_LINEARSIZE          0x80000

#define DDSCAPS_COMPLEX          0x8
#define DDSCAPS_TEXTURE          0x1000
#define DDSCAPS_MIPMAP           0x400000

#define BLOCK_SIZE_4X4           16
#define BLOCK_SIZE_4X4X4         64
#define BLOCK_SIZE_4X4BPP4       8
#define BLOCK_SIZE_4X4BPP8       16

typedef enum DDS_FORMAT {
    DDS_FORMAT_UNKNOWN = 0x0,
    DDS_FORMAT_R32G32B32A32_TYPELESS = 0x1,
    DDS_FORMAT_R32G32B32A32_FLOAT = 0x2,
    DDS_FORMAT_R32G32B32A32_UINT = 0x3,
    DDS_FORMAT_R32G32B32A32_SINT = 0x4,
    DDS_FORMAT_R32G32B32_TYPELESS = 0x5,
    DDS_FORMAT_R32G32B32_FLOAT = 0x6,
    DDS_FORMAT_R32G32B32_UINT = 0x7,
    DDS_FORMAT_R32G32B32_SINT = 0x8,
    DDS_FORMAT_R16G16B16A16_TYPELESS = 0x9,
    DDS_FORMAT_R16G16B16A16_FLOAT = 0xa,
    DDS_FORMAT_R16G16B16A16_UNORM = 0xb,
    DDS_FORMAT_R16G16B16A16_UINT = 0xc,
    DDS_FORMAT_R16G16B16A16_SNORM = 0xd,
    DDS_FORMAT_R16G16B16A16_SINT = 0xe,
    DDS_FORMAT_R32G32_TYPELESS = 0xf,
    DDS_FORMAT_R32G32_FLOAT = 0x10,
    DDS_FORMAT_R32G32_UINT = 0x11,
    DDS_FORMAT_R32G32_SINT = 0x12,
    DDS_FORMAT_R32G8X24_TYPELESS = 0x13,
    DDS_FORMAT_D32_FLOAT_S8X24_UINT = 0x14,
    DDS_FORMAT_R32_FLOAT_X8X24_TYPELESS = 0x15,
    DDS_FORMAT_X32_TYPELESS_G8X24_UINT = 0x16,
    DDS_FORMAT_R10G10B10A2_TYPELESS = 0x17,
    DDS_FORMAT_R10G10B10A2_UNORM = 0x18,
    DDS_FORMAT_R10G10B10A2_UINT = 0x19,
    DDS_FORMAT_R11G11B10_FLOAT = 0x1a,
    DDS_FORMAT_R8G8B8A8_TYPELESS = 0x1b,
    DDS_FORMAT_R8G8B8A8_UNORM = 0x1c,
    DDS_FORMAT_R8G8B8A8_UNORM_SRGB = 0x1d,
    DDS_FORMAT_R8G8B8A8_UINT = 0x1e,
    DDS_FORMAT_R8G8B8A8_SNORM = 0x1f,
    DDS_FORMAT_R8G8B8A8_SINT = 0x20,
    DDS_FORMAT_R16G16_TYPELESS = 0x21,
    DDS_FORMAT_R16G16_FLOAT = 0x22,
    DDS_FORMAT_R16G16_UNORM = 0x23,
    DDS_FORMAT_R16G16_UINT = 0x24,
    DDS_FORMAT_R16G16_SNORM = 0x25,
    DDS_FORMAT_R16G16_SINT = 0x26,
    DDS_FORMAT_R32_TYPELESS = 0x27,
    DDS_FORMAT_D32_FLOAT = 0x28,
    DDS_FORMAT_R32_FLOAT = 0x29,
    DDS_FORMAT_R32_UINT = 0x2a,
    DDS_FORMAT_R32_SINT = 0x2b,
    DDS_FORMAT_R24G8_TYPELESS = 0x2c,
    DDS_FORMAT_D24_UNORM_S8_UINT = 0x2d,
    DDS_FORMAT_R24_UNORM_X8_TYPELESS = 0x2e,
    DDS_FORMAT_X24_TYPELESS_G8_UINT = 0x2f,
    DDS_FORMAT_R8G8_TYPELESS = 0x30,
    DDS_FORMAT_R8G8_UNORM = 0x31,
    DDS_FORMAT_R8G8_UINT = 0x32,
    DDS_FORMAT_R8G8_SNORM = 0x33,
    DDS_FORMAT_R8G8_SINT = 0x34,
    DDS_FORMAT_R16_TYPELESS = 0x35,
    DDS_FORMAT_R16_FLOAT = 0x36,
    DDS_FORMAT_D16_UNORM = 0x37,
    DDS_FORMAT_R16_UNORM = 0x38,
    DDS_FORMAT_R16_UINT = 0x39,
    DDS_FORMAT_R16_SNORM = 0x3a,
    DDS_FORMAT_R16_SINT = 0x3b,
    DDS_FORMAT_R8_TYPELESS = 0x3c,
    DDS_FORMAT_R8_UNORM = 0x3d,
    DDS_FORMAT_R8_UINT = 0x3e,
    DDS_FORMAT_R8_SNORM = 0x3f,
    DDS_FORMAT_R8_SINT = 0x40,
    DDS_FORMAT_A8_UNORM = 0x41,
    DDS_FORMAT_R1_UNORM = 0x42,
    DDS_FORMAT_R9G9B9E5_SHAREDEXP = 0x43,
    DDS_FORMAT_R8G8_B8G8_UNORM = 0x44,
    DDS_FORMAT_G8R8_G8B8_UNORM = 0x45,
    DDS_FORMAT_BC1_TYPELESS = 0x46,
    DDS_FORMAT_BC1_UNORM = 0x47,
    DDS_FORMAT_BC1_UNORM_SRGB = 0x48,
    DDS_FORMAT_BC2_TYPELESS = 0x49,
    DDS_FORMAT_BC2_UNORM = 0x4a,
    DDS_FORMAT_BC2_UNORM_SRGB = 0x4b,
    DDS_FORMAT_BC3_TYPELESS = 0x4c,
    DDS_FORMAT_BC3_UNORM = 0x4d,
    DDS_FORMAT_BC3_UNORM_SRGB = 0x4e,
    DDS_FORMAT_BC4_TYPELESS = 0x4f,
    DDS_FORMAT_BC4_UNORM = 0x50,
    DDS_FORMAT_BC4_SNORM = 0x51,
    DDS_FORMAT_BC5_TYPELESS = 0x52,
    DDS_FORMAT_BC5_UNORM = 0x53,
    DDS_FORMAT_BC5_SNORM = 0x54,
    DDS_FORMAT_B5G6R5_UNORM = 0x55,
    DDS_FORMAT_B5G5R5A1_UNORM = 0x56,
    DDS_FORMAT_B8G8R8A8_UNORM = 0x57,
    DDS_FORMAT_B8G8R8X8_UNORM = 0x58,
    DDS_FORMAT_R10G10B10_XR_BIAS_A2_UNORM = 0x59,
    DDS_FORMAT_B8G8R8A8_TYPELESS = 0x5a,
    DDS_FORMAT_B8G8R8A8_UNORM_SRGB = 0x5b,
    DDS_FORMAT_B8G8R8X8_TYPELESS = 0x5c,
    DDS_FORMAT_B8G8R8X8_UNORM_SRGB = 0x5d,
    DDS_FORMAT_BC6H_TYPELESS = 0x5e,
    DDS_FORMAT_BC6H_UF16 = 0x5f,
    DDS_FORMAT_BC6H_SF16 = 0x60,
    DDS_FORMAT_BC7_TYPELESS = 0x61,
    DDS_FORMAT_BC7_UNORM = 0x62,
    DDS_FORMAT_BC7_UNORM_SRGB = 0x63,
    DDS_FORMAT_AYUV = 0x64,
    DDS_FORMAT_Y410 = 0x65,
    DDS_FORMAT_Y416 = 0x66,
    DDS_FORMAT_NV12 = 0x67,
    DDS_FORMAT_P010 = 0x68,
    DDS_FORMAT_P016 = 0x69,
    DDS_FORMAT_420_OPAQUE = 0x6a,
    DDS_FORMAT_YUY2 = 0x6b,
    DDS_FORMAT_Y210 = 0x6c,
    DDS_FORMAT_Y216 = 0x6d,
    DDS_FORMAT_NV11 = 0x6e,
    DDS_FORMAT_AI44 = 0x6f,
    DDS_FORMAT_IA44 = 0x70,
    DDS_FORMAT_P8 = 0x71,
    DDS_FORMAT_A8P8 = 0x72,
    DDS_FORMAT_B4G4R4A4_UNORM = 0x73,
    DDS_FORMAT_P208 = 0x82,
    DDS_FORMAT_V208 = 0x83,
    DDS_FORMAT_V408 = 0x84,
    DDS_FORMAT_FORCE_UINT = 0xffffffff
} DDS_FORMAT;

typedef enum DDS_RESOURCE_DIMENSION {
    DDS_RESOURCE_DIMENSION_UNKNOWN = 0,
    DDS_RESOURCE_DIMENSION_BUFFER = 1,
    DDS_RESOURCE_DIMENSION_TEXTURE1D = 2,
    DDS_RESOURCE_DIMENSION_TEXTURE2D = 3,
    DDS_RESOURCE_DIMENSION_TEXTURE3D = 4
} DDS_RESOURCE_DIMENSION;

typedef enum DDS_RESOURCE_MISC_FLAG {
    DDS_RESOURCE_MISC_TEXTURECUBE = 4,
} DDS_RESOURCE_MISC_FLAG;

typedef enum DDS_ALPHA_MODE {
    DDS_ALPHA_MODE_UNKNOWN = 0,
    DDS_ALPHA_MODE_STRAIGHT = 1,
    DDS_ALPHA_MODE_PREMULTIPLIED = 2,
    DDS_ALPHA_MODE_OPAQUE = 3,
    DDS_ALPHA_MODE_CUSTOM = 4
} DDS_ALPHA_MODE;

#define BMP_TAG                  0x4D42

class Image
{
    struct DDS_PF
    {
        uint flags;
        uint fourCC;
        uint bits;
        uint Rmask;
        uint Gmask;
        uint Bmask;
        uint Amask;
    };

private:

    QList<MipMap *> mipMaps;
    PixelFormat pixelFormat = PixelFormat::UnknownPixelFormat;
    DDS_PF ddsPixelFormat{};
    uint DDSflags{};
    bool DX10Type{};

    ImageFormat DetectImageByFilename(const QString &fileName);
    ImageFormat DetectImageByExtension(const QString &extension);
    void LoadImageFromStream(Stream &stream, ImageFormat format);
    void LoadImageFromBuffer(ByteBuffer data, ImageFormat format);
    void LoadImageDDS(Stream &stream);
    void LoadImageTGA(Stream &stream);
    void LoadImageBMP(Stream &stream);
    static void clearAlphaFromInternal(quint8 *data, int w, int h);
    static ByteBuffer RGBtoInternal(const quint8 *src, int w, int h);
    static ByteBuffer ARGBtoInternal(const quint8 *src, int w, int h);
    static ByteBuffer RGBAtoInternal(const quint8 *src, int w, int h);
    static ByteBuffer InternalToARGB(const quint8 *src, int w, int h);
    static ByteBuffer InternalToRGB(const quint8 *src, int w, int h);
    static ByteBuffer InternalToRGBA(const quint8 *src, int w, int h);
    static ByteBuffer InternalToBGR(const quint8 *src, int w, int h);
    static ByteBuffer V8U8ToInternal(const quint8 *src, int w, int h);
    static ByteBuffer InternalToV8U8(const quint8 *src, int w, int h);
    static ByteBuffer G8ToInternal(const quint8 *src, int w, int h);
    static ByteBuffer InternalToG8(const quint8 *src, int w, int h);
    static ByteBuffer InternalToAlphaGreyscale(const quint8 *src, int w, int h);
    static ByteBuffer downscaleInternal(const quint8 *src, int w, int h);
    static ByteBuffer convertToFormat(PixelFormat srcFormat, const quint8 *src, int w, int h,
                                   PixelFormat dstFormat, bool dxt1HasAlpha, quint8 dxt1Threshold, float bc7quality);

public:

    QList<MipMap *> &getMipMaps() { return mipMaps; }
    PixelFormat getPixelFormat() { return pixelFormat; }

    Image(const QString &fileName, ImageFormat format = ImageFormat::UnknownImageFormat);
    Image(Stream &stream, ImageFormat format);
    Image(Stream &stream, const QString &extension);
    Image(const ByteBuffer &data, ImageFormat format);
    Image(const ByteBuffer &data, const QString &extension);
    Image(QList<MipMap *> &mipmaps, PixelFormat pixelFmt);
    ~Image();
    static ByteBuffer convertRawToInternal(const quint8 *src, int w, int h, PixelFormat format, bool clearAlpha = false);
    static ByteBuffer convertRawToRGB(const quint8 *src, int w, int h, PixelFormat format);
    static ByteBuffer convertRawToARGB(const quint8 *src, int w, int h, PixelFormat format);
    static ByteBuffer convertRawToRGBA(const quint8 *src, int w, int h, PixelFormat format);
    static ByteBuffer convertRawToBGR(const quint8 *src, int w, int h, PixelFormat format);
    static ByteBuffer convertRawToAlphaGreyscale(const quint8 *src, int w, int h, PixelFormat format);
    static void saveToPng(const quint8 *src, int w, int h, PixelFormat format, const QString &filename, bool clearAlpha = false);
    void correctMips(PixelFormat dstFormat, bool dxt1HasAlpha, quint8 dxt1Threshold, float bc7quality);
    static PixelFormat getPixelFormatType(const QString &format);
    static QString getEngineFormatType(PixelFormat format);
    void removeMipByIndex(int n);
    static bool checkPowerOfTwo(int n);
    static int returnPowerOfTwo(int n);

    // DDS
private:

    static DDS_PF getDDSPixelFormat(PixelFormat format);
    static void readBlockInternalToDxt(quint8 blockARGB[BLOCK_SIZE_4X4X4], const quint8 *srcARGB,
                                 int srcW, int blockX, int blockY);
    static void writeBlockDxtToInternal(const quint8 blockARGB[BLOCK_SIZE_4X4X4], quint8 *dstARGB,
                                  int dstW, int blockX, int blockY);

    static void readBlockDxtBpp4(quint8 *dst, const quint8 *src, int srcW, int blockX, int blockY);
    static void writeBlockDxtBpp4(quint8 *src, quint8 *dst, int dstW, int blockX, int blockY);

    static void readBlockDxtBpp8(quint8 *dst, const quint8 *src, int srcW, int blockX, int blockY);
    static void writeBlockDxtBpp8(quint8 *src, quint8 *dst, int dstW, int blockX, int blockY);

    static void convertBlock4X4X4FloatToByte(quint8 dst[BLOCK_SIZE_4X4X4], double src[16][4]);
    static void convertBlock4X4X4ByteToFloat(double dst[BLOCK_SIZE_4X4][4], const quint8 src[BLOCK_SIZE_4X4X4]);


    static void readBlockInternalToAti2(quint8 blockDstX[BLOCK_SIZE_4X4BPP8],
                                 quint8 blockDstY[BLOCK_SIZE_4X4BPP8],
                                 const quint8 *src, int srcW, int blockX, int blockY);
    static void writeBlock4X4ATI2(quint8 *blockSrcX, quint8 *blockSrcY,
                                  quint8 *dst, int dstW, int blockX, int blockY);

    static void writeBlockAti2ToInternal(const quint8 blockR[BLOCK_SIZE_4X4BPP8],
                                      const quint8 blockG[BLOCK_SIZE_4X4BPP8],
                                      quint8 *dstARGB, int srcW, int blockX, int blockY);

    static ByteBuffer compressMipmap(PixelFormat dstFormat, const quint8 *src, int w, int h,
                                     bool useDXT1Alpha, quint8 DXT1Threshold, float bc7quality);
    static ByteBuffer decompressMipmap(PixelFormat srcFormat, const quint8 *src, int w, int h);

public:

    bool checkDDSHaveAllMipmaps();
    void StoreImageToDDS(Stream &stream, PixelFormat format = PixelFormat::UnknownPixelFormat);
    ByteBuffer StoreImageToDDS();
};

#endif
