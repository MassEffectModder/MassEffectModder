/* WraperUnzip.cpp

        Copyright (C) 2017-2020 Pawel Kolodziejski

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
#include <cstdio>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>

#include <iomemapi.h>
#if defined(_WIN32)
#include <windows.h>
#include <direct.h>
#include <sys/stat.h>
#include <cwchar>
#include <cerrno>
#include "iowin32.h"
#endif
#include <unzip.h>

#pragma pack(4)
typedef struct
{
    zlib_filefunc64_def api;
    voidpf handle;
    unzFile file;
    unz_global_info globalInfo;
    unz_file_info curFileInfo;
    int tpfMode;
} UnzipHandle;
#pragma pack()

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

    memset(unzipHandle, 0, sizeof(UnzipHandle));
    gXor = unzipHandle->tpfMode = tpf;

#if defined(_WIN32)
    fill_win32_filefunc64W(&unzipHandle->api);
#else
    fill_fopen64_filefunc(&unzipHandle->api);
#endif
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

void *ZipOpenFromMem(unsigned char *src, unsigned long long srcLen, int *numEntries, int tpf)
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

int ZipGetCurrentFileInfo(void *handle, char **fileName,
                          int *sizeOfFileName, unsigned long long *dstLen,
                          unsigned long *fileFlags)
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
    if (*fileName == nullptr)
        return -100;
    strcpy(*fileName, f);
    *sizeOfFileName = strlen(*fileName);
    *dstLen = unzipHandle->curFileInfo.uncompressed_size;
    *fileFlags = unzipHandle->curFileInfo.external_fa;

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

int ZipReadCurrentFile(void *handle, unsigned char *dst, unsigned long long dst_len, const unsigned char *pass)
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
    unzipHandle = nullptr;
}

#if defined(_WIN32)
static int MyCreateDir(const wchar_t *name)
{
    errno_t error = _waccess_s(name, 0);
    if (error != 0 && errno != ENOENT) {
        fwprintf(stderr, L"Error: failed to check directory: %s\n", name);
        return 1;
    }
    struct _stat s{};
    _wstat(name, &s);
    if (error == 0 && !S_ISDIR(s.st_mode)) {
        fwprintf(stderr, L"Error: output path is not directory: %s\n", name);
        return 1;
    }
    if (error != 0 && !CreateDirectoryW(name, nullptr)) {
        fwprintf(stderr, L"Error: failed to create directory: %s\n", name);
        return 1;
    }
    return 0;
}
#else
static int MyCreateDir(const char *name)
{
    struct stat s{};
    int error = stat(name, &s);
    if (error == -1 && errno != ENOENT) {
        fprintf(stderr, "Error: failed to check directory: %s\n", name);
        return 1;
    }
    if (error == 0 && !S_ISDIR(s.st_mode)) {
        fprintf(stderr, "Error: output path is not directory: %s\n", name);
        return 1;
    }
    if (error == -1 && mkdir(name, 0755) != 0) {
        fprintf(stderr, "Error: failed to create directory: %s\n", name);
        return 1;
    }
    return 0;
}
#endif

int ZipUnpack(const void *path, const void *output_path, bool full_path)
{
    int result;
    unsigned long long dstLen = 0;
    int numEntries = 0;
#if defined(_WIN32)
    auto outputDir = static_cast<const wchar_t *>(output_path);
#else
    const char *outputDir = static_cast<const char *>(output_path);
#endif

    void *handle = ZipOpenFromFile(path, &numEntries, 0);
    if (handle == nullptr)
        goto failed;

    for (int i = 0; i < numEntries; i++)
    {
        char *filetmp;
        int filetmplen = 0;
        unsigned long fileFlags = 0;
        result = ZipGetCurrentFileInfo(handle, &filetmp, &filetmplen, &dstLen, &fileFlags);
        if (result != 0)
            goto failed;
        char fileName[strlen(filetmp) + 1];
        strcpy(fileName, filetmp);
        delete[] filetmp;

        if (dstLen == 0)
        {
            ZipGoToNextFile(handle);
            continue;
        }

        printf("%s\n", fileName);

        auto data = new unsigned char[dstLen];
        result = ZipReadCurrentFile(handle, data, dstLen, nullptr);
        if (result != 0)
        {
            delete[] data;
            goto failed;
        }

#if defined(_WIN32)
        int size = strlen(fileName) + 1;
        wchar_t tmpfile[size];
        mbstowcs(tmpfile, fileName, size);
        size = wcslen(outputDir) + strlen(fileName) + 2;
        wchar_t outputPath[size];
        wchar_t outputFile[size];

        wcscpy(outputPath, tmpfile);
        wcscpy(outputFile, tmpfile);

        for (int j = 0; tmpfile[j] != 0; j++)
        {
            if (tmpfile[j] == '/' && tmpfile[1] != ':')
            {
                if (full_path)
                {
                    tmpfile[j] = 0;
                    if (outputDir && outputDir[0] != 0)
                        swprintf(outputPath, size, L"%s/%s", outputDir, tmpfile);
                    else
                        wcscpy(outputPath, tmpfile);

                    if (MyCreateDir(outputPath) != 0)
                    {
                        result = 1;
                        break;
                    }
                    tmpfile[j] = '/';
                }
                else
                {
                    if (outputDir && outputDir[0] != 0)
                        swprintf(outputPath, size, L"%s/%s", outputDir, tmpfile + j + 1);
                    else
                        wcscpy(outputPath, tmpfile + j + 1);
                }
            }
        }

        if (outputDir && outputDir[0] != 0)
        {
            swprintf(outputFile, size, "%s/%s", outputDir, tmpfile);
        }
        else
        {
            wcscpy(outputFile, tmpfile);
        }

        if (result != 0)
        {
            delete[] data;
            result = 1;
            break;
        }
        HANDLE file = CreateFileW(outputPath,
                                    GENERIC_WRITE,
                                    FILE_SHARE_READ, nullptr,
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE)
        {
            delete[] data;
            fwprintf(stderr, L"Failed to write to file: %s", outputPath);
            result = 1;
            break;
        }
        DWORD nWritten = 0;
        if (!WriteFile(file, data, dstLen, &nWritten, nullptr))
        {
            delete[] data;
            CloseHandle(file);
            result = 1;
            break;
        }
        CloseHandle(file);
#else
        char outputPath[strlen(outputDir) + strlen(fileName) + 2];
        char outputFile[strlen(outputDir) + strlen(fileName) + 2];
        char tmpfile[strlen(fileName) + 1];

        strcpy(outputPath, fileName);
        strcpy(outputFile, fileName);
        strcpy(tmpfile, fileName);

        for (int j = 0; tmpfile[j] != 0; j++)
        {
            if (tmpfile[j] == '/')
            {
                if (full_path)
                {
                    tmpfile[j] = 0;
                    if (outputDir && outputDir[0] != 0)
                        sprintf(outputPath, "%s/%s", outputDir, tmpfile);
                    else
                        strcpy(outputPath, tmpfile);
                    if (MyCreateDir(outputPath) != 0)
                    {
                        result = 1;
                        break;
                    }
                    tmpfile[j] = '/';
                }
                else
                {
                    if (outputDir && outputDir[0] != 0)
                        sprintf(outputPath, "%s/%s", outputDir, tmpfile + j + 1);
                    else
                        strcpy(outputPath, tmpfile + j + 1);
                }
            }
        }
        if (result != 0)
        {
            delete[] data;
            result = 1;
            break;
        }

        if (outputDir && outputDir[0] != 0)
        {
            sprintf(outputFile, "%s/%s", outputDir, tmpfile);
        }
        else
        {
            strcpy(outputFile, tmpfile);
        }

        FILE *file = fopen(outputFile, "wb+");
        if (!file)
        {
            delete[] data;
            printf("Failed to write to file: %s", outputFile);
            result = 1;
            break;
        }
        unsigned long long size = fwrite(data, 1, dstLen, file);
        if (size != dstLen)
        {
            delete[] data;
            ferror(file);
            fclose(file);
            result = 1;
            break;
        }
        fclose(file);
        if (((fileFlags >> 16) & 0xFFFF))
            chmod(outputFile, (fileFlags >> 16) & 0xFFFF);
#endif

        delete[] data;
        ZipGoToNextFile(handle);
    }
    ZipClose(handle);
    handle = nullptr;
    if (result == 0)
        printf("\nEverything is Ok\n");
    fflush(stdout);
    return result;

failed:

#if defined(_WIN32)
    fwprintf(stderr, L"Zip file damaged: %s", (wchar_t *)path);
#else
    printf("Zip file damaged: %s", (char *)path);
#endif
    if (handle != nullptr)
        ZipClose(handle);
    handle = nullptr;

    return 1;
}
