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

#ifndef STREAMIO_H
#define STREAMIO_H

#include <stdint.h>

class QString;

typedef enum
{
    Begin = 0,
    Current,
    End
} SeekOrigin;

class StreamIO
{
protected:

    int64_t length;
    int64_t position;
    bool readable;
    bool writtable;
    bool seekable;

    virtual ~StreamIO() {}

public:

    int64_t Length() { return length; }
    int64_t Position() { return position; }

    virtual void Flush() = 0;
    virtual void Close() = 0;

    virtual void CopyTo(StreamIO *stream) = 0;
    virtual void CopyTo(StreamIO *stream, int32_t bufferSize) = 0;
    virtual void CopyTo(StreamIO *stream, uint32_t bufferSize) = 0;
    virtual void CopyFrom(StreamIO *stream) = 0;
    virtual void CopyFrom(StreamIO *stream, int32_t bufferSize) = 0;
    virtual void CopyFrom(StreamIO *stream, uint32_t bufferSize) = 0;
    virtual int64_t ReadToBuffer(uint8_t *buffer, int64_t offset, int64_t count) = 0;
    virtual int64_t ReadToBuffer(uint8_t *buffer, uint64_t offset, int64_t count) = 0;
    virtual uint64_t ReadToBuffer(uint8_t *buffer, int64_t offset, uint64_t count) = 0;
    virtual uint64_t ReadToBuffer(uint8_t *buffer, uint64_t offset, uint64_t count) = 0;
    virtual int32_t ReadToBuffer(uint8_t *buffer, int32_t offset, int32_t count) = 0;
    virtual int32_t ReadToBuffer(uint8_t *buffer, uint32_t offset, int32_t count) = 0;
    virtual uint32_t ReadToBuffer(uint8_t *buffer, int32_t offset, uint32_t count) = 0;
    virtual uint32_t ReadToBuffer(uint8_t *buffer, uint32_t offset, uint32_t count) = 0;
    virtual int ReadByte() = 0;
    virtual int64_t Seek(int64_t offset, SeekOrigin origin) = 0;
    virtual void WriteFromBuffer(uint8_t *buffer, int64_t offset, int64_t count) = 0;
    virtual void WriteByte(uint8_t value) = 0;
    virtual void WriteByte(uint32_t value) = 0;
    virtual void WriteByte(int32_t value) = 0;
    virtual void ReadStringASCII(QString &string, int32_t count) = 0;
    virtual void ReadStringASCII(QString &string, uint32_t count) = 0;
    virtual void ReadStringASCIINull(QString &string) = 0;
    virtual void ReadStringUnicode(QString &string, int32_t count) = 0;
    virtual void ReadStringUnicode(QString &string, uint32_t count) = 0;
    virtual void ReadStringUnicodeNull(QString &string) = 0;
    virtual void WriteStringASCII(QString &string);
    virtual void WriteStringASCIINull(QString &string) = 0;
    virtual int64_t ReadInt64() = 0;
    virtual uint64_t ReadUInt64() = 0;
    virtual int32_t ReadInt32() = 0;
    virtual uint32_t ReadUInt32() = 0;
    virtual int16_t ReadInt16() = 0;
    virtual uint16_t ReadUInt16() = 0;
    virtual void WriteInt64(int64_t value) = 0;
    virtual void WriteUInt64(uint64_t value) = 0;
    virtual void WriteInt32(int32_t value) = 0;
    virtual void WriteUInt32(uint32_t value) = 0;
    virtual void WriteInt16(int16_t value) = 0;
    virtual void WriteUInt16(uint16_t value) = 0;
    virtual void WriteZeros(int32_t count) = 0;
    virtual void WriteZeros(uint32_t count) = 0;
    virtual void JumpTo(int32_t offset) = 0;
    virtual void JumpTo(uint32_t offset) = 0;
    virtual void JumpTo(int64_t offset) = 0;
    virtual void JumpTo(uint64_t offset) = 0;
    virtual void Skip(int32_t offset) = 0;
    virtual void Skip(uint32_t offset) = 0;
    virtual void SkipByte() = 0;
    virtual void SkipInt16() = 0;
    virtual void SkipInt32() = 0;
    virtual void SkipInt64() = 0;
};

#endif
