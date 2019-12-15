/* The MIT License
 *
 * Copyright (c) 2018-2019 Pawel Kolodziejski
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

#ifndef BYTE_BUFFER_H
#define BYTE_BUFFER_H

#include <Common/Types.h>
#include <Common/Exception.h>

struct ByteBuffer
{
private:

    uint8_t *_ptr;
    int64_t _size;

public:

    ByteBuffer()
    {
        _ptr = nullptr;
        _size = 0;
    }

    ByteBuffer(uint64_t size)
    {
        _ptr = new uint8_t[size];
        if (_ptr == nullptr)
            CRASH_MSG((std::string("ByteBuffer: Out of memory! - allocation size: ") + std::to_string(size)).c_str());
        _size = size;
    }

    ByteBuffer(const uint8_t *ptr, uint64_t size)
    {
        _ptr = new uint8_t[size];
        if (_ptr == nullptr)
            CRASH_MSG((std::string("ByteBuffer: Out of memory! - allocation size: ") + std::to_string(size)).c_str());
        memcpy(_ptr, ptr, size);
        _size = size;
    }

    void Free()
    {
        delete[] _ptr;
        _ptr = nullptr;
    }

    [[nodiscard]] uint8_t *ptr() const
    {
        return _ptr;
    }

    [[nodiscard]] int64_t size() const
    {
        return _size;
    }
};

#endif
