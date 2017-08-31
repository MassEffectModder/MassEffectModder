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

#include <stdio.h>
#include <stdint.h>

#include "Helpers/StreamIO.h"

typedef enum {
    Append = 0,
    Create,
    CreateNew,
    Open,
    OpenOrCreate,
    Truncate
} FileMode;

typedef enum {
    Read = 0,
    ReadWrite,
    Write
} FileAccess;

class FileStream : public StreamIO
{
    private:

    FILE *fileHandle;

public:

    FileStream();
    FileStream(QString &path, FileMode mode);
    ~FileStream();

    void Flush();
    void Close();

    void CopyTo(StreamIO *stream);
    void CopyTo(StreamIO *stream, int32_t bufferSize);
    void CopyTo(StreamIO *stream, uint32_t bufferSize);
    void CopyFrom(StreamIO *stream);
    void CopyFrom(StreamIO *stream, int32_t bufferSize);
    void CopyFrom(StreamIO *stream, uint32_t bufferSize);
    int64_t ReadToBuffer(uint8_t *buffer, int64_t offset, int64_t count);
    int64_t ReadToBuffer(uint8_t *buffer, uint64_t offset, int64_t count);
    uint64_t ReadToBuffer(uint8_t *buffer, int64_t offset, uint64_t count);
    uint64_t ReadToBuffer(uint8_t *buffer, uint64_t offset, uint64_t count);
    int32_t ReadToBuffer(uint8_t *buffer, int32_t offset, int32_t count);
    int32_t ReadToBuffer(uint8_t *buffer, uint32_t offset, int32_t count);
    uint32_t ReadToBuffer(uint8_t *buffer, int32_t offset, uint32_t count);
    uint32_t ReadToBuffer(uint8_t *buffer, uint32_t offset, uint32_t count);
    int ReadByte();
    int64_t Seek(int64_t offset, SeekOrigin origin);
    void WriteFromBuffer(uint8_t *buffer, int64_t offset, int64_t count);
    void WriteByte(uint8_t value);
    void WriteByte(uint32_t value);
    void WriteByte(int32_t value);
    void ReadStringASCII(QString &string, int32_t count);
    void ReadStringASCII(QString &string, uint32_t count);
    void ReadStringASCIINull(QString &string);
    void ReadStringUnicode(QString &string, int32_t count);
    void ReadStringUnicode(QString &string, uint32_t count);
    void ReadStringUnicodeNull(QString &string);
    void WriteStringASCII(QString &string);
    void WriteStringASCIINull(QString &string);
    int64_t ReadInt64();
    uint64_t ReadUInt64();
    int32_t ReadInt32();
    uint32_t ReadUInt32();
    int16_t ReadInt16();
    uint16_t ReadUInt16();
    void WriteInt64(int64_t value);
    void WriteUInt64(uint64_t value);
    void WriteInt32(int32_t value);
    void WriteUInt32(uint32_t value);
    void WriteInt16(int16_t value);
    void WriteUInt16(uint16_t value);
    void WriteZeros(int32_t count);
    void WriteZeros(uint32_t count);
    void JumpTo(int32_t offset);
    void JumpTo(uint32_t offset);
    void JumpTo(int64_t offset);
    void JumpTo(uint64_t offset);
    void Skip(int32_t offset);
    void Skip(uint32_t offset);
    void SkipByte();
    void SkipInt16();
    void SkipInt32();
    void SkipInt64();
};

#endif
