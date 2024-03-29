/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2022 Pawel Kolodziejski
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

#include "MemoryStream.h"

MemoryStream::MemoryStream()
{
    internalBuffer = static_cast<quint8 *>(std::malloc(static_cast<size_t>(initialBufferSize)));
    if (internalBuffer == nullptr)
    {
        CRASH_MSG("MemoryStream: out of memory.");
    }
    memset(internalBuffer, 0, initialBufferSize);
    internalBufferSize = initialBufferSize;
    length = 0;
    position = 0;
}

MemoryStream::MemoryStream(const ByteBuffer &buffer)
{
    internalBuffer = static_cast<quint8 *>(std::malloc(static_cast<size_t>(buffer.size())));
    if (internalBuffer == nullptr)
    {
        CRASH_MSG("MemoryStream: out of memory.");
    }
    internalBufferSize = length = buffer.size();
    memcpy(internalBuffer, buffer.ptr(), length);
    position = 0;
}

MemoryStream::MemoryStream(const ByteBuffer &buffer, qint64 offset)
{
    internalBuffer = static_cast<quint8 *>(std::malloc(static_cast<size_t>(buffer.size())));
    if (internalBuffer == nullptr)
    {
        CRASH_MSG("MemoryStream: out of memory.");
    }
    internalBufferSize = length = buffer.size();
    memcpy(internalBuffer, buffer.ptr() + offset, length);
    position = 0;
}

MemoryStream::MemoryStream(const ByteBuffer &buffer, qint64 offset, qint64 count)
{
    internalBuffer = static_cast<quint8 *>(std::malloc(static_cast<size_t>(count)));
    if (internalBuffer == nullptr )
    {
        CRASH_MSG("MemoryStream: out of memory.");
    }
    if ((offset + count) > buffer.size())
    {
        CRASH_MSG("MemoryStream: out of range.");
    }
    internalBufferSize = length = count;
    memcpy(internalBuffer, buffer.ptr() + offset, length);
    position = 0;
}

MemoryStream::MemoryStream(QString &filename, qint64 offset, qint64 count)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        auto error = (QString("Error: ") + file.errorString() + "\nFailed to open file: " + filename + "\n").toStdString();
        CRASH_MSG(error.c_str());
    }

    internalBuffer = static_cast<quint8 *>(std::malloc(static_cast<size_t>(count)));
    if (internalBuffer == nullptr)
    {
        CRASH_MSG("MemoryStream: out of memory.");
    }
    if ((offset + count) > file.size())
    {
        CRASH_MSG("MemoryStream: out of range.");
    }

    file.seek(offset);
    file.read(reinterpret_cast<char *>(internalBuffer), count);

    internalBufferSize = length = count;
    position = 0;
}

MemoryStream::MemoryStream(QString &filename, qint64 count)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        auto error = (QString("Error: ") + file.errorString() + "\nFailed to open file: " + filename + "\n").toStdString();
        CRASH_MSG(error.c_str());
    }

    internalBuffer = static_cast<quint8 *>(std::malloc(static_cast<size_t>(count)));
    if (internalBuffer == nullptr )
    {
        CRASH_MSG("MemoryStream: out of memory.");
    }

    file.read(reinterpret_cast<char *>(internalBuffer), count);

    internalBufferSize = length = count;
    position = 0;
}

MemoryStream::MemoryStream(QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        auto error = (QString("Error: ") + file.errorString() + "\nFailed to open file: " + filename + "\n").toStdString();
        CRASH_MSG(error.c_str());
    }

    qint64 count = file.size();
    internalBuffer = static_cast<quint8 *>(std::malloc(static_cast<size_t>(count)));
    if (internalBuffer == nullptr )
    {
        CRASH_MSG("MemoryStream: out of memory.");
    }

    file.read(reinterpret_cast<char *>(internalBuffer), count);

    internalBufferSize = length = count;
    position = 0;
}

MemoryStream::~MemoryStream()
{
    std::free(internalBuffer);
}

ByteBuffer MemoryStream::ToArray()
{
    ByteBuffer buffer(length);
    memcpy(buffer.ptr(), internalBuffer, length);
    return buffer;
}

void MemoryStream::CopyFrom(Stream &stream, qint64 count, qint64 bufferSize)
{
    if (count == 0)
        return;

    if (count < 0)
        CRASH();

    std::unique_ptr<quint8[]> buffer (new quint8[static_cast<unsigned long>(bufferSize)]);
    do
    {
        qint64 size = qMin(bufferSize, count);
        stream.ReadToBuffer(buffer.get(), size);
        WriteFromBuffer(buffer.get(), size);
        count -= size;
    } while (count != 0);
}

void MemoryStream::ReadToBuffer(quint8 *buffer, qint64 count)
{
    if (position + count > length)
    {
        CRASH_MSG("MemoryStream::ReadToBuffer() - Error: read out of buffer.");
    }

    memcpy(buffer, internalBuffer + position, static_cast<size_t>(count));
    position += count;
}

ByteBuffer MemoryStream::ReadToBuffer(qint64 count)
{
    ByteBuffer buffer(count);
    ReadToBuffer(buffer.ptr(), count);
    return buffer;
}

void MemoryStream::WriteFromBuffer(quint8 *buffer, qint64 count)
{
    qint64 newPosition = position + count;
    if (newPosition > internalBufferSize)
    {
        internalBufferSize = newPosition + internalBufferSize * 2;
        internalBuffer = static_cast<quint8 *>(std::realloc(internalBuffer, static_cast<size_t>(internalBufferSize)));
        if (internalBuffer == nullptr)
        {
            CRASH_MSG("MemoryStream: out of memory.");
        }
        memset(internalBuffer + length, 0, static_cast<size_t>(internalBufferSize - length));
    }
    if (newPosition > length)
        length = newPosition;
    memcpy(internalBuffer + position, buffer, static_cast<size_t>(count));
    position += count;
}

void MemoryStream::WriteFromBuffer(const ByteBuffer &buffer)
{
    WriteFromBuffer(buffer.ptr(), buffer.size());
}

void MemoryStream::ReadStringASCII(QString &str, qint64 count)
{
    std::unique_ptr<char[]> buffer (new char[static_cast<size_t>(count) + 1]);

    buffer.get()[count] = 0;
    ReadToBuffer(reinterpret_cast<quint8 *>(buffer.get()), count);
    str = QString(buffer.get());
}

void MemoryStream::ReadStringASCIINull(QString &str)
{
    str = "";
    do
    {
        auto c = static_cast<char>(ReadByte());
        if (c == 0)
            return;
        str += c;
    } while (position < length);
}

void MemoryStream::ReadStringUnicode16(QString &str, qint64 count)
{
    str = "";
    for (qint64 n = 0; n < count; n++)
    {
        quint16 c = ReadUInt16();
        str += QChar(static_cast<ushort>(c));
    }
}

void MemoryStream::ReadStringUnicode16Null(QString &str)
{
    str = "";
    do
    {
        quint16 c = ReadUInt16();
        if (c == 0)
            return;
        str += QChar(static_cast<ushort>(c));
    } while (position < length);
}

void MemoryStream::WriteStringASCII(const QString &str)
{
    std::string string = str.toStdString();
    auto s = const_cast<char *>(string.c_str());
    WriteFromBuffer(reinterpret_cast<quint8 *>(s), string.length());
}

void MemoryStream::WriteStringASCIINull(const QString &str)
{
    WriteStringASCII(str);
    WriteByte(0);
}

void MemoryStream::WriteStringUnicode16(const QString &str)
{
    auto *s = const_cast<ushort *>(str.utf16());
    WriteFromBuffer(reinterpret_cast<quint8 *>(s), str.length() * 2);
}

void MemoryStream::WriteStringUnicode16Null(const QString &str)
{
    WriteStringUnicode16(str);
    WriteUInt16(0);
}

qint64 MemoryStream::ReadInt64()
{
    qint64 value;
    ReadToBuffer(reinterpret_cast<quint8 *>(&value), sizeof(qint64));
    return value;
}

quint64 MemoryStream::ReadUInt64()
{
    quint64 value;
    ReadToBuffer(reinterpret_cast<quint8 *>(&value), sizeof(quint64));
    return value;
}

qint32 MemoryStream::ReadInt32()
{
    qint32 value;
    ReadToBuffer(reinterpret_cast<quint8 *>(&value), sizeof(qint32));
    return value;
}

quint32 MemoryStream::ReadUInt32()
{
    quint32 value;
    ReadToBuffer(reinterpret_cast<quint8 *>(&value), sizeof(quint32));
    return value;
}

qint16 MemoryStream::ReadInt16()
{
    qint16 value;
    ReadToBuffer(reinterpret_cast<quint8 *>(&value), sizeof(qint16));
    return value;
}

quint16 MemoryStream::ReadUInt16()
{
    quint16 value;
    ReadToBuffer(reinterpret_cast<quint8 *>(&value), sizeof(quint16));
    return value;
}

quint8 MemoryStream::ReadByte()
{
    quint8 value;
    ReadToBuffer(reinterpret_cast<quint8 *>(&value), sizeof(quint8));
    return value;
}

void MemoryStream::WriteInt64(qint64 value)
{
    WriteFromBuffer(reinterpret_cast<quint8 *>(&value), sizeof(qint64));
}

void MemoryStream::WriteUInt64(quint64 value)
{
    WriteFromBuffer(reinterpret_cast<quint8 *>(&value), sizeof(quint64));
}

void MemoryStream::WriteInt32(qint32 value)
{
    WriteFromBuffer(reinterpret_cast<quint8 *>(&value), sizeof(qint32));
}

void MemoryStream::WriteUInt32(quint32 value)
{
    WriteFromBuffer(reinterpret_cast<quint8 *>(&value), sizeof(quint32));
}

void MemoryStream::WriteInt16(qint16 value)
{
    WriteFromBuffer(reinterpret_cast<quint8 *>(&value), sizeof(qint16));
}

void MemoryStream::WriteUInt16(quint16 value)
{
    WriteFromBuffer(reinterpret_cast<quint8 *>(&value), sizeof(quint16));
}

void MemoryStream::WriteByte(quint8 value)
{
    WriteFromBuffer(reinterpret_cast<quint8 *>(&value), sizeof(quint8));
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
    {
        if (offset < 0)
        {
            CRASH_MSG("MemoryStream: out of stream.");
        }
        position = offset;
        break;
    }
    case SeekOrigin::Current:
    {
        qint64 newOffset = position + offset;
        if (newOffset < 0)
        {
            CRASH_MSG("MemoryStream: out of stream.");
        }
        position = newOffset;
        break;
    }
    case SeekOrigin::End:
    {
        qint64 newOffset = length + offset;
        if (newOffset < 0)
        {
            CRASH_MSG("MemoryStream: out of stream.");
        }
        position = newOffset;
        break;
    }
    }
}

void MemoryStream::SeekBegin()
{
    Seek(0, SeekOrigin::Begin);
}

void MemoryStream::SeekEnd()
{
    Seek(0, SeekOrigin::End);
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
