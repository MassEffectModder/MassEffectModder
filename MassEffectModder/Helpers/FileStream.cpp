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

#include <Helpers/FileStream.h>

void FileStream::CheckFileIOErrorStatus()
{
    if (file->error() != QFileDevice::NoError)
    {
        CRASH_MSG((QString("File: ") + filePath + QString(" Error: ") + file->errorString()).toStdString().c_str());
    }
}

FileStream::FileStream(QString &path, FileMode mode, FileAccess access)
    : file(nullptr)
{
    QFile::OpenMode openFlags = 0;
    file = new QFile(path);
    filePath = path;

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
        CRASH_MSG((QString("Failed to open file: ") + path + QString(" Error: ") + file->errorString()).toStdString().c_str());
}

FileStream::~FileStream()
{
    Close();
    delete file;
}

void FileStream::Flush()
{
    file->flush();
}

void FileStream::Close()
{
    file->close();
}

bool FileStream::CopyFrom(Stream *stream, qint64 count, qint64 bufferSize)
{
    qint64 status;
    quint8 *buffer = new quint8[bufferSize];
    do
    {
        status = stream->ReadToBuffer(buffer, qMin(bufferSize, count));
        if (status == 00)
            break;
        count -= status;
        status = WriteFromBuffer(buffer, count);
    } while (count != 0);

    delete[] buffer;

    if (status == -1)
        return false;
    else
        return true;
}

qint64 FileStream::ReadToBuffer(quint8 *buffer, qint64 count)
{
    qint64 readed = file->read((char *)buffer, count);
    CheckFileIOErrorStatus();
    return readed;
}

qint64 FileStream::WriteFromBuffer(quint8 *buffer, qint64 count)
{
    qint64 written = file->write((char *)buffer, count);
    CheckFileIOErrorStatus();
    return written;
}

bool FileStream::ReadStringASCII(QString &string, qint64 count)
{
    char *buffer = new char[count + 1];

    buffer[count] = 0;

    qint64 readed = file->read(buffer, count);
    CheckFileIOErrorStatus();
    if (readed == count)
        string = QString(buffer);

    delete[] buffer;

    if (readed == count)
        return true;
    else
        return false;
}

bool FileStream::ReadStringASCIINull(QString &str)
{
    do
    {
        char c = 0;
        file->getChar(&c);
        CheckFileIOErrorStatus();
        if (c == 0)
            return true;
        str += c;
    } while (true);

    return false;
}

bool FileStream::ReadStringUnicode16(QString &str, qint64 count)
{
    count *= 2;
    char *buffer = new char[count + 2];

    buffer[count] = 0;
    buffer[count + 1] = 0;

    qint64 readed = file->read(buffer, count);
    CheckFileIOErrorStatus();
    if (readed == count)
        str = QString(buffer);

    delete[] buffer;

    return true;
}

bool FileStream::ReadStringUnicode16Null(QString &str)
{
    do
    {
        quint16 c = ReadUInt16();
        CheckFileIOErrorStatus();
        if (c == 0)
            return true;
        str += QChar((ushort)c);
    } while (true);

    return false;
}

bool FileStream::WriteStringASCII(QString &str)
{
    const char *s = str.toStdString().c_str();
    file->write(s, str.length());
    CheckFileIOErrorStatus();
    return true;
}

bool FileStream::WriteStringASCIINull(QString &str)
{
    WriteStringASCII(str);
    WriteByte(0);
    return true;
}

bool FileStream::WriteStringUnicode16(QString &str)
{
    const ushort *s = str.utf16();
    file->write((char *)s, str.length() * 2);
    CheckFileIOErrorStatus();
    return true;
}

bool FileStream::WriteStringUnicode16Null(QString &str)
{
    WriteStringUnicode16(str);
    WriteUInt16(0);
    return true;
}

qint64 FileStream::ReadInt64()
{
    qint64 value;
    file->read((char *)&value, sizeof(qint64));
    CheckFileIOErrorStatus();
    return value;
}

quint64 FileStream::ReadUInt64()
{
    quint64 value;
    file->read((char *)&value, sizeof(quint64));
    CheckFileIOErrorStatus();
    return value;
}

qint32 FileStream::ReadInt32()
{
    qint32 value;
    file->read((char *)&value, sizeof(qint32));
    CheckFileIOErrorStatus();
    return value;
}

quint32 FileStream::ReadUInt32()
{
    quint32 value;
    file->read((char *)&value, sizeof(quint32));
    CheckFileIOErrorStatus();
    return value;
}

qint16 FileStream::ReadInt16()
{
    qint16 value;
    file->read((char *)&value, sizeof(qint16));
    CheckFileIOErrorStatus();
    return value;
}

quint16 FileStream::ReadUInt16()
{
    quint16 value;
    file->read((char *)&value, sizeof(quint16));
    CheckFileIOErrorStatus();
    return value;
}

quint8 FileStream::ReadByte()
{
    quint8 value;
    file->read((char *)&value, sizeof(quint8));
    CheckFileIOErrorStatus();
    return value;
}

bool FileStream::WriteInt64(qint64 value)
{
    file->write((char *)&value, sizeof(qint64));
    CheckFileIOErrorStatus();
    return true;
}

bool FileStream::WriteUInt64(quint64 value)
{
    file->write((char *)&value, sizeof(quint64));
    CheckFileIOErrorStatus();
    return true;
}

bool FileStream::WriteInt32(qint32 value)
{
    file->write((char *)&value, sizeof(qint32));
    CheckFileIOErrorStatus();
    return true;
}

bool FileStream::WriteUInt32(quint32 value)
{
    file->write((char *)&value, sizeof(quint32));
    CheckFileIOErrorStatus();
    return true;
}

bool FileStream::WriteInt16(qint16 value)
{
    file->write((char *)&value, sizeof(qint16));
    CheckFileIOErrorStatus();
    return true;
}

bool FileStream::WriteUInt16(quint16 value)
{
    file->write((char *)&value, sizeof(qint16));
    CheckFileIOErrorStatus();
    return true;
}

bool FileStream::WriteByte(quint8 value)
{
    file->write((char *)&value, sizeof(quint8));
    CheckFileIOErrorStatus();
    return true;
}

bool FileStream::WriteZeros(qint64 count)
{
    const quint8 z = 0;

    for (qint64 l = 0; l < count; l++)
    {
         file->write((char *)&z, sizeof(quint8));
         CheckFileIOErrorStatus();
    }

    return true;
}

bool FileStream::Seek(qint64 offset, SeekOrigin origin)
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

    return true;
}

bool FileStream::JumpTo(qint64 offset)
{
    return Seek(offset, SeekOrigin::Begin);
}

bool FileStream::Skip(qint64 offset)
{
    return Seek(offset, SeekOrigin::Current);
}

bool FileStream::SkipByte()
{
    return Seek(sizeof(quint8), SeekOrigin::Current);
}

bool FileStream::SkipInt16()
{
    return Seek(sizeof(quint16), SeekOrigin::Current);
}

bool FileStream::SkipInt32()
{
    return Seek(sizeof(qint32), SeekOrigin::Current);
}

bool FileStream::SkipInt64()
{
    return Seek(sizeof(quint64), SeekOrigin::Current);
}
