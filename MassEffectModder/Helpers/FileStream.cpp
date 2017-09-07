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

void FileStream::UpdateErrorStatus()
{
    if (file->error() != QFileDevice::NoError)
    {
        errorOccured = false;
    }
    else
    {
        errorOccured = true;
        errorString = file->errorString();
    }
}

FileStream::FileStream(QString &path, FileMode mode, FileAccess access)
    : file(nullptr), errorOccured(false)
{
    QFile::OpenMode openFlags = 0;
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
        CRASH_MSG((QString("Filed to open file: ") + path + QString(" Error: ") + file->errorString()).toStdString().c_str());
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
    qint64 copied = 0;
    qint64 status;
    quint8 *buffer = new quint8[bufferSize];
    do
    {
        status = stream->ReadToBuffer(buffer, qMin(bufferSize, count));
        UpdateErrorStatus();
        if (status == -1)
            break;
        status = WriteFromBuffer(buffer, count);
        UpdateErrorStatus();
        if (status == -1)
            break;
        copied += status;
        count -= status;
    } while (count != 0);

    delete[] buffer;

    if (status == -1)
        return false;
    else
        return true;
}

bool FileStream::ReadToBuffer(quint8 *buffer, qint64 count)
{
    qint64 readed = file->read((char *)buffer, count);
    UpdateErrorStatus();
    return readed;
}

bool FileStream::WriteFromBuffer(quint8 *buffer, qint64 count)
{
    qint64 written = file->write((char *)buffer, count);
    UpdateErrorStatus();
    return written;
}

bool FileStream::ReadStringASCII(QString &string, qint64 count)
{
    char *buffer = new char[count + 1];

    buffer[count] = 0;

    qint64 readed = file->read(buffer, count);
    if (readed == count)
        string = QString(buffer);

    delete[] buffer;

    UpdateErrorStatus();

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
        if (!file->getChar(&c))
            break;
        if (!c)
        {
            UpdateErrorStatus();
            return true;
        }
        str += c;
    } while (!file->atEnd() || file->error() != QFileDevice::NoError);

    UpdateErrorStatus();
    return false;
}

bool FileStream::ReadStringUnicode16(QString &str, qint64 count)
{
    count *= 2;
    char *buffer = new char[count + 2];

    buffer[count] = 0;
    buffer[count + 1] = 0;

    qint64 readed = file->read(buffer, count);
    if (readed == count)
        str = QString(buffer);

    delete[] buffer;

    UpdateErrorStatus();

    return true;
}

bool FileStream::ReadStringUnicode16Null(QString &str)
{
    do
    {
        quint16 c = ReadUInt16();
        if (errorOccured)
            return false;
        if (!c)
        {
            return true;
        }
        str += QChar((ushort)c);
    } while (!file->atEnd() || file->error() != QFileDevice::NoError);

    return false;
}

bool FileStream::WriteStringASCII(QString &str)
{
    const char *s = str.toStdString().c_str();
    if (file->write(s, str.length()) == -1)
    {
        UpdateErrorStatus();
        return false;
    }
    else
    {
        UpdateErrorStatus();
        return true;
    }
}

bool FileStream::WriteStringASCIINull(QString &str)
{
    if (!WriteStringASCII(str))
        return false;
    if (!WriteByte(0))
        return false;
    return true;
}

bool FileStream::WriteStringUnicode16(QString &str)
{
    const ushort *s = str.utf16();
    if (file->write((char *)s, str.length() * 2) == -1)
    {
        UpdateErrorStatus();
        return false;
    }
    else
    {
        UpdateErrorStatus();
        return true;
    }
}

bool FileStream::WriteStringUnicode16Null(QString &str)
{
    if (!WriteStringUnicode16(str))
        return false;
    if (!WriteUInt16(0))
        return false;
    return true;
}

qint64 FileStream::ReadInt64()
{
    qint64 value;
    file->read((char *)&value, sizeof(qint64));
    UpdateErrorStatus();
    return value;
}

quint64 FileStream::ReadUInt64()
{
    quint64 value;
    file->read((char *)&value, sizeof(quint64));
    UpdateErrorStatus();
    return value;
}

qint32 FileStream::ReadInt32()
{
    qint32 value;
    file->read((char *)&value, sizeof(qint32));
    UpdateErrorStatus();
    return value;
}

quint32 FileStream::ReadUInt32()
{
    quint32 value;
    file->read((char *)&value, sizeof(quint32));
    UpdateErrorStatus();
    return value;
}

qint16 FileStream::ReadInt16()
{
    qint16 value;
    file->read((char *)&value, sizeof(qint16));
    UpdateErrorStatus();
    return value;
}

quint16 FileStream::ReadUInt16()
{
    quint16 value;
    file->read((char *)&value, sizeof(quint16));
    UpdateErrorStatus();
    return value;
}

quint8 FileStream::ReadByte()
{
    quint8 value;
    file->read((char *)&value, sizeof(quint8));
    UpdateErrorStatus();
    return value;
}

bool FileStream::WriteInt64(qint64 value)
{
    if (file->write((char *)&value, sizeof(qint64)) == -1)
    {
        UpdateErrorStatus();
        return false;
    }
    else
    {
        UpdateErrorStatus();
        return true;
    }
}

bool FileStream::WriteUInt64(quint64 value)
{
    if (file->write((char *)&value, sizeof(quint64)) == -1)
    {
        UpdateErrorStatus();
        return false;
    }
    else
    {
        UpdateErrorStatus();
        return true;
    }
}

bool FileStream::WriteInt32(qint32 value)
{
    if (file->write((char *)&value, sizeof(qint32)) == -1)
    {
        UpdateErrorStatus();
        return false;
    }
    else
    {
        UpdateErrorStatus();
        return true;
    }
}

bool FileStream::WriteUInt32(quint32 value)
{
    if (file->write((char *)&value, sizeof(quint32)) == -1)
    {
        UpdateErrorStatus();
        return false;
    }
    else
    {
        UpdateErrorStatus();
        return true;
    }
}

bool FileStream::WriteInt16(qint16 value)
{
    if (file->write((char *)&value, sizeof(qint16)) == -1)
    {
        UpdateErrorStatus();
        return false;
    }
    else
    {
        UpdateErrorStatus();
        return true;
    }
}

bool FileStream::WriteUInt16(quint16 value)
{
    if (file->write((char *)&value, sizeof(quint16)) == -1)
    {
        UpdateErrorStatus();
        return false;
    }
    else
    {
        UpdateErrorStatus();
        return true;
    }
}

bool FileStream::WriteByte(quint8 value)
{
    if (file->write((char *)&value, sizeof(quint8)) == -1)
    {
        UpdateErrorStatus();
        return false;
    }
    else
    {
        UpdateErrorStatus();
        return true;
    }
}

bool FileStream::WriteZeros(qint64 count)
{
    const quint8 z = 0;

    for (qint64 l = 0; l < count; l++)
    {
         if (file->write((char *)&z, sizeof(quint8) == -1))
         {
             UpdateErrorStatus();
             return false;
         }
    }

    UpdateErrorStatus();
    return true;
}

bool FileStream::Seek(qint64 offset, SeekOrigin origin)
{
    switch (origin)
    {
    case SeekOrigin::Begin:
        if (!file->seek(offset))
        {
            UpdateErrorStatus();
            return false;
        }
        break;
    case SeekOrigin::Current:
        if (!file->seek(file->pos() + offset))
        {
            UpdateErrorStatus();
            return false;
        }
        break;
    case SeekOrigin::End:
        if (!file->seek(file->size() - offset))
        {
            UpdateErrorStatus();
            return false;
        }
        break;
    }

    UpdateErrorStatus();
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
