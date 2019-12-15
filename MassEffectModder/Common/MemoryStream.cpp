/* The MIT License
 *
 * Copyright (c) 2017-2019 Pawel Kolodziejski
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "MemoryStream.h"

MemoryStream::MemoryStream()
{
    internalBuffer = static_cast<uint8_t *>(std::malloc(static_cast<size_t>(initialBufferSize)));
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
    internalBuffer = static_cast<uint8_t *>(std::malloc(static_cast<size_t>(buffer.size())));
    if (internalBuffer == nullptr)
    {
        CRASH_MSG("MemoryStream: out of memory.");
    }
    internalBufferSize = length = buffer.size();
    memcpy(internalBuffer, buffer.ptr(), length);
    position = 0;
}

MemoryStream::MemoryStream(const ByteBuffer &buffer, int64 offset)
{
    internalBuffer = static_cast<uint8_t *>(std::malloc(static_cast<size_t>(buffer.size())));
    if (internalBuffer == nullptr)
    {
        CRASH_MSG("MemoryStream: out of memory.");
    }
    internalBufferSize = length = buffer.size();
    memcpy(internalBuffer, buffer.ptr() + offset, length);
    position = 0;
}

MemoryStream::MemoryStream(const ByteBuffer &buffer, int64 offset, int64 count)
{
    internalBuffer = static_cast<uint8_t *>(std::malloc(static_cast<size_t>(count)));
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

MemoryStream::MemoryStream(QString &filename, int64_t offset, int64_t count)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        auto error = (QString("Error: ") + file.errorString() + "\nFailed to open file: " + filename + "\n").toStdString();
        CRASH_MSG(error.c_str());
    }

    internalBuffer = static_cast<uint8_t *>(std::malloc(static_cast<size_t>(count)));
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

MemoryStream::MemoryStream(QString &filename, int64_t count)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        auto error = (QString("Error: ") + file.errorString() + "\nFailed to open file: " + filename + "\n").toStdString();
        CRASH_MSG(error.c_str());
    }

    internalBuffer = static_cast<uint8_t *>(std::malloc(static_cast<size_t>(count)));
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

    int64 count = file.size();
    internalBuffer = static_cast<uint8_t *>(std::malloc(static_cast<size_t>(count)));
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

void MemoryStream::CopyFrom(Stream &stream, int64_t count, int64_t bufferSize)
{
    if (count == 0)
        return;

    if (count < 0)
        CRASH();

    std::unique_ptr<quint8_t[]> buffer (new quint8_t[static_cast<unsigned long>(bufferSize)]);
    do
    {
        int64_t size = MIN(bufferSize, count);
        stream.ReadToBuffer(buffer.get(), size);
        WriteFromBuffer(buffer.get(), size);
        count -= size;
    } while (count != 0);
}

void MemoryStream::ReadToBuffer(uint8_t *buffer, int64_t count)
{
    if (position + count > length)
    {
        CRASH_MSG("MemoryStream::ReadToBuffer() - Error: read out of buffer.");
    }

    memcpy(buffer, internalBuffer + position, static_cast<size_t>(count));
    position += count;
}

ByteBuffer MemoryStream::ReadToBuffer(int64_t count)
{
    ByteBuffer buffer(count);
    ReadToBuffer(buffer.ptr(), count);
    return buffer;
}

void MemoryStream::WriteFromBuffer(uint8_t *buffer, int64_t count)
{
    qint64 newPosition = position + count;
    if (newPosition > internalBufferSize)
    {
        internalBufferSize = newPosition + internalBufferSize * 2;
        internalBuffer = static_cast<uint8_t *>(std::realloc(internalBuffer, static_cast<size_t>(internalBufferSize)));
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

void MemoryStream::ReadStringASCII(QString &str, qint64_t count)
{
    std::unique_ptr<char[]> buffer (new char[static_cast<size_t>(count) + 1]);

    buffer.get()[count] = 0;
    ReadToBuffer(reinterpret_cast<uint8_t *>(buffer.get()), count);
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

void MemoryStream::ReadStringUnicode16(QString &str, int64_t count)
{
    str = "";
    for (int64_t n = 0; n < count; n++)
    {
        uint16_t c = ReadUInt16();
        str += QChar(static_cast<ushort>(c));
    }
}

void MemoryStream::ReadStringUnicode16Null(QString &str)
{
    str = "";
    do
    {
        uint16_t c = ReadUInt16();
        if (c == 0)
            return;
        str += QChar(static_cast<ushort>(c));
    } while (position < length);
}

void MemoryStream::WriteStringASCII(const QString &str)
{
    std::string string = str.toStdString();
    auto s = const_cast<char *>(string.c_str());
    WriteFromBuffer(reinterpret_cast<uint8_t *>(s), string.length());
}

void MemoryStream::WriteStringASCIINull(const QString &str)
{
    WriteStringASCII(str);
    WriteByte(0);
}

void MemoryStream::WriteStringUnicode16(const QString &str)
{
    auto *s = const_cast<ushort *>(str.utf16());
    WriteFromBuffer(reinterpret_cast<uint8_t *>(s), str.length() * 2);
}

void MemoryStream::WriteStringUnicode16Null(const QString &str)
{
    WriteStringUnicode16(str);
    WriteUInt16(0);
}

int64_t MemoryStream::ReadInt64()
{
    int64_t value;
    ReadToBuffer(reinterpret_cast<uint8_t *>(&value), sizeof(int64_t));
    return value;
}

uint64_t MemoryStream::ReadUInt64()
{
    uint64_t value;
    ReadToBuffer(reinterpret_cast<uint8_t *>(&value), sizeof(uint64_t));
    return value;
}

int32_t MemoryStream::ReadInt32()
{
    int32_t value;
    ReadToBuffer(reinterpret_cast<uint8_t *>(&value), sizeof(int32_t));
    return value;
}

uint32_t MemoryStream::ReadUInt32()
{
    uint32_t value;
    ReadToBuffer(reinterpret_cast<uint8_t *>(&value), sizeof(uint32_t));
    return value;
}

int16_t MemoryStream::ReadInt16()
{
    int16_t value;
    ReadToBuffer(reinterpret_cast<uint8_t *>(&value), sizeof(int16_t));
    return value;
}

uint16_t MemoryStream::ReadUInt16()
{
    uint16_t value;
    ReadToBuffer(reinterpret_cast<uint8_t *>(&value), sizeof(uint16_t));
    return value;
}

uint8_t MemoryStream::ReadByte()
{
    uint8_t value;
    ReadToBuffer(reinterpret_cast<uint8_t *>(&value), sizeof(uint8_t));
    return value;
}

void MemoryStream::WriteInt64(int64_t value)
{
    WriteFromBuffer(reinterpret_cast<uint8_t *>(&value), sizeof(int64_t));
}

void MemoryStream::WriteUInt64(uint64_t value)
{
    WriteFromBuffer(reinterpret_cast<uint8_t *>(&value), sizeof(uint64_t));
}

void MemoryStream::WriteInt32(int32_t value)
{
    WriteFromBuffer(reinterpret_cast<uint8_t *>(&value), sizeof(int32_t));
}

void MemoryStream::WriteUInt32(uint32_t value)
{
    WriteFromBuffer(reinterpret_cast<uint8_t *>(&value), sizeof(uint32_t));
}

void MemoryStream::WriteInt16(int16 value)
{
    WriteFromBuffer(reinterpret_cast<uint8_t *>(&value), sizeof(int16_t));
}

void MemoryStream::WriteUInt16(uint16_t value)
{
    WriteFromBuffer(reinterpret_cast<uint8_t *>(&value), sizeof(uint16_t));
}

void MemoryStream::WriteByte(uint8_t value)
{
    WriteFromBuffer(reinterpret_cast<uint8_t *>(&value), sizeof(uint8_t));
}

void MemoryStream::WriteZeros(int64_t count)
{
    for (int64_t l = 0; l < count; l++)
         WriteByte(0);
}

void MemoryStream::Seek(int64_t offset, SeekOrigin origin)
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
        int64_t newOffset = position + offset;
        if (newOffset < 0)
        {
            CRASH_MSG("MemoryStream: out of stream.");
        }
        position = newOffset;
        break;
    }
    case SeekOrigin::End:
    {
        int64_t newOffset = length + offset;
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

void MemoryStream::JumpTo(int64_t offset)
{
    Seek(offset, SeekOrigin::Begin);
}

void MemoryStream::Skip(int64_t offset)
{
    Seek(offset, SeekOrigin::Current);
}

void MemoryStream::SkipByte()
{
    Seek(sizeof(uint8_t), SeekOrigin::Current);
}

void MemoryStream::SkipInt16()
{
    Seek(sizeof(uint16_t), SeekOrigin::Current);
}

void MemoryStream::SkipInt32()
{
    Seek(sizeof(int32_t), SeekOrigin::Current);
}

void MemoryStream::SkipInt64()
{
    Seek(sizeof(uint64_t), SeekOrigin::Current);
}
