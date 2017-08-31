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

#include <Helpers/FileStream.h>

FileStream::FileStream()
{

}

FileStream::~FileStream()
{

}

void FileStream::Flush()
{

}

void FileStream::Close()
{

}

void FileStream::CopyTo(StreamIO *stream)
void FileStream::CopyTo(StreamIO *stream, int32_t bufferSize)
void FileStream::CopyTo(StreamIO *stream, uint32_t bufferSize)
void FileStream::CopyFrom(StreamIO *stream)
void FileStream::CopyFrom(StreamIO *stream, int32_t bufferSize)
void FileStream::CopyFrom(StreamIO *stream, uint32_t bufferSize)
int64_t FileStream::ReadToBuffer(uint8_t *buffer, int64_t offset, int64_t count)
int64_t FileStream::ReadToBuffer(uint8_t *buffer, uint64_t offset, int64_t count)
uint64_t FileStream::ReadToBuffer(uint8_t *buffer, int64_t offset, uint64_t count)
uint64_t FileStream::ReadToBuffer(uint8_t *buffer, uint64_t offset, uint64_t count)
int32_t FileStream::ReadToBuffer(uint8_t *buffer, int32_t offset, int32_t count)
int32_t FileStream::ReadToBuffer(uint8_t *buffer, uint32_t offset, int32_t count)
uint32_t FileStream::ReadToBuffer(uint8_t *buffer, int32_t offset, uint32_t count)
uint32_t FileStream::ReadToBuffer(uint8_t *buffer, uint32_t offset, uint32_t count)
int FileStream::ReadByte()
int64_t FileStream::Seek(int64_t offset, SeekOrigin origin)
void FileStream::WriteFromBuffer(uint8_t *buffer, int64_t offset, int64_t count)
void FileStream::WriteByte(uint8_t value)
void FileStream::WriteByte(uint32_t value)
void FileStream::WriteByte(int32_t value)
void FileStream::ReadStringASCII(QString &string, int32_t count)
void FileStream::ReadStringASCII(QString &string, uint32_t count)
void FileStream::ReadStringASCIINull(QString &string)
void FileStream::ReadStringUnicode(QString &string, int32_t count)
void FileStream::ReadStringUnicode(QString &string, uint32_t count)
void FileStream::ReadStringUnicodeNull(QString &string)
void FileStream::WriteStringASCII(QString &string)
void FileStream::WriteStringASCIINull(QString &string)
int64_t FileStream::ReadInt64()
uint64_t FileStream::ReadUInt64()
int32_t FileStream::ReadInt32()
uint32_t FileStream::ReadUInt32()
int16_t FileStream::ReadInt16()
uint16_t FileStream::ReadUInt16()
void FileStream::WriteInt64(int64_t value)
void FileStream::WriteUInt64(uint64_t value)
void FileStream::WriteInt32(int32_t value)
void FileStream::WriteUInt32(uint32_t value)
void FileStream::WriteInt16(int16_t value)
void FileStream::WriteUInt16(uint16_t value)
void FileStream::WriteZeros(int32_t count)
void FileStream::WriteZeros(uint32_t count)
void FileStream::JumpTo(int32_t offset)
void FileStream::JumpTo(uint32_t offset)
void FileStream::JumpTo(int64_t offset)
void FileStream::JumpTo(uint64_t offset)
void FileStream::Skip(int32_t offset)
void FileStream::Skip(uint32_t offset)
void FileStream::SkipByte()
void FileStream::SkipInt16()
void FileStream::SkipInt32()
void FileStream::SkipInt64()
