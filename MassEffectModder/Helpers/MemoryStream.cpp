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
    internalBufferSize = bufferMargin;
    length = 0;
    position = 0;
}

MemoryStream::~MemoryStream()
{
    free(internalBuffer);
}

bool MemoryStream::CopyFrom(Stream *stream, qint64 count, qint64 bufferSize)
{
    qint64 status;
    quint8 *buffer = new quint8[bufferSize];
    do
    {
        status = stream->ReadToBuffer(buffer, qMin(bufferSize, count));
        if (status == 0)
            break;
        status = WriteFromBuffer(buffer, count);
        count -= status;
    } while (count != 0);

    delete[] buffer;

    return true;
}

qint64 MemoryStream::ReadToBuffer(quint8 *buffer, qint64 count)
{
    if (position + count > length)
    {
        CRASH_MSG("MemoryStream::ReadToBuffer() - Error: read out of buffer");
        return 0;
    }

    memcpy(buffer, internalBuffer + position, count);
    position += count;

    return count;
}

qint64 MemoryStream::WriteFromBuffer(quint8 *buffer, qint64 count)
{
    if (position + count + bufferMargin > length)
    {
        qint64 newOffset = position + count;
        buffer = (quint8 *)realloc(internalBuffer, newOffset + bufferMargin);
        if (buffer == nullptr)
        {
            CRASH_MSG("MemoryStream: out of memory");
            return 0;
        }
        internalBufferSize = newOffset + bufferMargin;
        memset(internalBuffer + length, 0, (newOffset + bufferMargin) - length);
        position = newOffset;
        length = position + 1;
    }
    memcpy(internalBuffer + position, buffer, count);
    position += count;

    return count;
}

bool MemoryStream::ReadStringASCII(QString &string, qint64 count)
{
    char *buffer = new char[count + 1];

    buffer[count] = 0;

    qint64 readed = ReadToBuffer((quint8 *)buffer, count);
    if (readed == count)
        string = QString(buffer);

    delete[] buffer;

    if (readed == count)
        return true;
    else
        return false;
}

bool MemoryStream::ReadStringASCIINull(QString &str)
{
    do
    {
        char c = ReadByte();
        if (c == 0)
            return true;
        str += c;
    } while (position < length);

    return false;
}

bool MemoryStream::ReadStringUnicode16(QString &str, qint64 count)
{
    count *= 2;
    char *buffer = new char[count + 2];

    buffer[count] = 0;
    buffer[count + 1] = 0;

    qint64 readed = ReadToBuffer((quint8 *)buffer, count);
    if (readed == count)
        str = QString(buffer);

    delete[] buffer;

    return true;
}

bool MemoryStream::ReadStringUnicode16Null(QString &str)
{
    do
    {
        quint16 c = ReadUInt16();
        if (c == 0)
            return true;
        str += QChar((ushort)c);
    } while (position < length);

    return false;
}

bool MemoryStream::WriteStringASCII(QString &str)
{
    const char *s = str.toStdString().c_str();
    if (WriteFromBuffer((quint8 *)s, str.length()) != str.length())
        return false;
    else
        return true;
}

bool MemoryStream::WriteStringASCIINull(QString &str)
{
    if (!WriteStringASCII(str))
        return false;
    if (!WriteByte(0))
        return false;
    return true;
}

bool MemoryStream::WriteStringUnicode16(QString &str)
{
    const ushort *s = str.utf16();
    if (WriteFromBuffer((quint8 *)s, str.length() * 2) != str.length() * 2)
        return false;
    else
        return true;
}

bool MemoryStream::WriteStringUnicode16Null(QString &str)
{
    if (!WriteStringUnicode16(str))
        return false;
    if (!WriteUInt16(0))
        return false;
    return true;
}

qint64 MemoryStream::ReadInt64()
{
    qint64 value;
    if (ReadToBuffer((quint8 *)&value, sizeof(qint64)) != sizeof(qint64))
        return 0;
    return value;
}

quint64 MemoryStream::ReadUInt64()
{
    quint64 value;
    if (ReadToBuffer((quint8 *)&value, sizeof(quint64)) != sizeof(quint64))
        return 0;
    return value;
}

qint32 MemoryStream::ReadInt32()
{
    quint64 value;
    if (ReadToBuffer((quint8 *)&value, sizeof(qint32)) != sizeof(qint32))
        return 0;
    return value;
}

quint32 MemoryStream::ReadUInt32()
{
    quint32 value;
    if (ReadToBuffer((quint8 *)&value, sizeof(quint32)) != sizeof(quint32))
        return 0;

    return value;
}

qint16 MemoryStream::ReadInt16()
{
    qint16 value;
    if (ReadToBuffer((quint8 *)&value, sizeof(qint16)) != sizeof(qint16))
        return 0;
    return value;
}

quint16 MemoryStream::ReadUInt16()
{
    quint16 value;
    if (ReadToBuffer((quint8 *)&value, sizeof(quint16)) != sizeof(quint16))
        return 0;
    return value;
}

quint8 MemoryStream::ReadByte()
{
    quint8 value;
    if (ReadToBuffer((quint8 *)&value, sizeof(quint8)) != sizeof(quint8))
        return 0;
    return value;
}

bool MemoryStream::WriteInt64(qint64 value)
{
    return WriteFromBuffer((quint8 *)&value, sizeof(qint64));
}

bool MemoryStream::WriteUInt64(quint64 value)
{
    return WriteFromBuffer((quint8 *)&value, sizeof(quint64));
}

bool MemoryStream::WriteInt32(qint32 value)
{
    return WriteFromBuffer((quint8 *)&value, sizeof(qint32));
}

bool MemoryStream::WriteUInt32(quint32 value)
{
    return WriteFromBuffer((quint8 *)&value, sizeof(quint32));
}

bool MemoryStream::WriteInt16(qint16 value)
{
    return WriteFromBuffer((quint8 *)&value, sizeof(qint16));
}

bool MemoryStream::WriteUInt16(quint16 value)
{
    return WriteFromBuffer((quint8 *)&value, sizeof(quint16));
}

bool MemoryStream::WriteByte(quint8 value)
{
    return WriteFromBuffer((quint8 *)&value, sizeof(quint8));
}

bool MemoryStream::WriteZeros(qint64 count)
{
    for (qint64 l = 0; l < count; l++)
         WriteByte(0);

    return true;
}

bool MemoryStream::Seek(qint64 offset, SeekOrigin origin)
{
    switch (origin)
    {
    case SeekOrigin::Begin:
        if (offset < 0)
            return false;
        if (offset > length + internalBufferSize)
        {
            internalBuffer = (quint8 *)realloc(internalBuffer, offset + bufferMargin);
            if (internalBuffer == nullptr)
            {
                CRASH_MSG("MemoryStream: out of memory");
                return false;
            }
            internalBufferSize = offset + bufferMargin;
            memset(internalBuffer + length, 0, (offset + bufferMargin) - length);
            length = offset + 1;
        }
        position = offset;
        break;
    case SeekOrigin::Current:
        if (position + offset < 0)
            return false;
        else if (position + offset > length + internalBufferSize)
        {
            qint64 newOffset = position + offset;
            internalBuffer = (quint8 *)realloc(internalBuffer, newOffset + bufferMargin);
            if (internalBuffer == nullptr)
            {
                CRASH_MSG("MemoryStream: out of memory");
                return false;
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
            return false;
        else if (position + offset > length + bufferMargin)
        {
            qint64 newOffset = position + offset;
            internalBuffer = (quint8 *)realloc(internalBuffer, newOffset + bufferMargin);
            if (internalBuffer == nullptr)
            {
                CRASH_MSG("MemoryStream: out of memory");
                return false;
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

    return true;
}

bool MemoryStream::JumpTo(qint64 offset)
{
    return Seek(offset, SeekOrigin::Begin);
}

bool MemoryStream::Skip(qint64 offset)
{
    return Seek(offset, SeekOrigin::Current);
}

bool MemoryStream::SkipByte()
{
    return Seek(sizeof(quint8), SeekOrigin::Current);
}

bool MemoryStream::SkipInt16()
{
    return Seek(sizeof(quint16), SeekOrigin::Current);
}

bool MemoryStream::SkipInt32()
{
    return Seek(sizeof(qint32), SeekOrigin::Current);
}

bool MemoryStream::SkipInt64()
{
    return Seek(sizeof(quint64), SeekOrigin::Current);
}
