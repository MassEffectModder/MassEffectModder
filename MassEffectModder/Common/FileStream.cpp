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

#include "FileStream.h"

void FileStream::CheckFileIOErrorStatus()
{
    if (file->error() != QFileDevice::NoError)
    {
        auto error = (QString("Error: ") + file->errorString() + ", File: " + file->fileName()).toStdString();
        CRASH_MSG(error.c_str());
    }
}

FileStream::FileStream(const std::wstring &path, FileMode mode, FileAccess access)
    : file(nullptr)
{
    std::fstream::openmode openFlags{};
    file = new std::wfstream;

    switch (access)
    {
    case FileAccess::ReadOnly:
        openFlags |= std::wfstream::in;
        break;
    case FileAccess::WriteOnly:
        if (mode == FileMode::Create)
            openFlags |= std::wfstream::out | std::wfstream::trunc;
        else
            openFlags |= std::wfstream::out;
        break;
    case FileAccess::ReadWrite:
        if (mode == FileMode::Create)
            openFlags |= std::wfstream::in | std::wfstream::out | std::wfstream::trunc;
        else
            openFlags |= std::wfstream::in | std::wfstream::out;
        break;
    }

    file->open(path, openFlags);

    if (!file->is_open())
    {
        auto error = std::string("Error: Failed to open file: " + path + "\n";
        CRASH_MSG(error.c_str());
    }
}

FileStream::~FileStream()
{
    Close();
    delete file;
}

int64_t FileStream::Length()
{
    return file->size();
}

int64_t FileStream::Position()
{
    return file->pos();
}

void FileStream::Flush()
{
    file->flush();
}

void FileStream::Close()
{
    file->close();
}

void FileStream::CopyFrom(Stream &stream, int64_t count, int64_t bufferSize)
{
    if (count == 0)
        return;

    if (count < 0)
        CRASH();

    std::unique_ptr<uint8_t[]> buffer (new uint8_t[static_cast<unsigned long>(bufferSize)]);
    do
    {
        int64_t size = qMin(bufferSize, count);
        stream.ReadToBuffer(buffer.get(), size);
        WriteFromBuffer(buffer.get(), size);
        count -= size;
    } while (count != 0);
}

void FileStream::ReadToBuffer(uint8 *buffer, int64_t count)
{
    file->read(reinterpret_cast<char *>(buffer), count);
    CheckFileIOErrorStatus();
}

ByteBuffer FileStream::ReadToBuffer(int64_t count)
{
    ByteBuffer buffer(count);
    ReadToBuffer(buffer.ptr(), count);
    return buffer;
}

ByteBuffer FileStream::ReadAllToBuffer()
{
    SeekBegin();
    return ReadToBuffer(file->size());
}

void FileStream::WriteFromBuffer(uint8_t *buffer, int64_t count)
{
    file->write(reinterpret_cast<char *>(buffer), count);
    CheckFileIOErrorStatus();
}

void FileStream::WriteFromBuffer(const ByteBuffer &buffer)
{
    WriteFromBuffer(buffer.ptr(), buffer.size());
}

void FileStream::ReadStringASCII(QString &str, int64_t count)
{
    std::unique_ptr<char[]> buffer (new char[static_cast<size_t>(count) + 1]);

    buffer.get()[count] = 0;
    file->read(buffer.get(), count);
    CheckFileIOErrorStatus();
    str = QString(buffer.get());
}

void FileStream::ReadStringASCIINull(QString &str)
{
    str = "";
    do
    {
        char c = 0;
        file->getChar(&c);
        CheckFileIOErrorStatus();
        if (c == 0)
            return;
        str += c;
    } while (true);
}

void FileStream::ReadStringUnicode16(QString &str, qint64_t count)
{
    str = "";
    for (qint64_t n = 0; n < count; n++)
    {
        uint16_t c = ReadUInt16();
        CheckFileIOErrorStatus();
        str += QChar(static_cast<ushort>(c));
    }
}

void FileStream::ReadStringUnicode16Null(QString &str)
{
    str = "";
    do
    {
        uint16_t c = ReadUInt16();
        CheckFileIOErrorStatus();
        if (c == 0)
            return;
        str += QChar(static_cast<ushort>(c));
    } while (true);
}

void FileStream::WriteStringASCII(const QString &str)
{
    std::string string = str.toStdString();
    const char *s = string.c_str();
    file->write(s, string.length());
    CheckFileIOErrorStatus();
}

void FileStream::WriteStringASCIINull(const QString &str)
{
    WriteStringASCII(str);
    WriteByte(0);
}

void FileStream::WriteStringUnicode16(const QString &str)
{
    auto *s = const_cast<ushort *>(str.utf16());
    file->write(reinterpret_cast<char *>(s), str.length() * 2);
    CheckFileIOErrorStatus();
}

void FileStream::WriteStringUnicode16Null(const QString &str)
{
    WriteStringUnicode16(str);
    WriteUInt16(0);
}

int64_t FileStream::ReadInt64()
{
    int64_t value;
    file->read(reinterpret_cast<char *>(&value), sizeof(int64_t));
    CheckFileIOErrorStatus();
    return value;
}

uint64_t FileStream::ReadUInt64()
{
    uint64_t value;
    file->read(reinterpret_cast<char *>(&value), sizeof(uint64_t));
    CheckFileIOErrorStatus();
    return value;
}

int32_t FileStream::ReadInt32()
{
    int32_t value;
    file->read(reinterpret_cast<char *>(&value), sizeof(int32_t));
    CheckFileIOErrorStatus();
    return value;
}

uint32_t FileStream::ReadUInt32()
{
    uint32_t value;
    file->read(reinterpret_cast<char *>(&value), sizeof(uint32_t));
    CheckFileIOErrorStatus();
    return value;
}

int16_t FileStream::ReadInt16()
{
    int16_t value;
    file->read(reinterpret_cast<char *>(&value), sizeof(int16_t));
    CheckFileIOErrorStatus();
    return value;
}

uint16_t FileStream::ReadUInt16()
{
    uint16_t value;
    file->read(reinterpret_cast<char *>(&value), sizeof(uint16_t));
    CheckFileIOErrorStatus();
    return value;
}

uint8_t FileStream::ReadByte()
{
    uint8_t value;
    file->read(reinterpret_cast<char *>(&value), sizeof(uint8_t));
    CheckFileIOErrorStatus();
    return value;
}

void FileStream::WriteInt64(int64_t value)
{
    file->write(reinterpret_cast<char *>(&value), sizeof(int64_t));
    CheckFileIOErrorStatus();
}

void FileStream::WriteUInt64(uint64_t value)
{
    file->write(reinterpret_cast<char *>(&value), sizeof(uint64_t));
    CheckFileIOErrorStatus();
}

void FileStream::WriteInt32(int32_t value)
{
    file->write(reinterpret_cast<char *>(&value), sizeof(int32_t));
    CheckFileIOErrorStatus();
}

void FileStream::WriteUInt32(uint32_t value)
{
    file->write(reinterpret_cast<char *>(&value), sizeof(uint32_t));
    CheckFileIOErrorStatus();
}

void FileStream::WriteInt16(int16_t value)
{
    file->write(reinterpret_cast<char *>(&value), sizeof(int16_t));
    CheckFileIOErrorStatus();
}

void FileStream::WriteUInt16(uint16_t value)
{
    file->write(reinterpret_cast<char *>(&value), sizeof(int16_t));
    CheckFileIOErrorStatus();
}

void FileStream::WriteByte(uint8_t value)
{
    file->write(reinterpret_cast<char *>(&value), sizeof(uint8_t));
    CheckFileIOErrorStatus();
}

void FileStream::WriteZeros(int64_t count)
{
    uint8_t z = 0;

    for (int64_t l = 0; l < count; l++)
    {
         file->write(reinterpret_cast<char *>(&z), sizeof(uint8_t));
         CheckFileIOErrorStatus();
    }
}

void FileStream::Seek(int64_t offset, SeekOrigin origin)
{
    switch (origin)
    {
    case SeekOrigin::Begin:
        file->seek(offset);
        CheckFileIOErrorStatus();
        break;
    case SeekOrigin::Current:
        file->seek(file->pos() + offset);
        CheckFileIOErrorStatus();
        break;
    case SeekOrigin::End:
        file->seek(file->size() + offset);
        CheckFileIOErrorStatus();
        break;
    }
}

void FileStream::SeekBegin()
{
    Seek(0, SeekOrigin::Begin);
}

void FileStream::SeekEnd()
{
    Seek(0, SeekOrigin::End);
}

void FileStream::JumpTo(int64_t offset)
{
    Seek(offset, SeekOrigin::Begin);
}

void FileStream::Skip(int64_t offset)
{
    Seek(offset, SeekOrigin::Current);
}

void FileStream::SkipByte()
{
    Seek(sizeof(uint8_t), SeekOrigin::Current);
}

void FileStream::SkipInt16()
{
    Seek(sizeof(uint16_t), SeekOrigin::Current);
}

void FileStream::SkipInt32()
{
    Seek(sizeof(int32_t), SeekOrigin::Current);
}

void FileStream::SkipInt64()
{
    Seek(sizeof(uint64_t), SeekOrigin::Current);
}
