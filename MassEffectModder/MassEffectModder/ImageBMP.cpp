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

#include "Image.h"
#include "Helpers/MemoryStream.h"
#include "Wrappers.h"

static int getShiftFromMask(uint mask)
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

void Image::LoadImageBMP(Stream &stream)
{
    ushort tag = stream.ReadUInt16();
    if (tag != BMP_TAG)
        CRASH_MSG("Not BMP header.\n");

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
        CRASH_MSG("Dimensions not power of two.\n");

    stream.Skip(2);

    int bits = stream.ReadUInt16();
    if (bits != 32 && bits != 24)
        CRASH_MSG("Only 24 and 32 bits BMP supported!\n");

    bool hasAlphaMask = false;
    uint Rmask = 0xFF0000, Gmask = 0xFF00, Bmask = 0xFF, Amask = 0xFF000000;
    int Rshift, Gshift, Bshift, Ashift;

    if (headerSize >= 40)
    {
        int compression = stream.ReadInt32();
        if (compression == 1 || compression == 2)
            CRASH_MSG("Compression not supported in BMP!\n");

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

    auto buffer = ByteBuffer(imageWidth * imageHeight * 4);
    quint8 *ptr = buffer.ptr();
    int pos = downToTop ? imageWidth * (imageHeight - 1) * 4 : 0;
    int delta = downToTop ? -imageWidth * 4 * 2 : 0;
    for (int h = 0; h < imageHeight; h++)
    {
        for (int i = 0; i < imageWidth; i++)
        {
            if (bits == 24)
            {
                ptr[pos++] = stream.ReadByte();
                ptr[pos++] = stream.ReadByte();
                ptr[pos++] = stream.ReadByte();
                ptr[pos++] = 255;
            }
            else if (bits == 32)
            {
                uint p1 = stream.ReadByte();
                uint p2 = stream.ReadByte();
                uint p3 = stream.ReadByte();
                uint p4 = stream.ReadByte();
                uint pixel = p4 << 24 | p3 << 16 | p2 << 8 | p1;
                ptr[pos++] = (pixel & Bmask) >> Bshift;
                ptr[pos++] = (pixel & Gmask) >> Gshift;
                ptr[pos++] = (pixel & Rmask) >> Rshift;
                ptr[pos++] = (pixel & Amask) >> Ashift;
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
            if (ptr[4 * i + 3] != 0)
            {
                hasAlpha = true;
                break;
            }
        }

        if (!hasAlpha)
        {
            for (int i = 0; i < imageWidth * imageHeight; i++)
            {
                ptr[4 * i + 3] = 255;
            }
        }
    }

    pixelFormat = PixelFormat::ARGB;

    mipMaps.push_back(new MipMap(buffer, imageWidth, imageHeight, PixelFormat::ARGB));

    buffer.Free();
}
