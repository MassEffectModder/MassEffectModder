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

#ifndef MIPMAP_H
#define MIPMAP_H

#include "MemTypes.h"
#include "Helpers/ByteBuffer.h"

class MipMap
{
private:

    ByteBuffer buffer;
    int width;
    int height;
    int origWidth;
    int origHeight;

public:

    MipMap(int w, int h, PixelFormat format);
    MipMap(const ByteBuffer &data, int w, int h, PixelFormat format);
    void Free() { buffer.Free(); }
    static int getBufferSize(int w, int h, PixelFormat format);
    ByteBuffer& getRefData() { return buffer; }
    int getWidth() { return width; }
    int getHeight() { return height; }
    int getOrigWidth() { return origWidth; }
    int getOrigHeight() { return origHeight; }
};

#endif
