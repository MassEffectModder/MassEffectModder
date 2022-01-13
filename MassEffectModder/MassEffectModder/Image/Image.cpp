/*
 * MassEffectModder
 *
 * Copyright (C) 2018-2022 Pawel Kolodziejski
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

#include <Image/Image.h>
#include <Helpers/MemoryStream.h>
#include <Helpers/FileStream.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <Wrappers.h>

Image::Image(int width, int height)
{
    ByteBuffer pixels(width * height * 4 * sizeof(float));
    float *ptr = pixels.ptrAsFloat();
    int offset = 0;
    for (int h = 0; h < height; h++)
    {
        for (int w = 0; w < width; w++)
        {
            ptr[4 * offset + 0] = 0.0f;
            ptr[4 * offset + 1] = 0.0f;
            ptr[4 * offset + 2] = 0.0f;
            ptr[4 * offset + 3] = 1.0f;
            offset++;
        }
    }

    pixelFormat = PixelFormat::Internal;
    mipMaps.push_back(new MipMap(pixels, width, height, PixelFormat::Internal));
    pixels.Free();
}

Image::Image(const QString &fileName, ImageFormat format)
{
    if (format == ImageFormat::UnknownImageFormat)
        format = DetectImageByFilename(fileName);

    switch (format)
    {
        case ImageFormat::BMP:
        case ImageFormat::DDS:
        case ImageFormat::TGA:
        {
            FileStream file(fileName, FileMode::Open, FileAccess::ReadOnly);
            LoadImageFromStream(file, format, sourceIs8Bits);
            return;
        }
        case ImageFormat::PNG:
        {
            FileStream file(fileName, FileMode::Open, FileAccess::ReadOnly);
            ByteBuffer buffer = file.ReadToBuffer(file.Length());
            LoadImageFromBuffer(buffer, format, sourceIs8Bits);
            buffer.Free();
            return;
        }
        case ImageFormat::UnknownImageFormat:
            break;
    }

    CRASH_MSG("Not supported format");
}

Image::Image(Stream &stream, ImageFormat format)
{
    switch (format)
    {
        case ImageFormat::BMP:
        case ImageFormat::DDS:
        case ImageFormat::TGA:
        {
            LoadImageFromStream(stream, format, sourceIs8Bits);
            return;
        }
        case ImageFormat::PNG:
        case ImageFormat::UnknownImageFormat:
            break;
    }

    PERROR("Not supported format!\n");
}

Image::Image(Stream &stream, const QString &extension)
{
    ImageFormat format = DetectImageByExtension(extension);
    switch (format)
    {
        case ImageFormat::BMP:
        case ImageFormat::DDS:
        case ImageFormat::TGA:
        {
            LoadImageFromStream(stream, format, sourceIs8Bits);
            return;
        }
        case ImageFormat::PNG:
        case ImageFormat::UnknownImageFormat:
            break;
    }

    PERROR("Not supported format: " + extension + "\n");
}

Image::Image(const ByteBuffer &data, ImageFormat format)
{
    switch (format)
    {
        case ImageFormat::BMP:
        case ImageFormat::DDS:
        case ImageFormat::TGA:
        {
            MemoryStream stream(data);
            LoadImageFromStream(stream, format, sourceIs8Bits);
            return;
        }
        case ImageFormat::PNG:
        {
            LoadImageFromBuffer(data, format, sourceIs8Bits);
            return;
        }
        case ImageFormat::UnknownImageFormat:
            return;
    }

    CRASH();
}

Image::Image(const ByteBuffer &data, const QString &extension)
{
    ImageFormat format = DetectImageByExtension(extension);
    switch (format)
    {
        case ImageFormat::BMP:
        case ImageFormat::DDS:
        case ImageFormat::TGA:
        {
            MemoryStream stream(data);
            LoadImageFromStream(stream, format, sourceIs8Bits);
            return;
        }
        case ImageFormat::PNG:
        {
            LoadImageFromBuffer(data, format, sourceIs8Bits);
            return;
        }
        case ImageFormat::UnknownImageFormat:
            return;
    }

    CRASH();
}

Image::Image(QList<MipMap *> &mipmaps, PixelFormat pixelFmt)
{
    mipMaps = std::move(mipmaps);
    pixelFormat = pixelFmt;
}

Image::~Image()
{
    foreach(MipMap *mipmap, mipMaps)
    {
        mipmap->Free();
        delete mipmap;
    }
}

void Image::generateGradient()
{
    if (pixelFormat != PixelFormat::Internal)
        CRASH();

    auto mipmap = mipMaps.first();
    float *ptr = mipmap->getRefData().ptrAsFloat();
    int offset = 0;
    for (int h = 0; h < mipmap->getHeight(); h++)
    {
        for (int w = 0; w < mipmap->getWidth(); w++)
        {
            float color = (float)w / mipmap->getWidth();
            ptr[4 * offset + 0] = color;
            ptr[4 * offset + 1] = color;
            ptr[4 * offset + 2] = color;
            ptr[4 * offset + 3] = 1.0f;
            offset++;
        }
    }
}

void Image::removeMipByIndex(int n)
{
    if (n < 0 || n >= mipMaps.count())
        CRASH();

    mipMaps[n]->Free();
    delete mipMaps[n];
    mipMaps.removeAt(n);
}

ImageFormat Image::DetectImageByFilename(const QString &fileName)
{
    return DetectImageByExtension(GetFileExtension(fileName));
}

ImageFormat Image::DetectImageByExtension(const QString &extension)
{
    QString ext = extension.toLower();
    if (ext == "dds")
        return ImageFormat::DDS;
    if (ext == "tga")
        return ImageFormat::TGA;
    if (ext == "bmp")
        return ImageFormat::BMP;
    if (ext == "png")
        return ImageFormat::PNG;

    return ImageFormat::UnknownImageFormat;
}

void Image::LoadImageFromStream(Stream &stream, ImageFormat format, bool &source8Bits)
{
    foreach(MipMap *mipmap, mipMaps)
    {
        mipmap->Free();
        delete mipmap;
    }
    mipMaps.clear();
    source8Bits = true;

    switch (format)
    {
        case ImageFormat::BMP:
            {
                LoadImageBMP(stream);
                return;
            }
        case ImageFormat::DDS:
            {
                LoadImageDDS(stream, source8Bits);
                return;
            }
        case ImageFormat::TGA:
            {
                LoadImageTGA(stream);
                return;
            }
        case ImageFormat::PNG:
        case ImageFormat::UnknownImageFormat:
            return;
    }

    CRASH();
}

void Image::LoadImageFromBuffer(ByteBuffer data, ImageFormat format, bool &source8Bits)
{
    foreach(MipMap *mipmap, mipMaps)
    {
        mipmap->Free();
        delete mipmap;
    }
    mipMaps.clear();
    source8Bits = true;

    float *imageBuffer;
    quint32 imageSize, imageWidth, imageHeight;
    if (format == ImageFormat::PNG)
    {
        if (PngRead(data.ptr(), data.size(), &imageBuffer,
                    &imageSize, &imageWidth, &imageHeight, source8Bits) != 0)
        {
            PERROR("Failed load PNG.\n");
            return;
        }
    }
    else
        CRASH();

    if (!checkPowerOfTwo(imageWidth) ||
        !checkPowerOfTwo(imageHeight))
    {
        PERROR("PNG image dimensions are not power of two.\n");
        return;
    }

    ByteBuffer pixels((quint8 *)imageBuffer, imageSize);
    delete[] imageBuffer;
    mipMaps.push_back(new MipMap(pixels, imageWidth, imageHeight, PixelFormat::Internal));
    pixels.Free();
    pixelFormat = PixelFormat::Internal;
}

void Image::convertInternalToRGBE()
{
    if (pixelFormat != PixelFormat::Internal)
        CRASH();
    auto mipmap = mipMaps.first();
    auto pixels = InternalToRGBE(mipmap->getRefData(), mipmap->getWidth(), mipmap->getHeight());
    int width = mipmap->getWidth();
    int height = mipmap->getHeight();
    foreach(MipMap *mip, mipMaps)
    {
        mip->Free();
        delete mip;
    }
    mipMaps.clear();
    mipMaps.push_back(new MipMap(pixels, width, height, PixelFormat::RGBE));
    pixels.Free();
    pixelFormat = PixelFormat::RGBE;
}

void Image::convertInternalToRGBA10(bool clearAlpha)
{
    if (pixelFormat != PixelFormat::Internal)
        CRASH();
    auto mipmap = mipMaps.first();
    ByteBuffer pixels;
    if (clearAlpha)
        pixels = InternalToR10G10B10A2ClearAlpha(mipmap->getRefData(), mipmap->getWidth(), mipmap->getHeight());
    else
        pixels = InternalToR10G10B10A2(mipmap->getRefData(), mipmap->getWidth(), mipmap->getHeight());
    int width = mipmap->getWidth();
    int height = mipmap->getHeight();
    foreach(MipMap *mip, mipMaps)
    {
        mip->Free();
        delete mip;
    }
    mipMaps.clear();
    mipMaps.push_back(new MipMap(pixels, width, height, PixelFormat::R10G10B10A2));
    pixels.Free();
    pixelFormat = PixelFormat::R10G10B10A2;
}

void Image::convertInternalToRGBA16()
{
    if (pixelFormat != PixelFormat::Internal)
        CRASH();
    auto mipmap = mipMaps.first();
    auto pixels = InternalToR16G16B16A16(mipmap->getRefData(), mipmap->getWidth(), mipmap->getHeight());
    int width = mipmap->getWidth();
    int height = mipmap->getHeight();
    foreach(MipMap *mip, mipMaps)
    {
        mip->Free();
        delete mip;
    }
    mipMaps.clear();
    mipMaps.push_back(new MipMap(pixels, width, height, PixelFormat::R16G16B16A16));
    pixels.Free();
    pixelFormat = PixelFormat::R16G16B16A16;
}

ByteBuffer Image::convertRawToInternal(const ByteBuffer src, int w, int h, PixelFormat format, bool clearAlpha)
{
    ByteBuffer tmpPtr;

    switch (format)
    {
        case PixelFormat::Internal:
            tmpPtr = ByteBuffer(src.ptr(), src.size());
            break;
        case PixelFormat::DXT1:
        case PixelFormat::DXT3:
        case PixelFormat::DXT5:
        case PixelFormat::BC5:
        case PixelFormat::BC7:
            if (w < 4 || h < 4)
            {
                if (w < 4)
                    w = 4;
                if (h < 4)
                    h = 4;
                tmpPtr = ByteBuffer(w * h * 4 * sizeof(float));
                memset(tmpPtr.ptr(), 0, tmpPtr.size());
                return tmpPtr;
            }
            tmpPtr = decompressMipmap(format, src, w, h);
            break;
        case PixelFormat::ATI2:
            if (w < 4 || h < 4)
            {
                tmpPtr = ByteBuffer(w * h * 4 * sizeof(float));
                memset(tmpPtr.ptr(), 0, tmpPtr.size());
                return tmpPtr;
            }
            tmpPtr = decompressMipmap(format, src, w, h);
            break;
        case PixelFormat::RGBA: tmpPtr = RGBAtoInternal(src, w, h); break;
        case PixelFormat::R10G10B10A2: tmpPtr = R10G10B10A2toInternal(src, w, h); break;
        case PixelFormat::R16G16B16A16: tmpPtr = R16G16B16A16toInternal(src, w, h); break;
        case PixelFormat::ARGB: tmpPtr = ARGBtoInternal(src, w, h); break;
        case PixelFormat::RGB: tmpPtr = RGBtoInternal(src, w, h); break;
        case PixelFormat::V8U8: tmpPtr = V8U8ToInternal(src, w, h); break;
        case PixelFormat::G8: tmpPtr = G8ToInternal(src, w, h); break;
        case PixelFormat::RGBE: tmpPtr = RGBEToInternal(src, w, h); break;
        default:
            CRASH_MSG("Invalid texture format.");
    }

    if (clearAlpha)
        clearAlphaFromInternal(tmpPtr, w, h);

    return tmpPtr;
}

ByteBuffer Image::convertRawToRGB(const ByteBuffer src, int w, int h, PixelFormat format)
{
    auto dataRGBA = convertRawToInternal(src, w, h, format);
    auto dataRGB = InternalToRGB(dataRGBA, w, h);
    dataRGBA.Free();
    return dataRGB;
}

ByteBuffer Image::convertRawToARGB(const ByteBuffer src, int w, int h, PixelFormat format)
{
    auto dataRGBA = convertRawToInternal(src, w, h, format);
    auto dataARGB = InternalToARGB(dataRGBA, w, h);
    dataRGBA.Free();
    return dataARGB;
}

ByteBuffer Image::convertRawToRGBA(const ByteBuffer src, int w, int h, PixelFormat format)
{
    auto dataRGBA = convertRawToInternal(src, w, h, format);
    auto dataARGB = InternalToRGBA(dataRGBA, w, h);
    dataRGBA.Free();
    return dataARGB;
}

ByteBuffer Image::convertRawToR10G10B10A2(const ByteBuffer src, int w, int h, PixelFormat format)
{
    auto dataRGBA = convertRawToInternal(src, w, h, format);
    auto dataARGB = InternalToR10G10B10A2(dataRGBA, w, h);
    dataRGBA.Free();
    return dataARGB;
}

ByteBuffer Image::convertRawToR16G16B16A16(const ByteBuffer src, int w, int h, PixelFormat format)
{
    auto dataRGBA = convertRawToInternal(src, w, h, format);
    auto dataARGB = InternalToR16G16B16A16(dataRGBA, w, h);
    dataRGBA.Free();
    return dataARGB;
}

ByteBuffer Image::convertRawToBGR(const ByteBuffer src, int w, int h, PixelFormat format, bool clearAlpha)
{
    auto dataARGB = convertRawToInternal(src, w, h, format, clearAlpha);
    auto dataBGR = InternalToBGR(dataARGB, w, h);
    dataARGB.Free();
    return dataBGR;
}

ByteBuffer Image::convertRawToAlphaGreyscale(const ByteBuffer src, int w, int h, PixelFormat format, bool clearAlpha)
{
    auto dataARGB = convertRawToInternal(src, w, h, format, clearAlpha);
    auto dataRGB = InternalToAlphaGreyscale(dataARGB, w, h);
    dataARGB.Free();
    return dataRGB;
}

void Image::clearAlphaFromInternal(ByteBuffer data, int w, int h)
{
    float *dst = data.ptrAsFloat();
    for (int i = 0; i < w * h; i++)
    {
        dst[4 * i + 3] = 1.0f;
    }
}

ByteBuffer Image::RGBtoInternal(const ByteBuffer src, int w, int h)
{
    ByteBuffer tmpData(w * h * 4 * sizeof(float));
    float *ptr = tmpData.ptrAsFloat();
    quint8 *srcPtr = src.ptr();
    for (int i = 0; i < w * h; i++)
    {
        ptr[4 * i + 0] = CONVERT_BYTE_TO_FLOAT(srcPtr[3 * i + 2]);
        ptr[4 * i + 1] = CONVERT_BYTE_TO_FLOAT(srcPtr[3 * i + 1]);
        ptr[4 * i + 2] = CONVERT_BYTE_TO_FLOAT(srcPtr[3 * i + 0]);
        ptr[4 * i + 3] = 1.0f;
    }
    return tmpData;
}

ByteBuffer Image::RGBAtoInternal(const ByteBuffer src, int w, int h)
{
    ByteBuffer tmpData(w * h * 4 * sizeof(float));
    float *ptr = tmpData.ptrAsFloat();
    quint8 *srcPtr = src.ptr();
    for (int i = 0; i < w * h; i++)
    {
        ptr[4 * i + 0] = CONVERT_BYTE_TO_FLOAT(srcPtr[4 * i + 0]);
        ptr[4 * i + 1] = CONVERT_BYTE_TO_FLOAT(srcPtr[4 * i + 1]);
        ptr[4 * i + 2] = CONVERT_BYTE_TO_FLOAT(srcPtr[4 * i + 2]);
        ptr[4 * i + 3] = CONVERT_BYTE_TO_FLOAT(srcPtr[4 * i + 3]);
    }
    return tmpData;
}

ByteBuffer Image::R10G10B10A2toInternal(const ByteBuffer src, int w, int h)
{
    ByteBuffer tmpData(w * h * 4 * sizeof(float));
    float *ptr = tmpData.ptrAsFloat();
    auto *srcPtr = (quint32 *)src.ptr();
    for (int i = 0; i < w * h; i++)
    {
        quint32 r = (srcPtr[i] >> 0) & 0x3ff;
        quint32 g = (srcPtr[i] >> 10) & 0x3ff;
        quint32 b = (srcPtr[i] >> 20) & 0x3ff;
        quint32 a = (srcPtr[i] >> 30);
        ptr[4 * i + 0] = CONVERT_UINT10_TO_FLOAT(r);
        ptr[4 * i + 1] = CONVERT_UINT10_TO_FLOAT(g);
        ptr[4 * i + 2] = CONVERT_UINT10_TO_FLOAT(b);
        ptr[4 * i + 3] = CONVERT_UINT2_TO_FLOAT(a);
    }
    return tmpData;
}

ByteBuffer Image::R16G16B16A16toInternal(const ByteBuffer src, int w, int h)
{
    ByteBuffer tmpData(w * h * 4 * sizeof(float));
    float *ptr = tmpData.ptrAsFloat();
    auto *srcPtr = (quint64 *)src.ptr();
    for (int i = 0; i < w * h; i++)
    {
        quint64 r = (srcPtr[i] >> 0) & 0xffff;
        quint64 g = (srcPtr[i] >> 16) & 0xffff;
        quint64 b = (srcPtr[i] >> 32) & 0xffff;
        quint64 a = (srcPtr[i] >> 48);
        ptr[4 * i + 0] = CONVERT_UINT16_TO_FLOAT(r);
        ptr[4 * i + 1] = CONVERT_UINT16_TO_FLOAT(g);
        ptr[4 * i + 2] = CONVERT_UINT16_TO_FLOAT(b);
        ptr[4 * i + 3] = CONVERT_UINT16_TO_FLOAT(a);
    }
    return tmpData;
}

ByteBuffer Image::ARGBtoInternal(const ByteBuffer src, int w, int h)
{
    ByteBuffer tmpData(w * h * 4 * sizeof(float));
    float *ptr = tmpData.ptrAsFloat();
    quint8 *srcPtr = src.ptr();
    for (int i = 0; i < w * h; i++)
    {
        ptr[4 * i + 0] = CONVERT_BYTE_TO_FLOAT(srcPtr[4 * i + 2]);
        ptr[4 * i + 1] = CONVERT_BYTE_TO_FLOAT(srcPtr[4 * i + 1]);
        ptr[4 * i + 2] = CONVERT_BYTE_TO_FLOAT(srcPtr[4 * i + 0]);
        ptr[4 * i + 3] = CONVERT_BYTE_TO_FLOAT(srcPtr[4 * i + 3]);
    }
    return tmpData;
}

ByteBuffer Image::InternalToARGB(const ByteBuffer src, int w, int h)
{
    ByteBuffer tmpData(w * h * 4);
    quint8 *ptr = tmpData.ptr();
    float *srcPtr = src.ptrAsFloat();
    for (int i = 0; i < w * h; i++)
    {
        ptr[4 * i + 2] = ROUND_FLOAT_TO_BYTE(srcPtr[4 * i + 0]);
        ptr[4 * i + 1] = ROUND_FLOAT_TO_BYTE(srcPtr[4 * i + 1]);
        ptr[4 * i + 0] = ROUND_FLOAT_TO_BYTE(srcPtr[4 * i + 2]);
        ptr[4 * i + 3] = ROUND_FLOAT_TO_BYTE(srcPtr[4 * i + 3]);
    }
    return tmpData;
}

ByteBuffer Image::InternalToRGBA(const ByteBuffer src, int w, int h)
{
    ByteBuffer tmpData(w * h * 4);
    quint8 *ptr = tmpData.ptr();
    float *srcPtr = src.ptrAsFloat();
    for (int i = 0; i < w * h; i++)
    {
        ptr[4 * i + 0] = ROUND_FLOAT_TO_BYTE(srcPtr[4 * i + 0]);
        ptr[4 * i + 1] = ROUND_FLOAT_TO_BYTE(srcPtr[4 * i + 1]);
        ptr[4 * i + 2] = ROUND_FLOAT_TO_BYTE(srcPtr[4 * i + 2]);
        ptr[4 * i + 3] = ROUND_FLOAT_TO_BYTE(srcPtr[4 * i + 3]);
    }
    return tmpData;
}

ByteBuffer Image::InternalToR10G10B10A2(const ByteBuffer src, int w, int h)
{
    ByteBuffer tmpData(w * h * sizeof(quint32));
    auto *ptr = (quint32 *)tmpData.ptr();
    float *srcPtr = src.ptrAsFloat();
    for (int i = 0; i < w * h; i++)
    {
        quint32 r = ROUND_FLOAT_TO_UINT10(srcPtr[4 * i + 0]);
        quint32 g = ROUND_FLOAT_TO_UINT10(srcPtr[4 * i + 1]);
        quint32 b = ROUND_FLOAT_TO_UINT10(srcPtr[4 * i + 2]);
        quint32 a = ROUND_FLOAT_TO_UINT2(srcPtr[4 * i + 3]);
        quint32 pixel = r | g << 10 | b << 20 | a << 30;
        ptr[i] = pixel;
    }
    return tmpData;
}

ByteBuffer Image::InternalToR10G10B10A2ClearAlpha(const ByteBuffer src, int w, int h)
{
    ByteBuffer tmpData(w * h * sizeof(quint32));
    auto *ptr = (quint32 *)tmpData.ptr();
    float *srcPtr = src.ptrAsFloat();
    for (int i = 0; i < w * h; i++)
    {
        quint32 r = ROUND_FLOAT_TO_UINT10(srcPtr[4 * i + 0]);
        quint32 g = ROUND_FLOAT_TO_UINT10(srcPtr[4 * i + 1]);
        quint32 b = ROUND_FLOAT_TO_UINT10(srcPtr[4 * i + 2]);
        quint32 a = 3;
        quint32 pixel = r | g << 10 | b << 20 | a << 30;
        ptr[i] = pixel;
    }
    return tmpData;
}

ByteBuffer Image::InternalToR16G16B16A16(const ByteBuffer src, int w, int h)
{
    ByteBuffer tmpData(w * h * sizeof(quint64));
    auto *ptr = (quint64 *)tmpData.ptr();
    float *srcPtr = src.ptrAsFloat();
    for (int i = 0; i < w * h; i++)
    {
        quint64 r = ROUND_FLOAT_TO_UINT16(srcPtr[4 * i + 0]);
        quint64 g = ROUND_FLOAT_TO_UINT16(srcPtr[4 * i + 1]);
        quint64 b = ROUND_FLOAT_TO_UINT16(srcPtr[4 * i + 2]);
        quint64 a = ROUND_FLOAT_TO_UINT16(srcPtr[4 * i + 3]);
        quint64 pixel = r | g << 16 | b << 32 | a << 48;
        ptr[i] = pixel;
    }
    return tmpData;
}

ByteBuffer Image::InternalToRGB(const ByteBuffer src, int w, int h)
{
    ByteBuffer tmpData(w * h * 3);
    quint8 *ptr = tmpData.ptr();
    float *srcPtr = src.ptrAsFloat();
    for (int i = 0; i < w * h; i++)
    {
        ptr[3 * i + 0] = ROUND_FLOAT_TO_BYTE(srcPtr[4 * i + 2]);
        ptr[3 * i + 1] = ROUND_FLOAT_TO_BYTE(srcPtr[4 * i + 1]);
        ptr[3 * i + 2] = ROUND_FLOAT_TO_BYTE(srcPtr[4 * i + 0]);
    }
    return tmpData;
}

ByteBuffer Image::InternalToBGR(const ByteBuffer src, int w, int h)
{
    ByteBuffer tmpData(w * h * 3);
    quint8 *ptr = tmpData.ptr();
    float *srcPtr = src.ptrAsFloat();
    for (int i = 0; i < w * h; i++)
    {
        ptr[3 * i + 0] = ROUND_FLOAT_TO_BYTE(srcPtr[4 * i + 0]);
        ptr[3 * i + 1] = ROUND_FLOAT_TO_BYTE(srcPtr[4 * i + 1]);
        ptr[3 * i + 2] = ROUND_FLOAT_TO_BYTE(srcPtr[4 * i + 2]);
    }
    return tmpData;
}

ByteBuffer Image::V8U8ToInternal(const ByteBuffer src, int w, int h)
{
    ByteBuffer tmpData(w * h * 4 * sizeof(float));
    float *ptr = tmpData.ptrAsFloat();
    quint8 *srcPtr = src.ptr();
    for (int i = 0; i < w * h; i++)
    {
        ptr[4 * i + 0] = CONVERT_BYTE_TO_FLOAT((qint8)srcPtr[2 * i + 0] + 128);
        ptr[4 * i + 1] = CONVERT_BYTE_TO_FLOAT((qint8)srcPtr[2 * i + 1] + 128);
        ptr[4 * i + 2] = 1.0f;
        ptr[4 * i + 3] = 1.0f;
    }
    return tmpData;
}

ByteBuffer Image::InternalToV8U8(const ByteBuffer src, int w, int h)
{
    ByteBuffer tmpData(w * h * 2);
    quint8 *ptr = tmpData.ptr();
    float *srcPtr = src.ptrAsFloat();
    for (int i = 0; i < w * h; i++)
    {
        ptr[2 * i + 0] = (quint8)((qint8)ROUND_FLOAT_TO_BYTE(srcPtr[4 * i + 0]) - 128);
        ptr[2 * i + 1] = (quint8)((qint8)ROUND_FLOAT_TO_BYTE(srcPtr[4 * i + 1]) - 128);
    }
    return tmpData;
}

ByteBuffer Image::G8ToInternal(const ByteBuffer src, int w, int h)
{
    ByteBuffer tmpData(w * h * 4 * sizeof(float));
    float *ptr = tmpData.ptrAsFloat();
    quint8 *srcPtr = src.ptr();
    for (int i = 0; i < w * h; i++)
    {
        ptr[4 * i + 0] = CONVERT_BYTE_TO_FLOAT(srcPtr[i]);
        ptr[4 * i + 1] = CONVERT_BYTE_TO_FLOAT(srcPtr[i]);
        ptr[4 * i + 2] = CONVERT_BYTE_TO_FLOAT(srcPtr[i]);
        ptr[4 * i + 3] = 1.0f;
    }

    return tmpData;
}

ByteBuffer Image::InternalToG8(const ByteBuffer src, int w, int h)
{
    ByteBuffer tmpData(w * h);
    quint8 *ptr = tmpData.ptr();
    float *srcPtr = src.ptrAsFloat();
    for (int i = 0; i < w * h; i++)
    {
        float c = srcPtr[i * 4 + 0] + srcPtr[i * 4 + 1] + srcPtr[i * 4 + 2];
        ptr[i] = ROUND_FLOAT_TO_BYTE(c / 3.0f);
    }

    return tmpData;
}

static inline void float2rgbe(unsigned char rgbe[4], float red, float green, float blue)
{
    float v = red;

    if (green > v)
        v = green;

    if (blue > v)
        v = blue;

    if (v < 1e-32)
    {
        rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
    }
    else
    {
        int e;
        v = frexp(v, &e) * 256.0f / v;
        rgbe[0] = (quint8)(red * v);
        rgbe[1] = (quint8)(green * v);
        rgbe[2] = (quint8)(blue * v);
        rgbe[3] = (quint8)(e + 128);
    }
}

ByteBuffer Image::InternalToRGBE(const ByteBuffer src, int w, int h)
{
    ByteBuffer tmpData(w * h * 4);
    quint8 *ptr = tmpData.ptr();
    float *srcPtr = src.ptrAsFloat();

    for (int i = 0; i < w * h; i++)
    {
        float2rgbe(&ptr[4 * i], srcPtr[4 * i + 0], srcPtr[4 * i + 1], srcPtr[4 * i + 2]);
    }

    return tmpData;
}

static inline void rgbe2float(float *red, float *green, float *blue, unsigned char rgbe[4])
{
    if (rgbe[3])
    {
        float f = ldexp(1.0f, rgbe[3] - (int)(128 + 8));
        *red = rgbe[0] * f;
        *green = rgbe[1] * f;
        *blue = rgbe[2] * f;
    }
    else
    {
        *red = *green = *blue = 0.0f;
    }
}

ByteBuffer Image::RGBEToInternal(const ByteBuffer src, int w, int h)
{
    ByteBuffer tmpData(w * h * 4 * sizeof(float));
    float *ptr = tmpData.ptrAsFloat();
    quint8 *srcPtr = src.ptr();

    for (int i = 0; i < w * h; i++)
    {
        rgbe2float(&ptr[4 * i + 0], &ptr[4 * i + 1], &ptr[4 * i + 2], &srcPtr[4 * i]);
        ptr[4 * i + 3] = 1.0f;
    }

    return tmpData;
}

ByteBuffer Image::InternalToAlphaGreyscale(const ByteBuffer src, int w, int h)
{
    ByteBuffer tmpData(w * h * 3);
    quint8 *ptr = tmpData.ptr();
    float *srcPtr = src.ptrAsFloat();

    for (int i = 0; i < w * h; i++)
    {
        float alpha = srcPtr[4 * i + 3];
        ptr[3 * i + 0] = ROUND_FLOAT_TO_BYTE(alpha);
        ptr[3 * i + 1] = ROUND_FLOAT_TO_BYTE(alpha);
        ptr[3 * i + 2] = ROUND_FLOAT_TO_BYTE(alpha);
    }

    return tmpData;
}

bool Image::InternalDetectAlphaData(const ByteBuffer src, int w, int h)
{
    float *srcPtr = src.ptrAsFloat();
    for (int i = 0; i < w * h; i++)
    {
        if (ROUND_FLOAT_TO_BYTE(srcPtr[4 * i + 3]) != 255)
        {
            return true;
        }
    }
    return false;
}

ByteBuffer Image::downscaleInternal(const ByteBuffer src, int w, int h)
{
    if (w == 1 && h == 1)
        CRASH_MSG("1x1 can not be downscaled");

    float *srcPtr = src.ptrAsFloat();

    if (w == 1 || h == 1)
    {
        ByteBuffer tmpData(w * h * 2 * sizeof(float));
        float *ptr = tmpData.ptrAsFloat();
        for (int srcPos = 0, dstPos = 0; dstPos < w * h * 2; srcPos += 8)
        {
            ptr[dstPos++] = (srcPtr[srcPos + 0] + srcPtr[srcPos + 4 + 0]) / 2.0f;
            ptr[dstPos++] = (srcPtr[srcPos + 1] + srcPtr[srcPos + 4 + 1]) / 2.0f;
            ptr[dstPos++] = (srcPtr[srcPos + 2] + srcPtr[srcPos + 4 + 2]) / 2.0f;
            ptr[dstPos++] = (srcPtr[srcPos + 3] + srcPtr[srcPos + 4 + 3]) / 2.0f;
        }
        return tmpData;
    }

    ByteBuffer tmpData(w * h * sizeof(float));
    float *ptr = tmpData.ptrAsFloat();
    int pitch = w * 4;
    for (int srcPos = 0, dstPos = 0; dstPos < w * h; srcPos += pitch)
    {
        for (int x = 0; x < (w / 2); x++, srcPos += 8)
        {
            ptr[dstPos++] = (srcPtr[srcPos + 0] + srcPtr[srcPos + 4 + 0] + srcPtr[srcPos + pitch + 0] + srcPtr[srcPos + pitch + 4 + 0]) / 4.0f;
            ptr[dstPos++] = (srcPtr[srcPos + 1] + srcPtr[srcPos + 4 + 1] + srcPtr[srcPos + pitch + 1] + srcPtr[srcPos + pitch + 4 + 1]) / 4.0f;
            ptr[dstPos++] = (srcPtr[srcPos + 2] + srcPtr[srcPos + 4 + 2] + srcPtr[srcPos + pitch + 2] + srcPtr[srcPos + pitch + 4 + 2]) / 4.0f;
            ptr[dstPos++] = (srcPtr[srcPos + 3] + srcPtr[srcPos + 4 + 3] + srcPtr[srcPos + pitch + 3] + srcPtr[srcPos + pitch + 4 + 3]) / 4.0f;
        }
    }
    return tmpData;
}

void Image::saveToPng(const ByteBuffer src, int w, int h, PixelFormat format, const QString &filename, bool storeAs8bits, bool clearAlpha)
{
    auto dataARGB = convertRawToInternal(src, w, h, format, clearAlpha);
    quint8 *buffer;
    quint32 bufferSize;
    if (PngWrite(dataARGB.ptrAsFloat(), &buffer, &bufferSize, w, h, storeAs8bits) != 0)
    {
        PERROR("Failed to save to PNG.\n");
        return;
    }
    FileStream fs = FileStream(filename, FileMode::Create, FileAccess::WriteOnly);
    fs.WriteFromBuffer(buffer, bufferSize);
    free(buffer);
    dataARGB.Free();
}

ByteBuffer Image::convertToFormat(PixelFormat srcFormat, const ByteBuffer src, int w, int h, PixelFormat dstFormat,
                                  bool dxt1HasAlpha, float dxt1Threshold, float bc7quality)
{
    ByteBuffer tempData;

    switch (dstFormat)
    {
        case PixelFormat::DXT1:
        case PixelFormat::DXT3:
        case PixelFormat::DXT5:
        case PixelFormat::ATI2:
        case PixelFormat::BC5:
        case PixelFormat::BC7:
        {
            if (dstFormat == PixelFormat::ATI2 && (w < 4 || h < 4))
            {
                tempData = ByteBuffer(MipMap::getBufferSize(w, h, dstFormat));
                memset(tempData.ptr(), 0, tempData.size());
            }
            else if (w < 4 || h < 4)
            {
                if (w < 4)
                    w = 4;
                if (h < 4)
                    h = 4;
                tempData = ByteBuffer(MipMap::getBufferSize(w, h, dstFormat));
                memset(tempData.ptr(), 0, tempData.size());
            }
            else
            {
                ByteBuffer tempDataInternal = convertRawToInternal(src, w, h, srcFormat);
                tempData = compressMipmap(dstFormat, tempDataInternal, w, h, dxt1HasAlpha, dxt1Threshold, bc7quality);
                tempDataInternal.Free();
            }
            break;
        }
        case PixelFormat::ARGB:
            tempData = convertRawToARGB(src, w, h, srcFormat);
            break;
        case PixelFormat::RGBA:
            tempData = convertRawToRGBA(src, w, h, srcFormat);
            break;
        case PixelFormat::R10G10B10A2:
            tempData = convertRawToR10G10B10A2(src, w, h, srcFormat);
            break;
        case PixelFormat::R16G16B16A16:
            tempData = convertRawToR16G16B16A16(src, w, h, srcFormat);
            break;
        case PixelFormat::RGB:
            tempData = convertRawToRGB(src, w, h, srcFormat);
            break;
        case PixelFormat::V8U8:
        {
            ByteBuffer tempDataInternal = convertRawToInternal(src, w, h, srcFormat);
            tempData = InternalToV8U8(tempDataInternal, w, h);
            tempDataInternal.Free();
            break;
        }
        case PixelFormat::G8:
        {
            ByteBuffer tempDataInternal = convertRawToInternal(src, w, h, srcFormat);
            tempData = InternalToG8(tempDataInternal, w, h);
            tempDataInternal.Free();
            break;
        }
        case PixelFormat::RGBE:
        {
            ByteBuffer tempDataInternal = convertRawToInternal(src, w, h, srcFormat);
            tempData = InternalToRGBE(tempDataInternal, w, h);
            tempDataInternal.Free();
            break;
        }
        default:
            CRASH("Not supported format.\n");
    }

    return tempData;
}

void Image::correctMips(PixelFormat dstFormat, bool dxt1HasAlpha, float dxt1Threshold, float bc7quality)
{
    MipMap *firstMip = mipMaps.first();
    auto tempData = convertRawToInternal(firstMip->getRefData(), firstMip->getWidth(), firstMip->getHeight(), pixelFormat);

    int width = firstMip->getOrigWidth();
    int height = firstMip->getOrigHeight();

    int numberToRemove = mipMaps.count() - 1;
    if (dstFormat != pixelFormat || (dstFormat == PixelFormat::DXT1 && !dxt1HasAlpha))
        numberToRemove++;
    for (int l = 0; l < numberToRemove; l++)
    {
        mipMaps.last()->Free();
        delete mipMaps.last();
        mipMaps.removeLast();
    }

    if (dstFormat != pixelFormat || (dstFormat == PixelFormat::DXT1 && !dxt1HasAlpha))
    {
        auto top = convertToFormat(PixelFormat::Internal,
                                   tempData, width, height, dstFormat, dxt1HasAlpha, dxt1Threshold, bc7quality);
        mipMaps.push_back(new MipMap(top, width, height, dstFormat));
        top.Free();
        pixelFormat = dstFormat;
    }
    if (dstFormat == PixelFormat::RGBE)
    {
        tempData.Free();
        return;
    }

    int prevW, prevH;
    int origW = width;
    int origH = height;
    for (;;)
    {
        prevW = width;
        prevH = height;
        origW >>= 1;
        origH >>= 1;
        if (origW == 0 && origH == 0)
            break;
        if (origW == 0)
            origW = 1;
        if (origH == 0)
            origH = 1;
        width = origW;
        height = origH;

        if (pixelFormat == PixelFormat::ATI2 && (width < 4 || height < 4))
        {
            mipMaps.push_back(new MipMap(width, height, pixelFormat));
            continue;
        }

        if (pixelFormat == PixelFormat::DXT1 ||
            pixelFormat == PixelFormat::DXT3 ||
            pixelFormat == PixelFormat::DXT5 ||
            pixelFormat == PixelFormat::BC5 ||
            pixelFormat == PixelFormat::BC7)
        {
            if (width < 4 || height < 4)
            {
                if (width < 4)
                    width = 4;
                if (height < 4)
                    height = 4;
                mipMaps.push_back(new MipMap(origW, origH, pixelFormat));
                continue;
            }
        }

        auto tempDataDownscaled = downscaleInternal(tempData, prevW, prevH);
        if (pixelFormat != PixelFormat::Internal)
        {
            auto converted = convertToFormat(PixelFormat::Internal, tempDataDownscaled, origW, origH,
                                             pixelFormat, dxt1HasAlpha, dxt1Threshold, bc7quality);
            mipMaps.push_back(new MipMap(converted, origW, origH, pixelFormat));
            converted.Free();
        }
        else
        {
            mipMaps.push_back(new MipMap(tempDataDownscaled, origW, origH, pixelFormat));
        }
        tempData.Free();
        tempData = tempDataDownscaled;
    }
    tempData.Free();
}

PixelFormat Image::getPixelFormatType(const QString &format)
{
    if (format == "PF_DXT1")
        return PixelFormat::DXT1;
    if (format == "PF_DXT3")
        return PixelFormat::DXT3;
    if (format == "PF_DXT5")
        return PixelFormat::DXT5;
    if (format == "PF_NormalMap_HQ")
        return PixelFormat::ATI2;
    if (format == "PF_V8U8")
        return PixelFormat::V8U8;
    if (format == "PF_A8R8G8B8")
        return PixelFormat::ARGB;
    if (format == "PF_R8G8B8")
        return PixelFormat::RGB;
    if (format == "PF_G8")
        return PixelFormat::G8;
    if (format == "PF_BC5")
        return PixelFormat::BC5;
    if (format == "PF_BC7")
        return PixelFormat::BC7;
    if (format == "PF_A2B10G10R10")
        return PixelFormat::R10G10B10A2;
    if (format == "PF_A16B16G16R16_UNORM")
        return PixelFormat::R16G16B16A16;

    CRASH_MSG("Invalid texture format.");
}

QString Image::getEngineFormatType(PixelFormat format)
{
    switch (format)
    {
        case PixelFormat::DXT1:
            return "PF_DXT1";
        case PixelFormat::DXT3:
            return "PF_DXT3";
        case PixelFormat::DXT5:
            return "PF_DXT5";
        case PixelFormat::ATI2:
            return "PF_NormalMap_HQ";
        case PixelFormat::V8U8:
            return "PF_V8U8";
        case PixelFormat::ARGB:
            return "PF_A8R8G8B8";
        case PixelFormat::RGB:
            return "PF_R8G8B8";
        case PixelFormat::G8:
            return "PF_G8";
        case PixelFormat::BC5:
            return "PF_BC5";
        case PixelFormat::BC7:
            return "PF_BC7";
        case PixelFormat::R10G10B10A2:
            return "PF_A2B10G10R10";
        case PixelFormat::R16G16B16A16:
            return "PF_A16B16G16R16_UNORM";
        default:
            CRASH_MSG("Invalid texture format.");
    }
}

bool Image::checkPowerOfTwo(int n)
{
    return (n & (n - 1)) == 0;
}

int Image::returnPowerOfTwo(int n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;

    return n;
}
