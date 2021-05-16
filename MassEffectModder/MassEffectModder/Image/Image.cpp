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

#include <Image/Image.h>
#include <Helpers/MemoryStream.h>
#include <Helpers/FileStream.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <Wrappers.h>

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
            LoadImageFromStream(file, format);
            return;
        }
        case ImageFormat::PNG:
        {
            FileStream file(fileName, FileMode::Open, FileAccess::ReadOnly);
            ByteBuffer buffer = file.ReadToBuffer(file.Length());
            LoadImageFromBuffer(buffer, format);
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
            LoadImageFromStream(stream, format);
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
            LoadImageFromStream(stream, format);
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
            LoadImageFromStream(stream, format);
            return;
        }
        case ImageFormat::PNG:
        {
            LoadImageFromBuffer(data, format);
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
            LoadImageFromStream(stream, format);
            return;
        }
        case ImageFormat::PNG:
        {
            LoadImageFromBuffer(data, format);
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

void Image::LoadImageFromStream(Stream &stream, ImageFormat format)
{
    foreach(MipMap *mipmap, mipMaps)
    {
        mipmap->Free();
        delete mipmap;
    }
    mipMaps.clear();

    switch (format)
    {
        case ImageFormat::BMP:
            {
                LoadImageBMP(stream);
                return;
            }
        case ImageFormat::DDS:
            {
                LoadImageDDS(stream);
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

void Image::LoadImageFromBuffer(ByteBuffer data, ImageFormat format)
{
    foreach(MipMap *mipmap, mipMaps)
    {
        mipmap->Free();
        delete mipmap;
    }
    mipMaps.clear();

    quint8 *imageBuffer;
    quint32 imageSize, imageWidth, imageHeight;
    if (format == ImageFormat::PNG)
    {
        if (PngRead(data.ptr(), data.size(), &imageBuffer,
                    &imageSize, &imageWidth, &imageHeight) != 0)
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

    ByteBuffer pixels(imageBuffer, imageSize);
    delete[] imageBuffer;
    mipMaps.push_back(new MipMap(pixels, imageWidth, imageHeight, PixelFormat::ARGB));
    pixels.Free();
    pixelFormat = PixelFormat::ARGB;
}

ByteBuffer Image::convertRawToARGB(const quint8 *src, int w, int h, PixelFormat format, bool clearAlpha)
{
    ByteBuffer tmpPtr;

    switch (format)
    {
        case PixelFormat::DXT1:
        case PixelFormat::DXT3:
        case PixelFormat::DXT5:
            if (w < 4 || h < 4)
            {
                if (w < 4)
                    w = 4;
                if (h < 4)
                    h = 4;
                tmpPtr = ByteBuffer(w * h * 4);
                memset(tmpPtr.ptr(), 0, tmpPtr.size());
                return tmpPtr;
            }
            tmpPtr = decompressMipmap(format, src, w, h);
            break;
        case PixelFormat::ATI2:
            if (w < 4 || h < 4)
            {
                tmpPtr = ByteBuffer(w * h * 4);
                memset(tmpPtr.ptr(), 0, tmpPtr.size());
                return tmpPtr;
            }
            tmpPtr = decompressMipmap(format, src, w, h);
            break;
        case PixelFormat::ARGB:
        {
            tmpPtr = ByteBuffer(src, w * h * 4);
            break;
        }
        case PixelFormat::RGBA: tmpPtr = RGBAToARGB(src, w, h); break;
        case PixelFormat::RGB: tmpPtr = RGBToARGB(src, w, h); break;
        case PixelFormat::V8U8: tmpPtr = V8U8ToARGB(src, w, h); break;
        case PixelFormat::G8: tmpPtr = G8ToARGB(src, w, h); break;
        default:
            CRASH_MSG("Invalid texture format.");
    }

    if (clearAlpha)
        clearAlphaFromARGB(tmpPtr.ptr(), w, h);

    return tmpPtr;
}

ByteBuffer Image::convertRawToRGB(const quint8 *src, int w, int h, PixelFormat format)
{
    auto dataARGB = convertRawToARGB(src, w, h, format);
    auto dataRGB = ARGBtoRGB(dataARGB.ptr(), w, h);
    dataARGB.Free();
    return dataRGB;
}

ByteBuffer Image::convertRawToRGBA(const quint8 *src, int w, int h, PixelFormat format, bool clearAlpha)
{
    return convertRawToARGB(src, w, h, format, clearAlpha);
}

ByteBuffer Image::convertRawToBGR(const quint8 *src, int w, int h, PixelFormat format)
{
    auto dataARGB = convertRawToARGB(src, w, h, format);
    auto dataRGB = ARGBtoBGR(dataARGB.ptr(), w, h);
    dataARGB.Free();
    return dataRGB;
}

ByteBuffer Image::convertRawToAlphaGreyscale(const quint8 *src, int w, int h, PixelFormat format)
{
    auto dataARGB = convertRawToARGB(src, w, h, format);
    auto dataRGB = ARGBtoAlphaGreyscale(dataARGB.ptr(), w, h);
    dataARGB.Free();
    return dataRGB;
}

void Image::clearAlphaFromARGB(quint8 *data, int w, int h)
{
    for (int i = 0; i < w * h; i++)
    {
        data[4 * i + 3] = 255;
    }
}

ByteBuffer Image::RGBToARGB(const quint8 *src, int w, int h)
{
    ByteBuffer tmpData(w * h * 4);
    quint8 *ptr = tmpData.ptr();
    for (int i = 0; i < w * h; i++)
    {
        ptr[4 * i + 0] = src[3 * i + 0];
        ptr[4 * i + 1] = src[3 * i + 1];
        ptr[4 * i + 2] = src[3 * i + 2];
        ptr[4 * i + 3] = 255;
    }
    return tmpData;
}

ByteBuffer Image::RGBAToARGB(const quint8 *src, int w, int h)
{
    ByteBuffer tmpData(w * h * 4);
    quint8 *ptr = tmpData.ptr();
    for (int i = 0; i < w * h; i++)
    {
        ptr[4 * i + 0] = src[4 * i + 2];
        ptr[4 * i + 1] = src[4 * i + 1];
        ptr[4 * i + 2] = src[4 * i + 0];
        ptr[4 * i + 3] = src[4 * i + 3];
    }
    return tmpData;
}

ByteBuffer Image::ARGBtoRGB(const quint8 *src, int w, int h)
{
    ByteBuffer tmpData(w * h * 3);
    quint8 *ptr = tmpData.ptr();
    for (int i = 0; i < w * h; i++)
    {
        ptr[3 * i + 0] = src[4 * i + 0];
        ptr[3 * i + 1] = src[4 * i + 1];
        ptr[3 * i + 2] = src[4 * i + 2];
    }
    return tmpData;
}

ByteBuffer Image::ARGBtoBGR(const quint8 *src, int w, int h)
{
    ByteBuffer tmpData(w * h * 3);
    quint8 *ptr = tmpData.ptr();
    for (int i = 0; i < w * h; i++)
    {
        ptr[3 * i + 0] = src[4 * i + 2];
        ptr[3 * i + 1] = src[4 * i + 1];
        ptr[3 * i + 2] = src[4 * i + 0];
    }
    return tmpData;
}

ByteBuffer Image::V8U8ToARGB(const quint8 *src, int w, int h)
{
    ByteBuffer tmpData(w * h * 4);
    quint8 *ptr = tmpData.ptr();
    for (int i = 0; i < w * h; i++)
    {
        ptr[4 * i + 0] = 255;
        ptr[4 * i + 1] = (quint8)(((qint8)src[2 * i + 1]) + 128);
        ptr[4 * i + 2] = (quint8)(((qint8)src[2 * i + 0]) + 128);
        ptr[4 * i + 3] = 255;
    }
    return tmpData;
}

ByteBuffer Image::ARGBtoV8U8(const quint8 *src, int w, int h)
{
    ByteBuffer tmpData(w * h * 2);
    quint8 *ptr = tmpData.ptr();
    for (int i = 0; i < w * h; i++)
    {
        ptr[2 * i + 0] = (quint8)((qint8)(src[4 * i + 2]) - 128);
        ptr[2 * i + 1] = (quint8)((qint8)(src[4 * i + 1]) - 128);
    }
    return tmpData;
}

ByteBuffer Image::G8ToARGB(const quint8 *src, int w, int h)
{
    ByteBuffer tmpData(w * h * 4);
    quint8 *ptr = tmpData.ptr();
    for (int i = 0; i < w * h; i++)
    {
        ptr[4 * i + 0] = src[i];
        ptr[4 * i + 1] = src[i];
        ptr[4 * i + 2] = src[i];
        ptr[4 * i + 3] = 255;
    }

    return tmpData;
}

ByteBuffer Image::ARGBtoG8(const quint8 *src, int w, int h)
{
    ByteBuffer tmpData(w * h);
    quint8 *ptr = tmpData.ptr();
    for (int i = 0; i < w * h; i++)
    {
        int c = src[i * 4 + 0] + src[i * 4 + 1] + src[i * 4 + 2];
        ptr[i] = (quint8)(c / 3);
    }

    return tmpData;
}

ByteBuffer Image::ARGBtoAlphaGreyscale(const quint8 *src, int w, int h)
{
    ByteBuffer tmpData(w * h * 3);
    quint8 *ptr = tmpData.ptr();
    for (int i = 0; i < w * h; i++)
    {
        quint8 alpha = src[4 * i + 3];
        ptr[3 * i + 0] = alpha;
        ptr[3 * i + 1] = alpha;
        ptr[3 * i + 2] = alpha;
    }
    return tmpData;
}

ByteBuffer Image::downscaleARGB(const quint8 *src, int w, int h)
{
    if (w == 1 && h == 1)
        CRASH_MSG("1x1 can not be downscaled");

    if (w == 1 || h == 1)
    {
        ByteBuffer tmpData(w * h * 2);
        quint8 *ptr = tmpData.ptr();
        for (int srcPos = 0, dstPos = 0; dstPos < w * h * 2; srcPos += 8)
        {
            ptr[dstPos++] = (quint8)((uint)(src[srcPos + 0] + src[srcPos + 4 + 0]) >> 1);
            ptr[dstPos++] = (quint8)((uint)(src[srcPos + 1] + src[srcPos + 4 + 1]) >> 1);
            ptr[dstPos++] = (quint8)((uint)(src[srcPos + 2] + src[srcPos + 4 + 2]) >> 1);
            ptr[dstPos++] = (quint8)((uint)(src[srcPos + 3] + src[srcPos + 4 + 3]) >> 1);
        }
        return tmpData;
    }

    ByteBuffer tmpData(w * h);
    quint8 *ptr = tmpData.ptr();
    int pitch = w * 4;
    for (int srcPos = 0, dstPos = 0; dstPos < w * h; srcPos += pitch)
    {
        for (int x = 0; x < (w / 2); x++, srcPos += 8)
        {
            ptr[dstPos++] = (quint8)((uint)(src[srcPos + 0] + src[srcPos + 4 + 0] + src[srcPos + pitch + 0] + src[srcPos + pitch + 4 + 0]) >> 2);
            ptr[dstPos++] = (quint8)((uint)(src[srcPos + 1] + src[srcPos + 4 + 1] + src[srcPos + pitch + 1] + src[srcPos + pitch + 4 + 1]) >> 2);
            ptr[dstPos++] = (quint8)((uint)(src[srcPos + 2] + src[srcPos + 4 + 2] + src[srcPos + pitch + 2] + src[srcPos + pitch + 4 + 2]) >> 2);
            ptr[dstPos++] = (quint8)((uint)(src[srcPos + 3] + src[srcPos + 4 + 3] + src[srcPos + pitch + 3] + src[srcPos + pitch + 4 + 3]) >> 2);
        }
    }
    return tmpData;
}

ByteBuffer Image::downscaleRGB(const quint8 *src, int w, int h)
{
    if (w == 1 && h == 1)
        CRASH_MSG("1x1 can not be downscaled");

    if (w == 1 || h == 1)
    {
        ByteBuffer tmpData((w * h * 3) / 2);
        quint8 *ptr = tmpData.ptr();
        for (int srcPos = 0, dstPos = 0; dstPos < (w * h * 3) / 2; srcPos += 6)
        {
            ptr[dstPos++] = (quint8)((uint)(src[srcPos + 0] + src[srcPos + 3 + 0]) >> 1);
            ptr[dstPos++] = (quint8)((uint)(src[srcPos + 1] + src[srcPos + 3 + 1]) >> 1);
            ptr[dstPos++] = (quint8)((uint)(src[srcPos + 2] + src[srcPos + 3 + 2]) >> 1);
        }
        return tmpData;
    }

    ByteBuffer tmpData((w * h * 3) / 4);
    quint8 *ptr = tmpData.ptr();
    int pitch = w * 3;
    for (int srcPos = 0, dstPos = 0; dstPos < (w * h * 3) / 4; srcPos += pitch)
    {
        for (int x = 0; x < (w / 2); x++, srcPos += 6)
        {
            ptr[dstPos++] = (quint8)((uint)(src[srcPos + 0] + src[srcPos + 3 + 0] + src[srcPos + pitch + 0] + src[srcPos + pitch + 3 + 0]) >> 2);
            ptr[dstPos++] = (quint8)((uint)(src[srcPos + 1] + src[srcPos + 3 + 1] + src[srcPos + pitch + 1] + src[srcPos + pitch + 3 + 1]) >> 2);
            ptr[dstPos++] = (quint8)((uint)(src[srcPos + 2] + src[srcPos + 3 + 2] + src[srcPos + pitch + 2] + src[srcPos + pitch + 3 + 2]) >> 2);
        }
    }

    return tmpData;
}

void Image::saveToPng(const quint8 *src, int w, int h, PixelFormat format, const QString &filename)
{
    auto dataARGB = convertRawToARGB(src, w, h, format, true);
    quint8 *buffer;
    quint32 bufferSize;
    if (PngWrite(dataARGB.ptr(), &buffer, &bufferSize, w, h) != 0)
    {
        PERROR("Failed to save to PNG.\n");
        return;
    }
    FileStream fs = FileStream(filename, FileMode::Create, FileAccess::WriteOnly);
    fs.WriteFromBuffer(buffer, bufferSize);
    free(buffer);
    dataARGB.Free();
}

ByteBuffer Image::convertToFormat(PixelFormat srcFormat, const quint8 *src, int w, int h, PixelFormat dstFormat,
                               bool dxt1HasAlpha, quint8 dxt1Threshold)
{
    ByteBuffer tempData;

    switch (dstFormat)
    {
        case PixelFormat::DXT1:
        case PixelFormat::DXT3:
        case PixelFormat::DXT5:
        case PixelFormat::ATI2:
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
                ByteBuffer tempDataARGB = convertRawToARGB(src, w, h, srcFormat);
                tempData = compressMipmap(dstFormat, tempDataARGB.ptr(), w, h, dxt1HasAlpha, dxt1Threshold);
                tempDataARGB.Free();
            }
            break;
        }
        case PixelFormat::ARGB:
            tempData = convertRawToARGB(src, w, h, srcFormat);
            break;
        case PixelFormat::RGBA:
            tempData = convertRawToRGBA(src, w, h, srcFormat);
            break;
        case PixelFormat::RGB:
            tempData = convertRawToRGB(src, w, h, srcFormat);
            break;
        case PixelFormat::V8U8:
        {
            ByteBuffer tempDataV8U8 = convertRawToARGB(src, w, h, srcFormat);
            tempData = ARGBtoV8U8(tempDataV8U8.ptr(), w, h);
            tempDataV8U8.Free();
            break;
        }
        case PixelFormat::G8:
        {
            ByteBuffer tempDataG8 = convertRawToARGB(src, w, h, srcFormat);
            tempData = ARGBtoG8(tempDataG8.ptr(), w, h);
            tempDataG8.Free();
            break;
        }
        default:
            CRASH("Not supported format.\n");
    }

    return tempData;
}

void Image::correctMips(PixelFormat dstFormat, bool dxt1HasAlpha, quint8 dxt1Threshold)
{
    MipMap *firstMip = mipMaps.first();
    auto tempData = convertRawToARGB(firstMip->getRefData().ptr(), firstMip->getWidth(), firstMip->getHeight(), pixelFormat);

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
        auto top = convertToFormat(PixelFormat::ARGB,
                                   tempData.ptr(), width, height, dstFormat, dxt1HasAlpha, dxt1Threshold);
        mipMaps.push_back(new MipMap(top, width, height, dstFormat));
        top.Free();
        pixelFormat = dstFormat;
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
            pixelFormat == PixelFormat::DXT5)
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

        auto tempDataDownscaled = downscaleARGB(tempData.ptr(), prevW, prevH);
        if (pixelFormat != PixelFormat::ARGB)
        {
            auto converted = convertToFormat(PixelFormat::ARGB, tempDataDownscaled.ptr(), origW, origH, pixelFormat, dxt1HasAlpha, dxt1Threshold);
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
