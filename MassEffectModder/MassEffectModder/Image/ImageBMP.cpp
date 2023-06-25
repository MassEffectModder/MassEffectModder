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
#include <Helpers/Logs.h>
#include <Wrappers.h>

namespace {

int getShiftFromMask(uint mask)
{
    int shift = 0;

    if (mask == 0)
        return shift;

    while ((mask & 1) == 0)
    {
        mask >>= 1;
        shift++;
    }

    return shift;
}

} // namespace

void Image::LoadImageBMP(Stream &stream)
{
    ushort tag = stream.ReadUInt16();
    if (tag != BMP_TAG)
    {
        PERROR("Not BMP header.\n");
        return;
    }

    stream.Skip(8);

    stream.ReadUInt32();
    int headerSize = stream.ReadInt32();

    int imageWidth = stream.ReadInt32();
    int imageHeight = stream.ReadInt32();
    bool downToTop = true;
    if (imageHeight < 0)
    {
        imageHeight = -imageHeight;
        downToTop = false;
    }
    if (!checkPowerOfTwo(imageWidth) ||
        !checkPowerOfTwo(imageHeight))
    {
        PERROR("Dimensions not power of two.\n");
        return;
    }

    stream.Skip(2);

    int bits = stream.ReadUInt16();
    if (bits != 32 && bits != 24)
    {
        PERROR("Only 24 and 32 bits BMP supported!\n");
        return;
    }

    bool hasAlphaMask = false;
    uint Rmask = 0xFF0000, Gmask = 0xFF00, Bmask = 0xFF, Amask = 0xFF000000;
    int Rshift, Gshift, Bshift, Ashift;

    if (headerSize >= 40)
    {
        int compression = stream.ReadInt32();
        if (compression == 1 || compression == 2)
        {
            PERROR("Compression not supported in BMP!\n");
            return;
        }

        if (compression == 3)
        {
            stream.Skip(20);
            Rmask = stream.ReadUInt32();
            Gmask = stream.ReadUInt32();
            Bmask = stream.ReadUInt32();
            if (headerSize >= 56)
            {
                Amask = stream.ReadUInt32();
                hasAlphaMask = true;
            }
        }

        stream.JumpTo(headerSize + 14);
    }

    Rshift = getShiftFromMask(Rmask);
    Gshift = getShiftFromMask(Gmask);
    Bshift = getShiftFromMask(Bmask);
    Ashift = getShiftFromMask(Amask);

    auto buffer = ByteBuffer(imageWidth * imageHeight * 4 * sizeof(float));
    float *ptr = buffer.ptrAsFloat();
    int pos = downToTop ? imageWidth * (imageHeight - 1) * 4 : 0;
    int delta = downToTop ? -imageWidth * 4 * 2 : 0;
    for (int h = 0; h < imageHeight; h++)
    {
        for (int i = 0; i < imageWidth; i++)
        {
            if (bits == 24)
            {
                float b = CONVERT_BYTE_TO_FLOAT(stream.ReadByte());
                float g = CONVERT_BYTE_TO_FLOAT(stream.ReadByte());
                float r = CONVERT_BYTE_TO_FLOAT(stream.ReadByte());
                ptr[pos++] = r;
                ptr[pos++] = g;
                ptr[pos++] = b;
                ptr[pos++] = 1.0f;
            }
            else if (bits == 32)
            {
                uint p1 = stream.ReadByte();
                uint p2 = stream.ReadByte();
                uint p3 = stream.ReadByte();
                uint p4 = stream.ReadByte();
                uint pixel = p4 << 24 | p3 << 16 | p2 << 8 | p1;
                ptr[pos++] = CONVERT_BYTE_TO_FLOAT((pixel & Rmask) >> Rshift);
                ptr[pos++] = CONVERT_BYTE_TO_FLOAT((pixel & Gmask) >> Gshift);
                ptr[pos++] = CONVERT_BYTE_TO_FLOAT((pixel & Bmask) >> Bshift);
                ptr[pos++] = CONVERT_BYTE_TO_FLOAT((pixel & Amask) >> Ashift);
            }
        }
        if (imageWidth % 4 != 0)
            stream.Skip(4 - (imageWidth % 4));
        pos += delta;
    }

    if (bits == 32 && !hasAlphaMask)
    {
        bool hasAlpha = false;
        for (int i = 0; i < imageWidth * imageHeight; i++)
        {
            if (ROUND_FLOAT_TO_BYTE(ptr[4 * i + 3]) != 0)
            {
                hasAlpha = true;
                break;
            }
        }

        if (!hasAlpha)
        {
            for (int i = 0; i < imageWidth * imageHeight; i++)
            {
                ptr[4 * i + 3] = 1.0f;
            }
        }
    }

    pixelFormat = PixelFormat::Internal;

    mipMaps.push_back(new MipMap(buffer, imageWidth, imageHeight, PixelFormat::Internal));

    buffer.Free();
}
