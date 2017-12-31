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

#include <QString>
#include <QFile>

#include <Helpers/MemoryStream.h>

MemoryStream::MemoryStream()
{
    internalBuffer = (quint8 *)malloc(bufferMargin);
    if (internalBuffer == nullptr)
    {
        CRASH_MSG("MemoryStream: out of memory");
        return;
    }
    internalBufferSize = bufferMargin;
    length = 0;
    position = 0;
}

MemoryStream::~MemoryStream()
{
    free(internalBuffer);
}

void MemoryStream::CopyFrom(Stream *stream, qint64 count, qint64 bufferSize)
{
    quint8 *buffer = new quint8[bufferSize];
    do
    {
        qint64 size = qMin(bufferSize, count);
        stream->ReadToBuffer(buffer, size);
        WriteFromBuffer(buffer, size);
        count -= size;
    } while (count != 0);

    delete[] buffer;
}

void MemoryStream::ReadToBuffer(quint8 *buffer, qint64 count)
{
    if (position + count > length)
    {
        CRASH_MSG("MemoryStream::ReadToBuffer() - Error: read out of buffer");
        return;
    }

    memcpy(buffer, internalBuffer + position, count);
    position += count;
}

void MemoryStream::WriteFromBuffer(quint8 *buffer, qint64 count)
{
    if (position + count + bufferMargin > length)
    {
        qint64 newOffset = position + count;
        buffer = (quint8 *)realloc(internalBuffer, newOffset + bufferMargin);
        if (buffer == nullptr)
        {
            CRASH_MSG("MemoryStream: out of memory");
            return;
        }
        internalBufferSize = newOffset + bufferMargin;
        memset(internalBuffer + length, 0, (newOffset + bufferMargin) - length);
        position = newOffset;
        length = position + 1;
    }
    memcpy(internalBuffer + position, buffer, count);
    position += count;
}

void MemoryStream::ReadStringASCII(QString &str, qint64 count)
{
    char *buffer = new char[count + 1];

    buffer[count] = 0;
    ReadToBuffer((quint8 *)buffer, count);
    str = QString(buffer);
    delete[] buffer;
}

void MemoryStream::ReadStringASCIINull(QString &str)
{
    do
    {
        char c = ReadByte();
        if (c == 0)
            return;
        str += c;
    } while (position < length);
}

void MemoryStream::ReadStringUnicode16(QString &str, qint64 count)
{
    count *= 2;
    char *buffer = new char[count + 2];

    buffer[count] = 0;
    buffer[count + 1] = 0;
    ReadToBuffer((quint8 *)buffer, count);
    str = QString(buffer);
    delete[] buffer;
}

void MemoryStream::ReadStringUnicode16Null(QString &str)
{
    do
    {
        quint16 c = ReadUInt16();
        if (c == 0)
            return;
        str += QChar((ushort)c);
    } while (position < length);
}

void MemoryStream::WriteStringASCII(QString &str)
{
    const char *s = str.toStdString().c_str();
    WriteFromBuffer((quint8 *)s, str.length());
}

void MemoryStream::WriteStringASCIINull(QString &str)
{
    WriteStringASCII(str);
    WriteByte(0);
}

void MemoryStream::WriteStringUnicode16(QString &str)
{
    const ushort *s = str.utf16();
    WriteFromBuffer((quint8 *)s, str.length() * 2);
}

void MemoryStream::WriteStringUnicode16Null(QString &str)
{
    WriteStringUnicode16(str);
    WriteUInt16(0);
}

qint64 MemoryStream::ReadInt64()
{
    qint64 value;
    ReadToBuffer((quint8 *)&value, sizeof(qint64));
    return value;
}

quint64 MemoryStream::ReadUInt64()
{
    quint64 value;
    ReadToBuffer((quint8 *)&value, sizeof(quint64));
    return value;
}

qint32 MemoryStream::ReadInt32()
{
    quint64 value;
    ReadToBuffer((quint8 *)&value, sizeof(qint32));
    return value;
}

quint32 MemoryStream::ReadUInt32()
{
    quint32 value;
    ReadToBuffer((quint8 *)&value, sizeof(quint32));
    return value;
}

qint16 MemoryStream::ReadInt16()
{
    qint16 value;
    ReadToBuffer((quint8 *)&value, sizeof(qint16));
    return value;
}

quint16 MemoryStream::ReadUInt16()
{
    quint16 value;
    ReadToBuffer((quint8 *)&value, sizeof(quint16));
    return value;
}

quint8 MemoryStream::ReadByte()
{
    quint8 value;
    ReadToBuffer((quint8 *)&value, sizeof(quint8));
    return value;
}

void MemoryStream::WriteInt64(qint64 value)
{
    WriteFromBuffer((quint8 *)&value, sizeof(qint64));
}

void MemoryStream::WriteUInt64(quint64 value)
{
    WriteFromBuffer((quint8 *)&value, sizeof(quint64));
}

void MemoryStream::WriteInt32(qint32 value)
{
    WriteFromBuffer((quint8 *)&value, sizeof(qint32));
}

void MemoryStream::WriteUInt32(quint32 value)
{
    WriteFromBuffer((quint8 *)&value, sizeof(quint32));
}

void MemoryStream::WriteInt16(qint16 value)
{
    WriteFromBuffer((quint8 *)&value, sizeof(qint16));
}

void MemoryStream::WriteUInt16(quint16 value)
{
    WriteFromBuffer((quint8 *)&value, sizeof(quint16));
}

void MemoryStream::WriteByte(quint8 value)
{
    WriteFromBuffer((quint8 *)&value, sizeof(quint8));
}

void MemoryStream::WriteZeros(qint64 count)
{
    for (qint64 l = 0; l < count; l++)
         WriteByte(0);
}

void MemoryStream::Seek(qint64 offset, SeekOrigin origin)
{
    switch (origin)
    {
    case SeekOrigin::Begin:
        if (offset < 0)
        {
            CRASH_MSG("MemoryStream: out of stream");
            return;
        }
        else if (offset > length + internalBufferSize)
        {
            internalBuffer = (quint8 *)realloc(internalBuffer, offset + bufferMargin);
            if (internalBuffer == nullptr)
            {
                CRASH_MSG("MemoryStream: out of memory");
                return;
            }
            internalBufferSize = offset + bufferMargin;
            memset(internalBuffer + length, 0, (offset + bufferMargin) - length);
            length = offset + 1;
        }
        position = offset;
        break;
    case SeekOrigin::Current:
        if (position + offset < 0)
        {
            CRASH_MSG("MemoryStream: out of stream");
            return;
        }
        else if (position + offset > length + internalBufferSize)
        {
            qint64 newOffset = position + offset;
            internalBuffer = (quint8 *)realloc(internalBuffer, newOffset + bufferMargin);
            if (internalBuffer == nullptr)
            {
                CRASH_MSG("MemoryStream: out of memory");
                return;
            }
            internalBufferSize = newOffset + bufferMargin;
            memset(internalBuffer + length, 0, (newOffset + bufferMargin) - length);
            position = newOffset;
            length = position + 1;
        }
        else
            position += offset;
        break;
    case SeekOrigin::End:
        if (length + offset < 0)
        {
            CRASH_MSG("MemoryStream: out of stream");
            return;
        }
        else if (position + offset > length + bufferMargin)
        {
            qint64 newOffset = position + offset;
            internalBuffer = (quint8 *)realloc(internalBuffer, newOffset + bufferMargin);
            if (internalBuffer == nullptr)
            {
                CRASH_MSG("MemoryStream: out of memory");
                return;
            }
            internalBufferSize = newOffset + bufferMargin;
            memset(internalBuffer + length, 0, (newOffset + bufferMargin) - length);
            position = newOffset;
            length = position + 1;
        }
        else
            position = length + offset;
        break;
    }
}

void MemoryStream::JumpTo(qint64 offset)
{
    Seek(offset, SeekOrigin::Begin);
}

void MemoryStream::Skip(qint64 offset)
{
    Seek(offset, SeekOrigin::Current);
}

void MemoryStream::SkipByte()
{
    Seek(sizeof(quint8), SeekOrigin::Current);
}

void MemoryStream::SkipInt16()
{
    Seek(sizeof(quint16), SeekOrigin::Current);
}

void MemoryStream::SkipInt32()
{
    Seek(sizeof(qint32), SeekOrigin::Current);
}

void MemoryStream::SkipInt64()
{
    Seek(sizeof(quint64), SeekOrigin::Current);
}
