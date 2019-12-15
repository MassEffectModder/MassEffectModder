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

#ifndef STREAM_H
#define STREAM_H

#include "Types.h"
#include "ByteBuffer.h"

typedef enum
{
    Begin = 0,
    Current,
    End
} SeekOrigin;

class Stream
{
protected:

    bool readable{};
    bool writtable{};
    bool seekable{};

public:

    virtual ~Stream() = 0;

    virtual int64_t Length()  = 0;
    virtual int64_t Position() = 0;

    virtual void Flush() = 0;
    virtual void Close() = 0;

    virtual void CopyFrom(Stream &stream, int64_t count, int64_t bufferSize = 10000) = 0;
    virtual void ReadToBuffer(uint8_t *buffer, int64_t count) = 0;
    virtual ByteBuffer ReadToBuffer(int64_t count) = 0;
    virtual void WriteFromBuffer(uint8_t *buffer, int64_t count) = 0;
    virtual void WriteFromBuffer(const ByteBuffer &buffer) = 0;
    virtual void ReadStringASCII(QString &str, int64_t count) = 0;
    virtual void ReadStringASCIINull(QString &str) = 0;
    virtual void ReadStringUnicode16(QString &str, int64_t count) = 0;
    virtual void ReadStringUnicode16Null(QString &str) = 0;
    virtual void WriteStringASCII(const QString &str) = 0;
    virtual void WriteStringASCIINull(const QString &str) = 0;
    virtual void WriteStringUnicode16(const QString &str) = 0;
    virtual void WriteStringUnicode16Null(const QString &str) = 0;
    virtual int64_t ReadInt64() = 0;
    virtual uint64_t ReadUInt64() = 0;
    virtual int32_t ReadInt32() = 0;
    virtual uint32_t ReadUInt32() = 0;
    virtual int16_t ReadInt16() = 0;
    virtual uint16_t ReadUInt16() = 0;
    virtual uint8_t ReadByte() = 0;
    virtual void WriteInt64(int64_t value) = 0;
    virtual void WriteUInt64(uint64_t value) = 0;
    virtual void WriteInt32(int32_t value) = 0;
    virtual void WriteUInt32(uint32_t value) = 0;
    virtual void WriteInt16(int16_t value) = 0;
    virtual void WriteUInt16(uint16_t value) = 0;
    virtual void WriteByte(uint8_t value) = 0;
    virtual void WriteZeros(int64_t count) = 0;
    virtual void Seek(int64_t offset, SeekOrigin origin) = 0;
    virtual void SeekBegin() = 0;
    virtual void SeekEnd() = 0;
    virtual void JumpTo(int64_t offset) = 0;
    virtual void Skip(int64_t offset) = 0;
    virtual void SkipByte() = 0;
    virtual void SkipInt16() = 0;
    virtual void SkipInt32() = 0;
    virtual void SkipInt64() = 0;
};

#endif
