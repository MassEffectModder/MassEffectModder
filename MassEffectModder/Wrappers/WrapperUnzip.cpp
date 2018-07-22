/* WraperUnzip.cpp

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

#include <cstring>

#include <iomemapi.h>
#include <unzip.h>

#pragma pack(push, 4)

typedef struct
{
    zlib_filefunc64_def api;
    voidpf handle;
    unzFile file;
    unz_global_info globalInfo;
    unz_file_info curFileInfo;
    int tpfMode;
} UnzipHandle;

#pragma pack(pop)

static unsigned char tpfPassword[] =
{
    0x73, 0x2A, 0x63, 0x7D, 0x5F, 0x0A, 0xA6, 0xBD,
    0x7D, 0x65, 0x7E, 0x67, 0x61, 0x2A, 0x7F, 0x7F,
    0x74, 0x61, 0x67, 0x5B, 0x60, 0x70, 0x45, 0x74,
    0x5C, 0x22, 0x74, 0x5D, 0x6E, 0x6A, 0x73, 0x41,
    0x77, 0x6E, 0x46, 0x47, 0x77, 0x49, 0x0C, 0x4B,
    0x46, 0x6F, '\0'
};

unsigned char tpfXorKey[2] = { 0xA4, 0x3F };
int gXor = 0;

void *ZipOpenFromFile(const void *path, int *numEntries, int tpf)
{
    UnzipHandle *unzipHandle;
    int result;

    unzipHandle = static_cast<UnzipHandle *>(malloc(sizeof(UnzipHandle)));
    if (unzipHandle == nullptr || numEntries == nullptr)
        return nullptr;

    gXor = unzipHandle->tpfMode = tpf;

    unzipHandle->file = unzOpenIoFile(path, &unzipHandle->api);
    if (unzipHandle->file == nullptr)
    {
        free(unzipHandle);
        return nullptr;
    }
    result = unzGetGlobalInfo(unzipHandle->file, &unzipHandle->globalInfo);
    if (result != UNZ_OK)
    {
        unzClose(unzipHandle->file);
        free(unzipHandle);
        return nullptr;
    }

    *numEntries = unzipHandle->globalInfo.number_entry;

    return static_cast<void *>(unzipHandle);
}

void *ZipOpenFromMem(unsigned char *src, unsigned long srcLen, int *numEntries, int tpf)
{
    UnzipHandle *unzipHandle;
    int result;

    unzipHandle = static_cast<UnzipHandle *>(malloc(sizeof(UnzipHandle)));
    if (unzipHandle == nullptr || numEntries == nullptr)
        return nullptr;

    gXor = unzipHandle->tpfMode = tpf;

    unzipHandle->handle = create_ioapi_from_buffer(&unzipHandle->api, src, srcLen);
    if (unzipHandle->handle == nullptr)
    {
        free(unzipHandle);
        return nullptr;
    }
    unzipHandle->file = unzOpenIoMem(unzipHandle->handle, &unzipHandle->api, 1);
    if (unzipHandle->file == nullptr)
    {
        free(unzipHandle);
        return nullptr;
    }
    result = unzGetGlobalInfo(unzipHandle->file, &unzipHandle->globalInfo);
    if (result != UNZ_OK)
    {
        unzClose(unzipHandle->file);
        free(unzipHandle);
        return nullptr;
    }

    *numEntries = unzipHandle->globalInfo.number_entry;

    return static_cast<void *>(unzipHandle);
}

int ZipGetCurrentFileInfo(void *handle, char **fileName, int *sizeOfFileName, unsigned long *dstLen)
{
    auto unzipHandle = static_cast<UnzipHandle *>(handle);
    int result;
    char f[256];

    if (unzipHandle == nullptr || sizeOfFileName == nullptr || dstLen == nullptr)
        return -1;

    *sizeOfFileName = sizeof(f);
    result = unzGetCurrentFileInfo(unzipHandle->file, &unzipHandle->curFileInfo, f, *sizeOfFileName, nullptr, 0, nullptr, 0);
    if (result != UNZ_OK)
        return result;

    *fileName = new char[*sizeOfFileName];
    strcpy(*fileName, f);
    *sizeOfFileName = strlen(*fileName);
    *dstLen = unzipHandle->curFileInfo.uncompressed_size;

    return 0;
}

int ZipGoToFirstFile(void *handle)
{
    auto unzipHandle = static_cast<UnzipHandle *>(handle);
    int result;

    if (unzipHandle == nullptr)
        return -1;

    result = unzGoToFirstFile(unzipHandle->file);
    if (result != UNZ_OK)
        return result;

    return 0;
}

int ZipGoToNextFile(void *handle)
{
    auto unzipHandle = static_cast<UnzipHandle *>(handle);
    int result;

    if (unzipHandle == nullptr)
        return -1;

    result = unzGoToNextFile(unzipHandle->file);
    if (result != UNZ_OK)
        return result;

    return 0;
}

int ZipLocateFile(void *handle, const char *filename)
{
    auto unzipHandle = static_cast<UnzipHandle *>(handle);
    int result;

    if (unzipHandle == nullptr || filename == nullptr)
        return -1;

    result = unzLocateFile(unzipHandle->file, filename, 2);
    if (result != UNZ_OK)
        return result;

    return 0;
}

int ZipReadCurrentFile(void *handle, unsigned char *dst, unsigned long dst_len, const unsigned char *pass)
{
    auto unzipHandle = static_cast<UnzipHandle *>(handle);
    int result;

    if (unzipHandle == nullptr || dst == nullptr)
        return -1;

    if ((unzipHandle->curFileInfo.flag & 1) != 0)
    {
        result = unzOpenCurrentFilePassword(unzipHandle->file,
                unzipHandle->tpfMode == 1 ? reinterpret_cast<char *>(tpfPassword) :
                                            pass == nullptr ? "" : reinterpret_cast<const char *>(pass));
    }
    else
    {
        result = unzOpenCurrentFile(unzipHandle->file);
    }
    if (result != UNZ_OK)
        return result;

    result = unzReadCurrentFile(unzipHandle->file, dst, static_cast<unsigned>(dst_len));
    if (result < 0)
        return result;

    result = unzCloseCurrentFile(unzipHandle->file);
    if (result != UNZ_OK)
        return result;

    return 0;
}

void ZipClose(void *handle)
{
    auto unzipHandle = static_cast<UnzipHandle *>(handle);

    if (unzipHandle == nullptr)
        return;

    unzClose(unzipHandle->file);

    free(unzipHandle);
}
