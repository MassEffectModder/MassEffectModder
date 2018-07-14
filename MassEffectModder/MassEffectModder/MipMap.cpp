/*
 * MassEffectModder
 *
 * Copyright (C) 2018 Pawel Kolodziejski <aquadran at users.sourceforge.net>
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

#include "MipMap.h"

MipMap::MipMap(ByteBuffer src, int w, int h, PixelFormat format)
{
    width = origWidth = w;
    height = origHeight = h;

    if (format == PixelFormat::DXT1 ||
        format == PixelFormat::DXT3 ||
        format == PixelFormat::DXT5)
    {
        if (width < 4)
            width = 4;
        if (height < 4)
            height = 4;
    }

    if (src.size() != getBufferSize(width, height, format))
        CRASH_MSG("data size is not valid");

    buffer = ByteBuffer(src.size());
    memcmp(buffer.ptr(), src.ptr(), src.size());
}

int MipMap::getBufferSize(int w, int h, PixelFormat format)
{
    switch (format)
    {
        case PixelFormat::ARGB:
            return 4 * w * h;
        case PixelFormat::RGB:
            return 3 * w * h;
        case PixelFormat::V8U8:
            return 2 * w * h;
        case PixelFormat::DXT3:
        case PixelFormat::DXT5:
        case PixelFormat::ATI2:
        case PixelFormat::G8:
            return w * h;
        case PixelFormat::DXT1:
            return (w * h) / 2;
        default:
            CRASH_MSG("unknown format");
    }
}
