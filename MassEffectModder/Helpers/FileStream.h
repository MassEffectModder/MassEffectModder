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

#ifndef FILESTREAM_H
#define FILESTREAM_H

#include <QtGlobal>

#include "Helpers/Stream.h"

typedef enum {
    Open = 0,
    Create,
} FileMode;

typedef enum {
    ReadOnly = 0,
    ReadWrite,
    WriteOnly
} FileAccess;

class QFile;

class FileStream : public Stream
{
private:

    QFile *file;
    QString filePath;
    QString errorString;

    void CheckFileIOErrorStatus();

public:

    FileStream(QString &path, FileMode mode)
      : FileStream(path, mode, FileAccess::ReadWrite) {}
    FileStream(QString &path, FileMode mode, FileAccess access);
    ~FileStream();

    bool isOpen() { return file->isOpen(); }
    void Flush();
    void Close();

    void CopyFrom(Stream *stream, qint64 count, qint64 bufferSize = 10000);
    void ReadToBuffer(quint8 *buffer, qint64 count);
    void WriteFromBuffer(quint8 *buffer, qint64 count);
    void ReadStringASCII(QString &str, qint64 count);
    void ReadStringASCIINull(QString &str);
    void ReadStringUnicode16(QString &str, qint64 count);
    void ReadStringUnicode16Null(QString &str);
    void WriteStringASCII(QString &str);
    void WriteStringASCIINull(QString &str);
    void WriteStringUnicode16(QString &str);
    void WriteStringUnicode16Null(QString &str);
    qint64 ReadInt64();
    quint64 ReadUInt64();
    qint32 ReadInt32();
    quint32 ReadUInt32();
    qint16 ReadInt16();
    quint16 ReadUInt16();
    quint8 ReadByte();
    void WriteInt64(qint64 value);
    void WriteUInt64(quint64 value);
    void WriteInt32(qint32 value);
    void WriteUInt32(quint32 value);
    void WriteInt16(qint16 value);
    void WriteUInt16(quint16 value);
    void WriteByte(quint8 value);
    void WriteZeros(qint64 count);
    void Seek(qint64 offset, SeekOrigin origin);
    void JumpTo(qint64 offset);
    void Skip(qint64 offset);
    void SkipByte();
    void SkipInt16();
    void SkipInt32();
    void SkipInt64();
};

#endif
