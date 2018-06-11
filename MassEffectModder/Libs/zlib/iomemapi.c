/* iomemapi.c

        Copyright (C) 2017-2018 Pawel Kolodziejski <aquadran at users.sourceforge.net>

        ---------------------------------------------------------------------------------

        Condition of use and distribution are the same than zlib :

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgement in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  ---------------------------------------------------------------------------------
*/

#include <string.h>

#include "iomemapi.h"

typedef struct _IOMEMHANDLE
{
	voidpf buffer;
	ZPOS64_T bufferLen;
	ZPOS64_T bufferPos;
} IOMEMHANDLE;

static uLong ZCALLBACK iomem_read_func(voidpf opaque __attribute__ ((unused)), voidpf stream, voidpf buf, uLong size)
{
    IOMEMHANDLE* ioMemHandle = stream;

    if (ioMemHandle == Z_NULL || buf == Z_NULL)
        return 0;

    if (ioMemHandle->bufferPos + size > ioMemHandle->bufferLen)
        size = (uLong)(ioMemHandle->bufferLen - ioMemHandle->bufferPos);

    if (gXor)
    {
        unsigned char *src = ioMemHandle->buffer;
        unsigned char *dst = buf;
        unsigned long pos = 0;
        if (ioMemHandle->bufferPos & 1)
            dst[pos++] = src[ioMemHandle->bufferPos] ^ tpfXorKey[1];
        for (unsigned long i = pos; i < size; i++)
            dst[i] = src[ioMemHandle->bufferPos + i] ^ tpfXorKey[(i - pos) % 2];
    }
    else
    {
        memcpy(buf, (char *)ioMemHandle->buffer + ioMemHandle->bufferPos, size);
    }
    ioMemHandle->bufferPos += size;

    return size;
}

static uLong ZCALLBACK iomem_write_func(voidpf opaque __attribute__ ((unused)), voidpf stream, voidpc buf, uLong size)
{
    IOMEMHANDLE* ioMemHandle = stream;

    if (ioMemHandle == Z_NULL || buf == Z_NULL)
        return 0;

    if (ioMemHandle->bufferPos + size > ioMemHandle->bufferLen)
    {
        voidp tmp = realloc(ioMemHandle->buffer, size);
        if (tmp == Z_NULL)
            return 0;
        ioMemHandle->buffer = tmp;
        ioMemHandle->bufferLen = ioMemHandle->bufferPos + size;
    }

    if (gXor)
    {
        unsigned char *dst = ioMemHandle->buffer;
        unsigned char *src = (unsigned char *)buf;
        unsigned long pos = (unsigned long)ioMemHandle->bufferPos;
        if (pos & 1)
            dst[pos++] = src[0] ^ tpfXorKey[1];
        for (unsigned long i = pos; i < size; i++)
            dst[ioMemHandle->bufferPos + i] = src[i] ^ tpfXorKey[(i - pos) % 2];
    }
    else
    {
        memcpy((char*)ioMemHandle->buffer + ioMemHandle->bufferPos, buf, size);
    }
    ioMemHandle->bufferPos += size;

    return size;
}

static ZPOS64_T ZCALLBACK iomem_tell_func(voidpf opaque __attribute__ ((unused)), voidpf stream)
{
    IOMEMHANDLE* ioMemHandle = stream;

    if (ioMemHandle == Z_NULL)
        return 0;

    return ioMemHandle->bufferPos;
}

static long ZCALLBACK iomem_seek_func(voidpf opaque __attribute__ ((unused)), voidpf stream, ZPOS64_T offset, int origin)
{
    IOMEMHANDLE* ioMemHandle = stream;

    if (ioMemHandle == Z_NULL)
        return -1;

    switch (origin)
    {
    case ZLIB_FILEFUNC_SEEK_CUR:
        if (ioMemHandle->bufferPos + offset > ioMemHandle->bufferLen)
        {
            return -1;
        }
        ioMemHandle->bufferPos += offset;
        break;
    case ZLIB_FILEFUNC_SEEK_END:
        if (ioMemHandle->bufferLen - offset > ioMemHandle->bufferLen)
        {
            return -1;
        }
        ioMemHandle->bufferPos = ioMemHandle->bufferLen - offset;
        break;
    case ZLIB_FILEFUNC_SEEK_SET:
        if (offset > ioMemHandle->bufferLen)
        {
            return -1;
        }
        ioMemHandle->bufferPos = offset;
        break;
    default:
        return -1;
    }

    return 0;
}

static int ZCALLBACK iomem_close_func(voidpf opaque __attribute__ ((unused)), voidpf stream)
{
    IOMEMHANDLE* ioMemHandle = stream;

    if (ioMemHandle == Z_NULL)
        return -1;

    free(ioMemHandle);

    return 0;
}

static int ZCALLBACK iomem_error_func(voidpf opaque __attribute__ ((unused)), voidpf stream __attribute__ ((unused)))
{
    return 0;
}

ZEXTERN voidpf create_ioapi_from_buffer(zlib_filefunc64_def* ioMemApi, voidpf buffer, size_t bufferLen)
{
    ioMemApi->zopen64_file = Z_NULL;
    ioMemApi->zread_file = iomem_read_func;
    ioMemApi->zwrite_file = iomem_write_func;
    ioMemApi->ztell64_file = iomem_tell_func;
    ioMemApi->zseek64_file = iomem_seek_func;
    ioMemApi->zclose_file = iomem_close_func;
    ioMemApi->zerror_file = iomem_error_func;
    ioMemApi->opaque = Z_NULL;

    IOMEMHANDLE* ioMemHandle = malloc(sizeof(IOMEMHANDLE));
    if (ioMemHandle == Z_NULL)
        return Z_NULL;
    ioMemHandle->buffer = buffer;
    ioMemHandle->bufferLen = bufferLen;
    ioMemHandle->bufferPos = 0;

    return ioMemHandle;
}
