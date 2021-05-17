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
#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <Wrappers.h>

void Image::LoadImageDDS(Stream &stream)
{
    if (stream.ReadUInt32() != DDS_TAG)
    {
        PERROR("The data has not DDS header!\n");
        return;
    }

    if (stream.ReadInt32() != DDS_HEADER_dwSize)
    {
        PERROR("The data has wrong DDS header dwSize.\n");
        return;
    }

    DDSflags = stream.ReadUInt32();

    int dwHeight = stream.ReadInt32();
    int dwWidth = stream.ReadInt32();
    if (!checkPowerOfTwo(dwWidth) ||
        !checkPowerOfTwo(dwHeight))
    {
        PERROR("DDS image has dimensions not power of two.\n");
        return;
    }

    stream.Skip(8); // dwPitchOrLinearSize, dwDepth

    int dwMipMapCount = stream.ReadInt32();
    if (dwMipMapCount == 0)
        dwMipMapCount = 1;

    stream.Skip(11 * 4); // dwReserved1
    stream.SkipInt32(); // ppf.dwSize

    ddsPixelFormat.flags = stream.ReadUInt32();
    ddsPixelFormat.fourCC = stream.ReadUInt32();
    ddsPixelFormat.bits = stream.ReadUInt32();
    ddsPixelFormat.Rmask = stream.ReadUInt32();
    ddsPixelFormat.Gmask = stream.ReadUInt32();
    ddsPixelFormat.Bmask = stream.ReadUInt32();
    ddsPixelFormat.Amask = stream.ReadUInt32();

    switch (ddsPixelFormat.fourCC)
    {
        case 0:
            if (ddsPixelFormat.bits == 32 &&
                (ddsPixelFormat.flags & DDPF_ALPHAPIXELS) != 0 &&
                   ddsPixelFormat.Rmask == 0xFF0000 &&
                   ddsPixelFormat.Gmask == 0xFF00 &&
                   ddsPixelFormat.Bmask == 0xFF &&
                   ddsPixelFormat.Amask == 0xFF000000)
            {
                pixelFormat = PixelFormat::ARGB;
                break;
            }
            if (ddsPixelFormat.bits == 24 &&
                   ddsPixelFormat.Rmask == 0xFF0000 &&
                   ddsPixelFormat.Gmask == 0xFF00 &&
                   ddsPixelFormat.Bmask == 0xFF)
            {
                pixelFormat = PixelFormat::RGB;
                break;
            }
            if (ddsPixelFormat.bits == 16 &&
                   ddsPixelFormat.Rmask == 0xFF &&
                   ddsPixelFormat.Gmask == 0xFF00 &&
                   ddsPixelFormat.Bmask == 0x00)
            {
                pixelFormat = PixelFormat::V8U8;
                break;
            }
            if (ddsPixelFormat.bits == 8 &&
                ddsPixelFormat.Rmask == 0xFF &&
                ddsPixelFormat.Gmask == 0x00 &&
                ddsPixelFormat.Bmask == 0x00)
            {
                pixelFormat = PixelFormat::G8;
                break;
            }

            PERROR("Not supported DDS format.\n");
            return;

        case 21:
            pixelFormat = PixelFormat::ARGB;
            break;

        case 20:
            pixelFormat = PixelFormat::RGB;
            break;

        case 60:
            pixelFormat = PixelFormat::V8U8;
            break;

        case 50:
            pixelFormat = PixelFormat::G8;
            break;

        case FOURCC_DXT1_TAG:
            pixelFormat = PixelFormat::DXT1;
            break;

        case FOURCC_DXT3_TAG:
            pixelFormat = PixelFormat::DXT3;
            break;

        case FOURCC_DXT5_TAG:
            pixelFormat = PixelFormat::DXT5;
            break;

        case FOURCC_ATI2_TAG:
            pixelFormat = PixelFormat::ATI2;
            break;

        case FOURCC_DX10_TAG:
            DX10Type = true;
            break;

        default:
            PERROR("Not supported DDS format.\n");
            return;
    }
    stream.Skip(20); // dwCaps, dwCaps2, dwCaps3, dwCaps4, dwReserved2

    DDS_FORMAT dds10Format = DDS_FORMAT_UNKNOWN;
    DDS_RESOURCE_DIMENSION dds10ResDim = DDS_RESOURCE_DIMENSION_UNKNOWN;
    quint32 dds10NumElements = 0;
    if (DX10Type)
    {
        dds10Format = (DDS_FORMAT)stream.ReadUInt32();
        dds10ResDim = (DDS_RESOURCE_DIMENSION)stream.ReadUInt32();
        if (dds10ResDim != DDS_RESOURCE_DIMENSION_TEXTURE2D)
        {
            PERROR("DDS DX10 dimension resource different than Texture2D is not supported.\n");
            return;
        }
        auto miscFlags = (DDS_RESOURCE_MISC_FLAG)stream.ReadUInt32();
        if (miscFlags & DDS_RESOURCE_MISC_TEXTURECUBE)
        {
            PERROR("DDS DX10 dimension resource flag cube is not supported.\n");
            return;
        }
        dds10NumElements = stream.ReadUInt32();
        if (dds10NumElements != 1)
        {
            PERROR("DDS DX10 number of elements must be 1.\n");
            return;
        }
        auto miscFlags2 = (DDS_ALPHA_MODE)stream.ReadUInt32();
        if (miscFlags2 != DDS_ALPHA_MODE_UNKNOWN && miscFlags2 != DDS_ALPHA_MODE_OPAQUE)
        {
            PERROR("DDS DX10 alpha mode different than opaque is not supported.\n");
            return;
        }

        switch (dds10Format)
        {
            case DDS_FORMAT_R8G8B8A8_UNORM:
                pixelFormat = PixelFormat::RGBA;
                break;
            case DDS_FORMAT_R8_UNORM:
                pixelFormat = PixelFormat::G8;
                DX10Type = false;
                break;
            case DDS_FORMAT_BC1_UNORM:
                pixelFormat = PixelFormat::DXT1;
                DX10Type = false;
                break;
            case DDS_FORMAT_BC2_UNORM:
                pixelFormat = PixelFormat::DXT3;
                DX10Type = false;
                break;
            case DDS_FORMAT_BC3_UNORM:
                pixelFormat = PixelFormat::DXT5;
                DX10Type = false;
                break;
            case DDS_FORMAT_BC5_UNORM:
                pixelFormat = PixelFormat::ATI2;
                DX10Type = false;
                break;
            case DDS_FORMAT_BC7_UNORM:
                pixelFormat = PixelFormat::BC7;
                DX10Type = true;
                break;
            default:
                PERROR("Not supported DDS DX10 format.\n");
                return;
        }
    }

    for (int i = 0; i < dwMipMapCount; i++)
    {
        int w = dwWidth >> i;
        int h = dwHeight >> i;
        int origW = w;
        int origH = h;
        if (origW == 0 && origH != 0)
            origW = 1;
        if (origH == 0 && origW != 0)
            origH = 1;
        w = origW;
        h = origH;

        if (pixelFormat == PixelFormat::DXT1 ||
            pixelFormat == PixelFormat::DXT3 ||
            pixelFormat == PixelFormat::DXT5 ||
            pixelFormat == PixelFormat::BC5 ||
            pixelFormat == PixelFormat::BC7)
        {
            if (w < 4)
                w = 4;
            if (h < 4)
                h = 4;
        }

        int size = MipMap::getBufferSize(w, h, pixelFormat);
        ByteBuffer tempData = stream.ReadToBuffer(size);
        mipMaps.push_back(new MipMap(tempData, origW, origH, pixelFormat));
        tempData.Free();
    }
}

bool Image::checkDDSHaveAllMipmaps()
{
    if ((DDSflags & DDSD_MIPMAPCOUNT) != 0 && mipMaps.count() > 1)
    {
        int width = mipMaps[0]->getOrigWidth();
        int height = mipMaps[0]->getOrigHeight();
        for (int i = 0; i < mipMaps.count(); i++)
        {
            if (mipMaps[i]->getOrigWidth() < 4 || mipMaps[i]->getOrigHeight() < 4)
                return true;
            if (mipMaps[i]->getOrigWidth() != width && mipMaps[i]->getOrigHeight() != height)
                return false;
            width /= 2;
            height /= 2;
        }
        return true;
    }

    return true;
}

Image *Image::convertToARGB()
{
    for (int i = 0; i < mipMaps.count(); i++)
    {
        ByteBuffer data = convertRawToARGB(mipMaps[i]->getRefData().ptr(),
                                           mipMaps[i]->getWidth(),
                                           mipMaps[i]->getHeight(),
                                           pixelFormat);

        mipMaps[i] = new MipMap(data,
                            mipMaps[i]->getWidth(),
                            mipMaps[i]->getHeight(),
                            PixelFormat::ARGB);
        data.Free();
    }
    pixelFormat = PixelFormat::ARGB;

    return this;
}

Image *Image::convertToRGB()
{
    for (int i = 0; i < mipMaps.count(); i++)
    {
        ByteBuffer data = convertRawToRGB(mipMaps[i]->getRefData().ptr(),
                                          mipMaps[i]->getWidth(),
                                          mipMaps[i]->getHeight(),
                                          pixelFormat);
        mipMaps[i] = new MipMap(data,
                            mipMaps[i]->getWidth(),
                            mipMaps[i]->getHeight(),
                            PixelFormat::RGB);
        data.Free();
    }
    pixelFormat = PixelFormat::RGB;

    return this;
}

Image::DDS_PF Image::getDDSPixelFormat(PixelFormat format)
{
    DDS_PF pixelFormat{};
    switch (format)
    {
        case PixelFormat::DXT1:
            pixelFormat.flags = DDPF_FOURCC;
            pixelFormat.fourCC = FOURCC_DXT1_TAG;
            break;

        case PixelFormat::DXT3:
            pixelFormat.flags = DDPF_FOURCC | DDPF_ALPHAPIXELS;
            pixelFormat.fourCC = FOURCC_DXT3_TAG;
            break;

        case PixelFormat::DXT5:
            pixelFormat.flags = DDPF_FOURCC | DDPF_ALPHAPIXELS;
            pixelFormat.fourCC = FOURCC_DXT5_TAG;
            break;

        case PixelFormat::ATI2:
        case PixelFormat::BC5:
            pixelFormat.flags = DDPF_FOURCC;
            pixelFormat.fourCC = FOURCC_ATI2_TAG;
            break;

        case PixelFormat::ARGB:
            pixelFormat.flags = DDPF_ALPHAPIXELS | DDPF_RGB;
            pixelFormat.bits = 32;
            pixelFormat.Rmask = 0xFF0000;
            pixelFormat.Gmask = 0xFF00;
            pixelFormat.Bmask = 0xFF;
            pixelFormat.Amask = 0xFF000000;
            break;

        case PixelFormat::RGB:
            pixelFormat.flags = DDPF_RGB;
            pixelFormat.bits = 24;
            pixelFormat.Rmask = 0xFF0000;
            pixelFormat.Gmask = 0xFF00;
            pixelFormat.Bmask = 0xFF;
            break;

        case PixelFormat::V8U8:
            pixelFormat.flags = DDPF_SIGNED;
            pixelFormat.bits = 16;
            pixelFormat.Rmask = 0xFF;
            pixelFormat.Gmask = 0xFF00;
            break;

        case PixelFormat::G8:
            pixelFormat.flags = DDPF_LUMINANCE;
            pixelFormat.bits = 8;
            pixelFormat.Rmask = 0xFF;
            break;

        case PixelFormat::RGBA:
            pixelFormat.flags = DDPF_FOURCC;
            pixelFormat.fourCC = FOURCC_DX10_TAG;
            break;

        case PixelFormat::BC7:
            pixelFormat.flags = DDPF_FOURCC;
            pixelFormat.fourCC = FOURCC_DX10_TAG;
            break;

        default:
            CRASH_MSG("Invalid texture format.");
    }

    return pixelFormat;
}

void Image::StoreImageToDDS(Stream &stream, PixelFormat format)
{
    stream.WriteUInt32(DDS_TAG);
    stream.WriteInt32(DDS_HEADER_dwSize);
    stream.WriteUInt32(DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_MIPMAPCOUNT | DDSD_PIXELFORMAT | DDSD_LINEARSIZE);
    stream.WriteInt32(mipMaps[0]->getHeight());
    stream.WriteInt32(mipMaps[0]->getWidth());

    int dataSize = 0;
    for (int i = 0; i < mipMaps.count(); i++)
        dataSize += MipMap::getBufferSize(mipMaps[i]->getWidth(),
                                          mipMaps[i]->getHeight(),
                                          format == PixelFormat::UnknownPixelFormat ? pixelFormat : format);
    stream.WriteInt32(dataSize);

    stream.WriteUInt32(0); // dwDepth
    stream.WriteInt32(mipMaps.count());
    stream.WriteZeros(44); // dwReserved1

    stream.WriteInt32(DDS_PIXELFORMAT_dwSize);
    DDS_PF pixfmt = getDDSPixelFormat(format == PixelFormat::UnknownPixelFormat ? pixelFormat : format);
    stream.WriteUInt32(pixfmt.flags);
    stream.WriteUInt32(pixfmt.fourCC);
    stream.WriteUInt32(pixfmt.bits);
    stream.WriteUInt32(pixfmt.Rmask);
    stream.WriteUInt32(pixfmt.Gmask);
    stream.WriteUInt32(pixfmt.Bmask);
    stream.WriteUInt32(pixfmt.Amask);

    stream.WriteInt32(DDSCAPS_COMPLEX | DDSCAPS_MIPMAP | DDSCAPS_TEXTURE);
    stream.WriteUInt32(0); // dwCaps2
    stream.WriteUInt32(0); // dwCaps3
    stream.WriteUInt32(0); // dwCaps4
    stream.WriteUInt32(0); // dwReserved2

    DDS_FORMAT dds10Format = DDS_FORMAT::DDS_FORMAT_UNKNOWN;
    UINT32 miscFlag2 = 0;
    switch (format == PixelFormat::UnknownPixelFormat ? pixelFormat : format)
    {
        case PixelFormat::RGBA:
            dds10Format = DDS_FORMAT_R8G8B8A8_UNORM;
            miscFlag2 = DDS_ALPHA_MODE_OPAQUE;
            DX10Type = true;
            break;
        case PixelFormat::BC7:
            dds10Format = DDS_FORMAT_BC7_UNORM;
            DX10Type = true;
            break;
        default:
            break;
    }

    if (DX10Type)
    {
        stream.WriteUInt32(dds10Format);
        stream.WriteUInt32(DDS_RESOURCE_DIMENSION_TEXTURE2D); // RESOURCE_DIMENSION
        stream.WriteUInt32(0); // miscFlag
        stream.WriteUInt32(1); // arraySize
        stream.WriteUInt32(miscFlag2);
    }

    for (int i = 0; i < mipMaps.count(); i++)
    {
        stream.WriteFromBuffer(mipMaps[i]->getRefData().ptr(),
                               mipMaps[i]->getRefData().size());
    }
}

ByteBuffer Image::StoreImageToDDS()
{
    MemoryStream stream;
    StoreImageToDDS(stream);
    return stream.ToArray();
}

void Image::readBlock4X4ARGB(quint8 blockARGB[BLOCK_SIZE_4X4X4], const quint8 *srcARGB,
                               int srcW, int blockX, int blockY)
{
    int srcPitch = srcW * 4;
    int blockPitch = 4 * 4;
    int srcARGBPtr = (blockY * 4) * srcPitch + blockX * 4 * 4;
    auto ptr = srcARGB;

    for (int y = 0; y < 4; y++)
    {
        int blockPtr = y * blockPitch;
        int srcARGBPtrY = srcARGBPtr + (y * srcPitch);
        for (int x = 0; x < 4 * 4; x += 4)
        {
            int srcPtr = srcARGBPtrY + x;
            blockARGB[blockPtr + 0] = ptr[srcPtr + 0];
            blockARGB[blockPtr + 1] = ptr[srcPtr + 1];
            blockARGB[blockPtr + 2] = ptr[srcPtr + 2];
            blockARGB[blockPtr + 3] = ptr[srcPtr + 3];
            blockPtr += 4;
        }
    }
}

void Image::writeBlock4X4ARGB(const quint8 blockARGB[BLOCK_SIZE_4X4X4], quint8 *dstARGB,
                                int dstW, int blockX, int blockY)
{
    int dstPitch = dstW * 4;
    int blockPitch = 4 * 4;
    int dstARGBPtr = (blockY * 4) * dstPitch + blockX * 4 * 4;

    for (int y = 0; y < 4; y++)
    {
        int blockPtr = y * blockPitch;
        int dstARGBPtrY = dstARGBPtr + (y * dstPitch);
        for (int x = 0; x < 4 * 4; x += 4)
        {
            int dstPtr = dstARGBPtrY + x;
            dstARGB[dstPtr + 0] = blockARGB[blockPtr + 0];
            dstARGB[dstPtr + 1] = blockARGB[blockPtr + 1];
            dstARGB[dstPtr + 2] = blockARGB[blockPtr + 2];
            dstARGB[dstPtr + 3] = blockARGB[blockPtr + 3];
            blockPtr += 4;
        }
    }
}

void Image::readBlock4X4BPP4(uint block[2], const quint8 *src, int srcW, int blockX, int blockY)
{
    auto ptr = const_cast<quint8 *>(src);
    int offset = blockY * srcW * 2 + blockX * 2 * sizeof(uint);
    block[0] = *reinterpret_cast<uint *>(ptr + offset + 0);
    block[1] = *reinterpret_cast<uint *>(ptr + offset + 4);
}

void Image::readBlock4X4BPP8(uint block[4], const quint8 *src, int srcW, int blockX, int blockY)
{
    auto ptr = const_cast<quint8 *>(src);
    int offset = blockY * srcW * 4 + blockX * 4 * sizeof(uint);
    block[0] = *reinterpret_cast<uint *>(ptr + offset + 0);
    block[1] = *reinterpret_cast<uint *>(ptr + offset + 4);
    block[2] = *reinterpret_cast<uint *>(ptr + offset + 8);
    block[3] = *reinterpret_cast<uint *>(ptr + offset + 12);
}

void Image::writeBlock4X4BPP4(const uint block[2], quint8 *dst, int dstW, int blockX, int blockY)
{
    auto ptr = dst;
    int offset = blockY * dstW * 2 + blockX * 2 * sizeof(uint);
    *reinterpret_cast<uint *>(ptr + offset + 0) = block[0];
    *reinterpret_cast<uint *>(ptr + offset + 4) = block[1];
}

void Image::writeBlock4X4BPP8(const uint block[4], quint8 *dst, int dstW, int blockX, int blockY)
{
    auto ptr = dst;
    int offset = blockY * dstW * 4 + blockX * 4 * sizeof(uint);
    *reinterpret_cast<uint *>(ptr + offset + 0) = block[0];
    *reinterpret_cast<uint *>(ptr + offset + 4) = block[1];
    *reinterpret_cast<uint *>(ptr + offset + 8) = block[2];
    *reinterpret_cast<uint *>(ptr + offset + 12) = block[3];
}

void Image::writeBlock4X4ATI2(const uint blockSrcX[2], const uint blockSrcY[2],
                                 quint8 *dst, int dstW, int blockX, int blockY)
{
    auto ptr = dst;
    int offset = blockY * dstW * 4 + blockX * 4 * sizeof(uint);
    *reinterpret_cast<uint *>(ptr + offset + 0) = blockSrcY[0];
    *reinterpret_cast<uint *>(ptr + offset + 4) = blockSrcY[1];
    *reinterpret_cast<uint *>(ptr + offset + 8) = blockSrcX[0];
    *reinterpret_cast<uint *>(ptr + offset + 12) = blockSrcX[1];
}

void Image::readBlock4X4ATI2(quint8 blockDstX[BLOCK_SIZE_4X4BPP8], quint8 blockDstY[BLOCK_SIZE_4X4BPP8],
                               const quint8 *src, int srcW, int blockX, int blockY)
{
    int srcPitch = srcW * 4;
    int srcPtr = (blockY * 4) * srcPitch + blockX * 4 * 4;
    auto ptr = src;

    for (int y = 0; y < 4; y++)
    {
        int srcPtrY = srcPtr + (y * srcPitch);
        for (int x = 0; x < 4; x++)
        {
            blockDstX[y * 4 + x] = ptr[srcPtrY + (x * 4) + 2];
            blockDstY[y * 4 + x] = ptr[srcPtrY + (x * 4) + 1];
        }
    }
}

void Image::writeBlock4X4ARGBATI2(const quint8 blockR[BLOCK_SIZE_4X4BPP8],
                                  const quint8 blockG[BLOCK_SIZE_4X4BPP8],
                                  quint8 *dstARGB, int srcW, int blockX, int blockY)
{
    int dstPitch = srcW * 4;
    int blockPitch = 4;
    int dstARGBPtr = (blockY * 4) * dstPitch + blockX * 4 * 4;

    for (int y = 0; y < 4; y++)
    {
        int dstARGBPtrY = dstARGBPtr + (y * dstPitch);
        for (int x = 0; x < 4 * 4; x += 4)
        {
            int blockPtr = y * blockPitch + (x / 4);
            int dstPtr = dstARGBPtrY + x;
            dstARGB[dstPtr + 0] = 255;
            dstARGB[dstPtr + 1] = blockG[blockPtr];
            dstARGB[dstPtr + 2] = blockR[blockPtr];
            dstARGB[dstPtr + 3] = 255;
        }
    }
}

ByteBuffer Image::compressMipmap(PixelFormat dstFormat, const quint8 *src, int w, int h,
                                   bool useDXT1Alpha, quint8 DXT1Threshold)
{
    int blockSize = BLOCK_SIZE_4X4BPP8;
    if (dstFormat == PixelFormat::DXT1)
        blockSize = BLOCK_SIZE_4X4BPP4;

    auto dst = ByteBuffer(blockSize * (w / 4) * (h / 4));
    int cores = omp_get_max_threads();
    int partSize;
    cores = returnPowerOfTwo(cores);
    if ((cores * 4 * 4) > h)
        cores = h / 4 / 4;
    if (cores == 0)
        cores = 1;
    partSize = h / 4 / cores;

    int range[cores + 1];
    range[0] = 0;
    for (int p = 1; p <= cores; p++)
        range[p] = (partSize * p);

    #pragma omp parallel for num_threads(cores)
    for (int p = 0; p < cores; p++)
    {
        for (int y = range[p]; y < range[p + 1]; y++)
        {
            for (int x = 0; x < w / 4; x++)
            {
                if (dstFormat == PixelFormat::DXT1)
                {
                    uint block[2];
                    quint8 srcBlock[BLOCK_SIZE_4X4X4];
                    readBlock4X4ARGB(srcBlock, src, w, x, y);
                    DxtcCompressRGBBlock(srcBlock, block, true, useDXT1Alpha, DXT1Threshold);
                    writeBlock4X4BPP4(block, dst.ptr(), w, x, y);
                }
                else if (dstFormat == PixelFormat::DXT3)
                {
                    uint block[4];
                    quint8 srcBlock[BLOCK_SIZE_4X4X4];
                    readBlock4X4ARGB(srcBlock, src, w, x, y);
                    DxtcCompressRGBABlock_ExplicitAlpha(srcBlock, block);
                    writeBlock4X4BPP8(block, dst.ptr(), w, x, y);
                }
                else if (dstFormat == PixelFormat::DXT5)
                {
                    uint block[4];
                    quint8 srcBlock[BLOCK_SIZE_4X4X4];
                    readBlock4X4ARGB(srcBlock, src, w, x, y);
                    DxtcCompressRGBABlock(srcBlock, block);
                    writeBlock4X4BPP8(block, dst.ptr(), w, x, y);
                }
                else if (dstFormat == PixelFormat::ATI2)
                {
                    uint blockX[2];
                    uint blockY[2];
                    quint8 srcBlockX[BLOCK_SIZE_4X4BPP8];
                    quint8 srcBlockY[BLOCK_SIZE_4X4BPP8];
                    readBlock4X4ATI2(srcBlockX, srcBlockY, src, w, x, y);
                    DxtcCompressAlphaBlock(srcBlockX, blockX);
                    DxtcCompressAlphaBlock(srcBlockY, blockY);
                    writeBlock4X4ATI2(blockX, blockY, dst.ptr(), w, x, y);
                }
                else
                    CRASH_MSG("Not supported codec.");
            }
        }
    }

    return dst;
}

ByteBuffer Image::decompressMipmap(PixelFormat srcFormat, const quint8 *src, int w, int h)
{
    auto dst = ByteBuffer(w * h * 4);
    int cores = omp_get_max_threads();
    int partSize;
    cores = returnPowerOfTwo(cores);
    if ((cores * 4 * 4) > h)
        cores = h / 4 / 4;
    if (cores == 0)
        cores = 1;
    partSize = h / 4 / cores;

    int range[cores + 1];
    range[0] = 0;
    for (int p = 1; p <= cores; p++)
        range[p] = (partSize * p);

    #pragma omp parallel for num_threads(cores)
    for (int p = 0; p < cores; p++)
    {
        for (int y = range[p]; y < range[p + 1]; y++)
        {
            for (int x = 0; x < w / 4; x++)
            {
                if (srcFormat == PixelFormat::DXT1)
                {
                    uint block[2];
                    quint8 dstBlock[BLOCK_SIZE_4X4X4];
                    readBlock4X4BPP4(block, src, w, x, y);
                    DxtcDecompressRGBBlock(dstBlock, block, true);
                    writeBlock4X4ARGB(dstBlock, dst.ptr(), w, x, y);
                }
                else if (srcFormat == PixelFormat::DXT3)
                {
                    uint block[4];
                    quint8 dstBlock[BLOCK_SIZE_4X4X4];
                    readBlock4X4BPP8(block, src, w, x, y);
                    DxtcDecompressRGBABlock_ExplicitAlpha(dstBlock, block);
                    writeBlock4X4ARGB(dstBlock, dst.ptr(), w, x, y);
                }
                else if (srcFormat == PixelFormat::DXT5)
                {
                    uint block[4];
                    quint8 dstBlock[BLOCK_SIZE_4X4X4];
                    readBlock4X4BPP8(block, src, w, x, y);
                    DxtcDecompressRGBABlock(dstBlock, block);
                    writeBlock4X4ARGB(dstBlock, dst.ptr(), w, x, y);
                }
                else if (srcFormat == PixelFormat::ATI2)
                {
                    uint blockX[2];
                    uint blockY[2];
                    uint block[4];
                    quint8 blockDstR[BLOCK_SIZE_4X4BPP8];
                    quint8 blockDstG[BLOCK_SIZE_4X4BPP8];
                    readBlock4X4BPP8(block, src, w, x, y);
                    blockY[0] = block[0];
                    blockY[1] = block[1];
                    blockX[0] = block[2];
                    blockX[1] = block[3];
                    DxtcDecompressAlphaBlock(blockDstR, blockX);
                    DxtcDecompressAlphaBlock(blockDstG, blockY);
                    writeBlock4X4ARGBATI2(blockDstR, blockDstG, dst.ptr(), w, x, y);
                }
                else
                    CRASH_MSG("Not supported codec.");
            }
        }
    }

    return dst;
}
