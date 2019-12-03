/*
 * MassEffectModder
 *
 * Copyright (C) 2018-2019 Pawel Kolodziejski
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

#define BLOCK_SIZE_4X4X4         64
#define BLOCK_SIZE_4X4BPP4       8
#define BLOCK_SIZE_4X4BPP8       16

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

    ImageFormat DetectImageByFilename(const QString &fileName);
    ImageFormat DetectImageByExtension(const QString &extension);
    void LoadImageFromStream(Stream &stream, ImageFormat format);
    void LoadImageFromBuffer(ByteBuffer data, ImageFormat format);
    void LoadImageDDS(Stream &stream);
    void LoadImageTGA(Stream &stream);
    void LoadImageBMP(Stream &stream);
    static void clearAlphaFromARGB(quint8 *data, int w, int h);
    static ByteBuffer RGBToARGB(const quint8 *src, int w, int h);
    static ByteBuffer ARGBtoRGB(const quint8 *src, int w, int h);
    static ByteBuffer ARGBtoBGR(const quint8 *src, int w, int h);
    static ByteBuffer V8U8ToARGB(const quint8 *src, int w, int h);
    static ByteBuffer ARGBtoV8U8(const quint8 *src, int w, int h);
    static ByteBuffer G8ToARGB(const quint8 *src, int w, int h);
    static ByteBuffer ARGBtoG8(const quint8 *src, int w, int h);
    static ByteBuffer ARGBtoAlphaGreyscale(const quint8 *src, int w, int h);
    static ByteBuffer downscaleARGB(const quint8 *src, int w, int h);
    static ByteBuffer downscaleRGB(const quint8 *src, int w, int h);
    static ByteBuffer convertToFormat(PixelFormat srcFormat, const quint8 *src, int w, int h,
                                   PixelFormat dstFormat, bool dxt1HasAlpha = false, quint8 dxt1Threshold = 128);

public:

    QList<MipMap *>& getMipMaps() { return mipMaps; }
    PixelFormat getPixelFormat() { return pixelFormat; }

    Image(const QString &fileName, ImageFormat format = ImageFormat::UnknownImageFormat);
    Image(Stream &stream, ImageFormat format);
    Image(Stream &stream, const QString &extension);
    Image(const ByteBuffer &data, ImageFormat format);
    Image(const ByteBuffer &data, const QString &extension);
    Image(QList<MipMap *> &mipmaps, PixelFormat pixelFmt);
    ~Image();
    static ByteBuffer convertRawToARGB(const quint8 *src, int w, int h, PixelFormat format, bool clearAlpha = false);
    static ByteBuffer convertRawToRGB(const quint8 *src, int w, int h, PixelFormat format);
    static ByteBuffer convertRawToBGR(const quint8 *src, int w, int h, PixelFormat format);
    static ByteBuffer convertRawToAlphaGreyscale(const quint8 *src, int w, int h, PixelFormat format);
    static void saveToPng(const quint8 *src, int w, int h, PixelFormat format, const QString &filename);
    void correctMips(PixelFormat dstFormat, bool dxt1HasAlpha = false, quint8 dxt1Threshold = 128);
    static PixelFormat getPixelFormatType(const QString &format);
    static QString getEngineFormatType(PixelFormat format);
    void removeMipByIndex(int n);
    static bool checkPowerOfTwo(int n);
    static int returnPowerOfTwo(int n);

    // DDS
private:

    static DDS_PF getDDSPixelFormat(PixelFormat format);
    static void readBlock4X4ARGB(quint8 blockARGB[BLOCK_SIZE_4X4X4], const quint8 *srcARGB,
                                 int srcW, int blockX, int blockY);
    static void writeBlock4X4ARGB(const quint8 blockARGB[BLOCK_SIZE_4X4X4], quint8 *dstARGB,
                                  int dstW, int blockX, int blockY);

    static void readBlock4X4BPP4(uint block[2], const quint8 *src, int srcW, int blockX, int blockY);
    static void writeBlock4X4BPP4(const uint block[2], quint8 *dst, int dstW, int blockX, int blockY);

    static void readBlock4X4BPP8(uint block[4], const quint8 *src, int srcW, int blockX, int blockY);
    static void writeBlock4X4BPP8(const uint block[4], quint8 *dst, int dstW, int blockX, int blockY);

    static void readBlock4X4ATI2(quint8 blockDstX[BLOCK_SIZE_4X4BPP8],
                                 quint8 blockDstY[BLOCK_SIZE_4X4BPP8],
                                 const quint8 *src, int srcW, int blockX, int blockY);
    static void writeBlock4X4ATI2(const uint blockSrcX[2], const uint blockSrcY[2],
                                  quint8 *dst, int dstW, int blockX, int blockY);

    static void writeBlock4X4ARGBATI2(const quint8 blockR[BLOCK_SIZE_4X4BPP8],
                                      const quint8 blockG[BLOCK_SIZE_4X4BPP8],
                                      quint8 *dstARGB, int srcW, int blockX, int blockY);

    static ByteBuffer compressMipmap(PixelFormat dstFormat, const quint8 *src, int w, int h,
                                     bool useDXT1Alpha = false, quint8 DXT1Threshold = 128);
    static ByteBuffer decompressMipmap(PixelFormat srcFormat, const quint8 *src, int w, int h);

public:

    bool checkDDSHaveAllMipmaps();
    Image *convertToARGB();
    Image *convertToRGB();
    void StoreImageToDDS(Stream &stream, PixelFormat format = PixelFormat::UnknownPixelFormat);
    ByteBuffer StoreImageToDDS();
};

#endif
