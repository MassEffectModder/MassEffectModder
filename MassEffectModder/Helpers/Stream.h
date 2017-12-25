/*
 * MassEffectModder
 *
 * Copyright (C) 2017 Pawel Kolodziejski <aquadran at users.sourceforge.net>
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

#include <QtGlobal>

class QString;

typedef enum
{
    Begin = 0,
    Current,
    End
} SeekOrigin;

class Stream
{
protected:

    qint64 length;
    qint64 position;
    bool readable;
    bool writtable;
    bool seekable;

    virtual ~Stream() {}

public:

    qint64 Length() { return length; }
    qint64 Position() { return position; }

    virtual void Flush() = 0;
    virtual void Close() = 0;

    virtual bool CopyFrom(Stream *stream, qint64 count, qint64 bufferSize = 10000) = 0;
    virtual qint64 ReadToBuffer(quint8 *buffer, qint64 count) = 0;
    virtual qint64 WriteFromBuffer(quint8 *buffer, qint64 count) = 0;
    virtual bool ReadStringASCII(QString &str, qint64 count) = 0;
    virtual bool ReadStringASCIINull(QString &str) = 0;
    virtual bool ReadStringUnicode16(QString &str, qint64 count) = 0;
    virtual bool ReadStringUnicode16Null(QString &str) = 0;
    virtual bool WriteStringASCII(QString &str) = 0;
    virtual bool WriteStringASCIINull(QString &str) = 0;
    virtual bool WriteStringUnicode16(QString &str) = 0;
    virtual bool WriteStringUnicode16Null(QString &str) = 0;
    virtual qint64 ReadInt64() = 0;
    virtual quint64 ReadUInt64() = 0;
    virtual qint32 ReadInt32() = 0;
    virtual quint32 ReadUInt32() = 0;
    virtual qint16 ReadInt16() = 0;
    virtual quint16 ReadUInt16() = 0;
    virtual quint8 ReadByte() = 0;
    virtual bool WriteInt64(qint64 value) = 0;
    virtual bool WriteUInt64(uint64_t value) = 0;
    virtual bool WriteInt32(qint32 value) = 0;
    virtual bool WriteUInt32(quint32 value) = 0;
    virtual bool WriteInt16(qint16 value) = 0;
    virtual bool WriteUInt16(quint16 value) = 0;
    virtual bool WriteByte(quint8 value) = 0;
    virtual bool WriteZeros(qint64 count) = 0;
    virtual bool Seek(qint64 offset, SeekOrigin origin) = 0;
    virtual bool JumpTo(qint64 offset) = 0;
    virtual bool Skip(qint64 offset) = 0;
    virtual bool SkipByte() = 0;
    virtual bool SkipInt16() = 0;
    virtual bool SkipInt32() = 0;
    virtual bool SkipInt64() = 0;
};

#endif
