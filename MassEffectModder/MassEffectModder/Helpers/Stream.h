/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2018 Pawel Kolodziejski <aquadran at users.sourceforge.net>
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

#ifndef STREAM_H
#define STREAM_H

#include "Helpers/ByteBuffer.h"

typedef enum
{
    Begin = 0,
    Current,
    End
} SeekOrigin;

class Stream
{
protected:

    bool readable{};
    bool writtable{};
    bool seekable{};

public:

    virtual ~Stream() = 0;

    virtual qint64 Length()  = 0;
    virtual qint64 Position() = 0;

    virtual void Flush() = 0;
    virtual void Close() = 0;

    virtual void CopyFrom(Stream &stream, qint64 count, qint64 bufferSize = 10000) = 0;
    virtual void ReadToBuffer(quint8 *buffer, qint64 count) = 0;
    virtual ByteBuffer ReadToBuffer(qint64 count) = 0;
    virtual void WriteFromBuffer(quint8 *buffer, qint64 count) = 0;
    virtual void WriteFromBuffer(ByteBuffer buffer) = 0;
    virtual void ReadStringASCII(QString &str, qint64 count) = 0;
    virtual void ReadStringASCIINull(QString &str) = 0;
    virtual void ReadStringUnicode16(QString &str, qint64 count) = 0;
    virtual void ReadStringUnicode16Null(QString &str) = 0;
    virtual void WriteStringASCII(const QString &str) = 0;
    virtual void WriteStringASCIINull(const QString &str) = 0;
    virtual void WriteStringUnicode16(const QString &str) = 0;
    virtual void WriteStringUnicode16Null(const QString &str) = 0;
    virtual qint64 ReadInt64() = 0;
    virtual quint64 ReadUInt64() = 0;
    virtual qint32 ReadInt32() = 0;
    virtual quint32 ReadUInt32() = 0;
    virtual qint16 ReadInt16() = 0;
    virtual quint16 ReadUInt16() = 0;
    virtual quint8 ReadByte() = 0;
    virtual void WriteInt64(qint64 value) = 0;
    virtual void WriteUInt64(quint64 value) = 0;
    virtual void WriteInt32(qint32 value) = 0;
    virtual void WriteUInt32(quint32 value) = 0;
    virtual void WriteInt16(qint16 value) = 0;
    virtual void WriteUInt16(quint16 value) = 0;
    virtual void WriteByte(quint8 value) = 0;
    virtual void WriteZeros(qint64 count) = 0;
    virtual void Seek(qint64 offset, SeekOrigin origin) = 0;
    virtual void SeekBegin() = 0;
    virtual void SeekEnd() = 0;
    virtual void JumpTo(qint64 offset) = 0;
    virtual void Skip(qint64 offset) = 0;
    virtual void SkipByte() = 0;
    virtual void SkipInt16() = 0;
    virtual void SkipInt32() = 0;
    virtual void SkipInt64() = 0;
};

#endif
