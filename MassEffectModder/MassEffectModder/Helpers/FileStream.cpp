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

#include "Helpers/FileStream.h"

void FileStream::CheckFileIOErrorStatus()
{
    if (file->error() != QFileDevice::NoError)
    {
        CRASH_MSG((QString("File: ") + file->fileName() + " Error: " + file->errorString()).toStdString().c_str());
    }
}

FileStream::FileStream(const QString &path, FileMode mode, FileAccess access)
    : file(nullptr)
{
    QFile::OpenMode openFlags = QIODevice::NotOpen;
    file = new QFile(path);

    switch (access)
    {
    case FileAccess::ReadOnly:
        openFlags |= QIODevice::ReadOnly;
        break;
    case FileAccess::WriteOnly:
        if (mode == FileMode::Create)
            openFlags |= QIODevice::WriteOnly | QIODevice::Truncate;
        else
            openFlags |= QIODevice::WriteOnly;
        break;
    case FileAccess::ReadWrite:
        if (mode == FileMode::Create)
            openFlags |= QIODevice::ReadWrite | QIODevice::Truncate;
        else
            openFlags |= QIODevice::ReadWrite;
        break;
    }

    if (!file->open(openFlags))
        CRASH_MSG((QString("Failed to open file: ") + path + " Error: " + file->errorString()).toStdString().c_str());
}

FileStream::~FileStream()
{
    Close();
    delete file;
}

qint64 FileStream::Length()
{
    return file->size();
}

qint64 FileStream::Position()
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

void FileStream::CopyFrom(Stream *stream, qint64 count, qint64 bufferSize)
{
    if (count == 0)
        return;

    if (count < 0)
        CRASH();

    std::unique_ptr<quint8> buffer (new quint8[static_cast<unsigned long>(bufferSize)]);
    do
    {
        qint64 size = qMin(bufferSize, count);
        stream->ReadToBuffer(buffer.get(), size);
        WriteFromBuffer(buffer.get(), size);
        count -= size;
    } while (count != 0);
}

void FileStream::ReadToBuffer(quint8 *buffer, qint64 count)
{
    file->read(reinterpret_cast<char *>(buffer), count);
    CheckFileIOErrorStatus();
}

ByteBuffer FileStream::ReadToBuffer(qint64 count)
{
    ByteBuffer buffer = ByteBuffer(count);
    ReadToBuffer(buffer.ptr(), count);
    return buffer;
}

ByteBuffer FileStream::ReadToBuffer()
{
    qint64 count = file->size();
    ByteBuffer buffer = ByteBuffer(count);
    ReadToBuffer(buffer.ptr(), count);
    return buffer;
}

void FileStream::WriteFromBuffer(quint8 *buffer, qint64 count)
{
    file->write(reinterpret_cast<char *>(buffer), count);
    CheckFileIOErrorStatus();
}

void FileStream::WriteFromBuffer(ByteBuffer buffer)
{
    WriteFromBuffer(buffer.ptr(), buffer.size());
}

void FileStream::ReadStringASCII(QString &str, qint64 count)
{
    std::unique_ptr<char> buffer (new char[static_cast<size_t>(count) + 1]);

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

void FileStream::ReadStringUnicode16(QString &str, qint64 count)
{
    str = "";
    for (int n = 0; n < count; n++)
    {
        quint16 c = ReadUInt16();
        CheckFileIOErrorStatus();
        str += QChar(static_cast<ushort>(c));
    }
}

void FileStream::ReadStringUnicode16Null(QString &str)
{
    str = "";
    do
    {
        quint16 c = ReadUInt16();
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

qint64 FileStream::ReadInt64()
{
    qint64 value;
    file->read(reinterpret_cast<char *>(&value), sizeof(qint64));
    CheckFileIOErrorStatus();
    return value;
}

quint64 FileStream::ReadUInt64()
{
    quint64 value;
    file->read(reinterpret_cast<char *>(&value), sizeof(quint64));
    CheckFileIOErrorStatus();
    return value;
}

qint32 FileStream::ReadInt32()
{
    qint32 value;
    file->read(reinterpret_cast<char *>(&value), sizeof(qint32));
    CheckFileIOErrorStatus();
    return value;
}

quint32 FileStream::ReadUInt32()
{
    quint32 value;
    file->read(reinterpret_cast<char *>(&value), sizeof(quint32));
    CheckFileIOErrorStatus();
    return value;
}

qint16 FileStream::ReadInt16()
{
    qint16 value;
    file->read(reinterpret_cast<char *>(&value), sizeof(qint16));
    CheckFileIOErrorStatus();
    return value;
}

quint16 FileStream::ReadUInt16()
{
    quint16 value;
    file->read(reinterpret_cast<char *>(&value), sizeof(quint16));
    CheckFileIOErrorStatus();
    return value;
}

quint8 FileStream::ReadByte()
{
    quint8 value;
    file->read(reinterpret_cast<char *>(&value), sizeof(quint8));
    CheckFileIOErrorStatus();
    return value;
}

void FileStream::WriteInt64(qint64 value)
{
    file->write(reinterpret_cast<char *>(&value), sizeof(qint64));
    CheckFileIOErrorStatus();
}

void FileStream::WriteUInt64(quint64 value)
{
    file->write(reinterpret_cast<char *>(&value), sizeof(quint64));
    CheckFileIOErrorStatus();
}

void FileStream::WriteInt32(qint32 value)
{
    file->write(reinterpret_cast<char *>(&value), sizeof(qint32));
    CheckFileIOErrorStatus();
}

void FileStream::WriteUInt32(quint32 value)
{
    file->write(reinterpret_cast<char *>(&value), sizeof(quint32));
    CheckFileIOErrorStatus();
}

void FileStream::WriteInt16(qint16 value)
{
    file->write(reinterpret_cast<char *>(&value), sizeof(qint16));
    CheckFileIOErrorStatus();
}

void FileStream::WriteUInt16(quint16 value)
{
    file->write(reinterpret_cast<char *>(&value), sizeof(qint16));
    CheckFileIOErrorStatus();
}

void FileStream::WriteByte(quint8 value)
{
    file->write(reinterpret_cast<char *>(&value), sizeof(quint8));
    CheckFileIOErrorStatus();
}

void FileStream::WriteZeros(qint64 count)
{
    quint8 z = 0;

    for (qint64 l = 0; l < count; l++)
    {
         file->write(reinterpret_cast<char *>(&z), sizeof(quint8));
         CheckFileIOErrorStatus();
    }
}

void FileStream::Seek(qint64 offset, SeekOrigin origin)
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
        file->seek(file->size() - offset);
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

void FileStream::JumpTo(qint64 offset)
{
    Seek(offset, SeekOrigin::Begin);
}

void FileStream::Skip(qint64 offset)
{
    Seek(offset, SeekOrigin::Current);
}

void FileStream::SkipByte()
{
    Seek(sizeof(quint8), SeekOrigin::Current);
}

void FileStream::SkipInt16()
{
    Seek(sizeof(quint16), SeekOrigin::Current);
}

void FileStream::SkipInt32()
{
    Seek(sizeof(qint32), SeekOrigin::Current);
}

void FileStream::SkipInt64()
{
    Seek(sizeof(quint64), SeekOrigin::Current);
}
