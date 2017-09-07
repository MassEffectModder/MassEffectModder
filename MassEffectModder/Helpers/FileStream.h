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
    bool errorOccured;
    QString errorString;

    void UpdateErrorStatus();

public:

    FileStream(QString &path, FileMode mode)
      : FileStream(path, mode, FileAccess::ReadWrite) {}
    FileStream(QString &path, FileMode mode, FileAccess access);
    ~FileStream();

    bool isOpen() { return file->isOpen(); }
    void Flush();
    void Close();

    bool CopyFrom(Stream *stream, qint64 count, qint64 bufferSize = 0x10000);
    bool ReadToBuffer(quint8 *buffer, qint64 count);
    bool WriteFromBuffer(quint8 *buffer, qint64 count);
    bool ReadStringASCII(QString &str, qint64 count);
    bool ReadStringASCIINull(QString &str);
    bool ReadStringUnicode16(QString &str, qint64 count);
    bool ReadStringUnicode16Null(QString &str);
    bool WriteStringASCII(QString &str);
    bool WriteStringASCIINull(QString &str);
    bool WriteStringUnicode16(QString &str);
    bool WriteStringUnicode16Null(QString &str);
    qint64 ReadInt64();
    quint64 ReadUInt64();
    qint32 ReadInt32();
    quint32 ReadUInt32();
    qint16 ReadInt16();
    quint16 ReadUInt16();
    quint8 ReadByte();
    bool WriteInt64(qint64 value);
    bool WriteUInt64(quint64 value);
    bool WriteInt32(qint32 value);
    bool WriteUInt32(quint32 value);
    bool WriteInt16(qint16 value);
    bool WriteUInt16(quint16 value);
    bool WriteByte(quint8 value);
    bool WriteZeros(qint64 count);
    bool Seek(qint64 offset, SeekOrigin origin);
    bool JumpTo(qint64 offset);
    bool Skip(qint64 offset);
    bool SkipByte();
    bool SkipInt16();
    bool SkipInt32();
    bool SkipInt64();
};

#endif
