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

#ifndef BYTE_BUFFER_H
#define BYTE_BUFFER_H

struct ByteBuffer
{
private:

    quint8 *_ptr;
    qint64 _size;

public:

    ByteBuffer()
    {
        _ptr = nullptr;
        _size = 0;
    }

    ByteBuffer(quint64 size)
    {
        _ptr = new quint8[size];
        _size = size;
    }

    ByteBuffer(const quint8 *ptr, quint64 size)
    {
        _ptr = new quint8[size];
        if (_ptr != nullptr)
        {
            memcpy(_ptr, ptr, size);
            _size = size;
        }
        else
        {
            _size = 0;
        }
    }

    void Free()
    {
        delete[] _ptr;
        _ptr = nullptr;
    }

    quint8 *ptr() const
    {
        return _ptr;
    }

    qint64 size() const
    {
        return _size;
    }
};

#endif
