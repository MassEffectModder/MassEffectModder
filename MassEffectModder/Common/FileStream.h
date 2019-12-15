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

#ifndef FILESTREAM_H
#define FILESTREAM_H

#include "Stream.h"

typedef enum
{
    Open = 0,
    Create,
} FileMode;

typedef enum
{
    ReadOnly = 0,
    ReadWrite,
    WriteOnly
} FileAccess;

class QFile;

class FileStream : public Stream
{
private:

    std::wfstream *file;
    std::wstring  filePath;

    void CheckFileIOErrorStatus();

public:

    FileStream(const QString &path, FileMode mode)
      : FileStream(path, mode, FileAccess::ReadWrite) {}
    FileStream(const QString &path, FileMode mode, FileAccess access);
    ~FileStream() override;

    int64_t Length() override;
    int64_t Position() override;

    bool isOpen() { return file->isOpen(); }
    void Flush() override;
    void Close() override;

    void CopyFrom(Stream &stream, int64_t count, int64_t bufferSize = 10000) override;
    void ReadToBuffer(uint8_t *buffer, int64_t count) override;
    ByteBuffer ReadToBuffer(int64_t count) override;
    ByteBuffer ReadAllToBuffer();
    void WriteFromBuffer(uint8_t *buffer, int64_t count) override;
    void WriteFromBuffer(const ByteBuffer &buffer) override;
    void ReadStringASCII(std::string &str, int64_t count) override;
    void ReadStringASCIINull(std::string &str) override;
    void ReadStringUnicode16(std::wstring &str, int64_t count) override;
    void ReadStringUnicode16Null(std::wstring &str) override;
    void WriteStringASCII(const std::string &str) override;
    void WriteStringASCIINull(const std::string &str) override;
    void WriteStringUnicode16(const std::wstring &str) override;
    void WriteStringUnicode16Null(const std::wstring &str) override;
    int64_t ReadInt64() override;
    uint64_t ReadUInt64() override;
    int32_t ReadInt32() override;
    uint32_t ReadUInt32() override;
    int16_t ReadInt16() override;
    uint16_t ReadUInt16() override;
    uint8_t ReadByte() override;
    void WriteInt64(int64_t value) override;
    void WriteUInt64(uint64_t value) override;
    void WriteInt32(int32_t value) override;
    void WriteUInt32(uint32_t value) override;
    void WriteInt16(int16_t value) override;
    void WriteUInt16(uint16_t value) override;
    void WriteByte(uint8_t value) override;
    void WriteZeros(int64_t count) override;
    void Seek(int64_t offset, SeekOrigin origin) override;
    void SeekBegin() override;
    void SeekEnd() override;
    void JumpTo(int64_t offset) override;
    void Skip(int64_t offset) override;
    void SkipByte() override;
    void SkipInt16() override;
    void SkipInt32() override;
    void SkipInt64() override;
};

#endif
