/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2019 Pawel Kolodziejski <aquadran at users.sourceforge.net>
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

#ifndef MEMORYSTREAM_H
#define MEMORYSTREAM_H

#include "Helpers/Stream.h"
#include "Helpers/ByteBuffer.h"

class MemoryStream : public Stream
{
private:

    enum BufferSize
    {
        initialBufferSize = 1000,
    };

    qint64 length;
    qint64 position;
    quint8 *internalBuffer;
    qint64 internalBufferSize;

public:

    MemoryStream();
    MemoryStream(const ByteBuffer &buffer);
    MemoryStream(const ByteBuffer &buffer, qint64 offset);
    MemoryStream(const ByteBuffer &buffer, qint64 offset, qint64 count);
    MemoryStream(QString &filename, qint64 offset, qint64 count);
    MemoryStream(QString &filename, qint64 count);
    MemoryStream(QString &filename);
    ~MemoryStream() override;

    qint64 Length() override { return length; }
    qint64 Position() override { return position; }

    void Flush() override {}
    void Close() override {}
    ByteBuffer ToArray();

    void CopyFrom(Stream &stream, qint64 count, qint64 bufferSize = 10000) override;
    void ReadToBuffer(quint8 *buffer, qint64 count) override;
    ByteBuffer ReadToBuffer(qint64 count) override;
    void WriteFromBuffer(quint8 *buffer, qint64 count) override;
    void WriteFromBuffer(const ByteBuffer &buffer) override;
    void ReadStringASCII(QString &str, qint64 count) override;
    void ReadStringASCIINull(QString &str) override;
    void ReadStringUnicode16(QString &str, qint64 count) override;
    void ReadStringUnicode16Null(QString &str) override;
    void WriteStringASCII(const QString &str) override;
    void WriteStringASCIINull(const QString &str) override;
    void WriteStringUnicode16(const QString &str) override;
    void WriteStringUnicode16Null(const QString &str) override;
    qint64 ReadInt64() override;
    quint64 ReadUInt64() override;
    qint32 ReadInt32() override;
    quint32 ReadUInt32() override;
    qint16 ReadInt16() override;
    quint16 ReadUInt16() override;
    quint8 ReadByte() override;
    void WriteInt64(qint64 value) override;
    void WriteUInt64(quint64 value) override;
    void WriteInt32(qint32 value) override;
    void WriteUInt32(quint32 value) override;
    void WriteInt16(qint16 value) override;
    void WriteUInt16(quint16 value) override;
    void WriteByte(quint8 value) override;
    void WriteZeros(qint64 count) override;
    void Seek(qint64 offset, SeekOrigin origin) override;
    void SeekBegin() override;
    void SeekEnd() override;
    void JumpTo(qint64 offset) override;
    void Skip(qint64 offset) override;
    void SkipByte() override;
    void SkipInt16() override;
    void SkipInt32() override;
    void SkipInt64() override;
};

#endif
