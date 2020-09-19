/*
 * Copyright (C) 2020 Pawel Kolodziejski
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
 * This code is based on LZX decompressor helper from
 * https://raw.githubusercontent.com/ME3Tweaks/lzxdhelper/master/lzxdhelper/lzxmain.h
 * which was originally made available under the GNU Lesser General Public License (LGPL) version 2.1
 * (C) 2020 Mgamerz.
 * (C) 2003-2010 Konstantin Nosov.
 *
 */

#include <string.h>
#include "mspack/mspack.h"
#include "mspack/lzx.h"

#define BLOCK_MAGIC     0xAE

typedef unsigned char   byte;

struct CBlockHeader
{
    byte    magic;
    byte    offset;
    byte    align;
    int     blockSize;
};

struct mspack_file
{
    byte    *buf;
    int     bufSize;
    int     pos;
    int     rest;
};

// Align integer or pointer of any type
#define ALIGN_PTR(ptr, alignment) (void *)(((size_t)ptr + alignment - 1) & ~(alignment - 1))

// Using size_t typecasts - that's platform integer type
#define OFFSET_PTR(ptr, offset) (void *)((size_t)ptr + offset)

static int mspack_read(struct mspack_file* file, void* buffer, int bytes)
{
    if (!file->rest)
    {
        // read block header
        if (file->buf[file->pos] == 0xFF)
        {
            // [0]   = FF
            // [1,2] = uncompressed block size
            // [3,4] = compressed block size
            file->rest = (file->buf[file->pos + 3] << 8) | file->buf[file->pos + 4];
            file->pos += 5;
        }
        else
        {
            // [0,1] = compressed size
            file->rest = (file->buf[file->pos + 0] << 8) | file->buf[file->pos + 1];
            file->pos += 2;
        }
        if (file->rest > file->bufSize - file->pos)
            file->rest = file->bufSize - file->pos;
    }
    if (bytes > file->rest) bytes = file->rest;
    if (bytes <= 0)
        return 0;

    // copy block data
    memcpy(buffer, file->buf + file->pos, bytes);
    file->pos += bytes;
    file->rest -= bytes;

    return bytes;
}

static int mspack_write(struct mspack_file* file, void* buffer, int bytes)
{
    memcpy(file->buf + file->pos, buffer, bytes);
    file->pos += bytes;
    return bytes;
}

void* appMalloc(int size)
{
    int alignment = 8;
    // Allocate memory
    void* block = malloc(size + sizeof(struct CBlockHeader) + (alignment - 1));
    if (!block)
    {
        // OUT OF MEMORY!
        return NULL; // Will crash
    }

    // Initialize the allocated block
    void* ptr = ALIGN_PTR(OFFSET_PTR(block, sizeof(struct CBlockHeader)), alignment);
    if (size > 0)
        memset(ptr, 0, size);

    // Prepare block header
    struct CBlockHeader* hdr = (struct CBlockHeader*)ptr - 1;
    byte offset = (byte*)ptr - (byte*)block;
    hdr->magic = BLOCK_MAGIC;
    hdr->offset = offset - 1;
    hdr->align = alignment - 1;
    hdr->blockSize = size;
    return ptr;
}

static void* mspack_alloc(struct mspack_system *self, size_t bytes)
{
    (void)self;

    return appMalloc(bytes);
}

void appFree(void* ptr)
{
    struct CBlockHeader* hdr = (struct CBlockHeader *)ptr - 1;
    hdr->magic--; // modify to any value
    int offset = hdr->offset + 1;
    void* block = OFFSET_PTR(ptr, -offset);
    free(block);
}

static void mspack_free(void* ptr)
{
    appFree(ptr);
}

static void mspack_copy(void* src, void* dst, size_t bytes)
{
    memcpy(dst, src, bytes);
}

static struct mspack_system lzxSys =
{
    NULL,// open
    NULL,// close
    mspack_read,
    mspack_write,
    NULL,// seek
    NULL,// tell
    NULL,// message
    mspack_alloc,
    mspack_free,
    mspack_copy,
    NULL
};

int lzx_decompress(byte *CompressedBuffer, int CompressedSize, byte *UncompressedBuffer, int UncompressedSize)
{
    // setup streams
    struct mspack_file src, dst;
    src.buf = CompressedBuffer;
    src.bufSize = CompressedSize;
    src.pos = 0;
    src.rest = 0;
    dst.buf = UncompressedBuffer;
    dst.bufSize = UncompressedSize;
    dst.pos = 0;
    // prepare decompressor
    struct lzxd_stream* lzxd = lzxd_init(&lzxSys, &src, &dst, 17, 0, 256 * 1024, UncompressedSize, 0);

    // decompress
    int r = lzxd_decompress(lzxd, UncompressedSize);
    // free resources
    lzxd_free(lzxd);
    return r;
}
