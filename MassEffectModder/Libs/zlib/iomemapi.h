/* iomemapi.h

        Copyright (C) 2017 Pawel Kolodziejski

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

#ifndef _IOMEMAPI_H
#define _IOMEMAPI_H

#include "zlib.h"
#include "ioapi.h"
#include "unzip.h"

#ifdef __cplusplus
extern "C" {
#endif

extern unsigned char tpfXorKey[2];
extern int gXor;

ZEXTERN voidpf create_ioapi_from_buffer(zlib_filefunc64_def* ioMemApi, voidpf buffer, size_t bufferLen);

unzFile unzOpenIoMem(voidpf stream, zlib_filefunc64_def* pzlib_filefunc64_def, int is64bitOpenFunction);

#ifdef __cplusplus
}
#endif

#endif
