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

int ZipGetCurrentFileInfo(void *handle, char *fileName,
                          int sizeOfFileName, unsigned long long *dstLen,
                          unsigned long *fileFlags)
{
    auto unzipHandle = static_cast<UnzipHandle *>(handle);
    int result;

    if (unzipHandle == nullptr || fileName == nullptr || dstLen == nullptr)
        return -1;

    result = unzGetCurrentFileInfo(unzipHandle->file, &unzipHandle->curFileInfo, fileName, sizeOfFileName, nullptr, 0, nullptr, 0);
    if (result != UNZ_OK)
        return result;

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

int ZipReadCurrentFileToOutputFile(void *handle, const void *outputFile)
{
    auto unzipHandle = static_cast<UnzipHandle *>(handle);
    char fileName[260];
    int result;

    if (unzipHandle == nullptr || outputFile == nullptr)
        return -1;

    result = unzOpenCurrentFile(unzipHandle->file);
    if (result != UNZ_OK)
        return result;

    result = unzGetCurrentFileInfo(unzipHandle->file, &unzipHandle->curFileInfo,
                                   fileName, sizeof (fileName), nullptr, 0, nullptr, 0);
    if (result != UNZ_OK)
        return result;

    result = unzReadCurrentFileToOutputFile(unzipHandle->file, outputFile,
                                            unzipHandle->curFileInfo.uncompressed_size,
                                            &unzipHandle->api);
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
        fwprintf(stderr, L"Error: failed to check directory: %ls\n", name);
        return 1;
    }
    struct _stat s{};
    _wstat(name, &s);
    if (error == 0 && !S_ISDIR(s.st_mode)) {
        fwprintf(stderr, L"Error: output path is not directory: %ls\n", name);
        return 1;
    }
    if (error != 0 && !CreateDirectoryW(name, nullptr)) {
        fwprintf(stderr, L"Error: failed to create directory: %ls\n", name);
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

int ZipUnpack(const void *path, const void *output_path, bool full_path, bool ipc)
{
    int result = 0;
    unsigned long long dstLen = 0;
    int numEntries = 0;
#if defined(_WIN32)
    auto outputDir = static_cast<const wchar_t *>(output_path);
#else
    auto outputDir = static_cast<const char *>(output_path);
#endif
    char fileName[260];
    int lastProgress = -1;

    void *handle = ZipOpenFromFile(path, &numEntries, 0);
    if (handle == nullptr)
    {
#if defined(_WIN32)
        fwprintf(stderr, L"Error: Failed to open archive!\n");
#else
        fprintf(stderr, "Error: Failed to open archive!\n");
#endif
        goto failed;
    }

    for (int i = 0; i < numEntries; i++)
    {
        unsigned long fileFlags = 0;
        result = ZipGetCurrentFileInfo(handle, fileName, sizeof (fileName), &dstLen, &fileFlags);
        if (result != 0)
        {
#if defined(_WIN32)
            fwprintf(stderr, L"Error: Failed to get file infomation in archive, aborting!\n");
#else
            fprintf(stderr, "Error: Failed to get file infomation in archive, aborting!\n");
#endif
            goto failed;
        }
        if (dstLen == 0)
        {
#if defined(_WIN32)
            wprintf(L"%d of %d - %s - Ok\n", (i + 1), numEntries, fileName);
#else
            printf("%d of %d - %s - Ok\n", (i + 1), numEntries, fileName);
#endif
            ZipGoToNextFile(handle);
            continue;
        }

        if (ipc)
        {
#if defined(_WIN32)
            wprintf(L"[IPC]FILENAME %s\n", fileName);
#else
            printf("[IPC]FILENAME %s\n", fileName);
#endif
            int newProgress = i * 100 / numEntries;
            if (lastProgress != newProgress)
            {
                lastProgress = newProgress;
#if defined(_WIN32)
                wprintf(L"[IPC]TASK_PROGRESS %d\n", newProgress);
#else
                printf("[IPC]TASK_PROGRESS %d\n", newProgress);
#endif
            }
            fflush(stdout);
        }

#if defined(_WIN32)
        wprintf(L"%d of %d - %s - size %lld - ", (i + 1), numEntries, fileName, dstLen);
#else
        printf("%d of %d - %s - size %lld - ", (i + 1), numEntries, fileName, dstLen);
#endif

#if defined(_WIN32)
        int size = strlen(fileName) + 1;
        if (size > MAX_PATH)
        {
            fwprintf(stderr, L"Error: File name too long, skipping!\n");
            result = 1;
            goto failed;
        }

        int dest_size = wcslen(outputDir) + size + 1;
        if (dest_size > MAX_PATH)
        {
            fwprintf(stderr, L"Error: Destination path for file too long, skipping!\n");
            result = 1;
            goto failed;
        }

        wchar_t tmpPath[MAX_PATH];
        mbstowcs(tmpPath, fileName, size);

        wchar_t outputFile[MAX_PATH];
        if (outputDir && outputDir[0] != 0)
        {
            swprintf(outputFile, MAX_PATH - 1, L"%ls/%ls", outputDir, tmpPath);
        }
        else
        {
            wcsncpy(outputFile, tmpPath, MAX_PATH - 1);
        }

        for (int j = 0; tmpPath[j] != 0; j++)
        {
            if (tmpPath[j] == '/')
            {
                if (full_path)
                {
                    tmpPath[j] = 0;
                    wchar_t outputPath[MAX_PATH];
                    if (outputDir && outputDir[0] != 0)
                        swprintf(outputPath, MAX_PATH - 1, L"%ls/%ls", outputDir, tmpPath);
                    else
                        wcsncpy(outputPath, tmpPath, MAX_PATH - 1);

                    if (MyCreateDir(outputPath) != 0)
                    {
                        result = 1;
                        break;
                    }
                    tmpPath[j] = '/';
                }
                else
                {
                    if (outputDir && outputDir[0] != 0)
                        swprintf(outputFile, MAX_PATH - 1, L"%ls/%ls", outputDir, tmpPath + j + 1);
                    else
                        wcsncpy(outputFile, tmpPath + j + 1, MAX_PATH - 1);
                }
            }
        }

        if (result != 0)
        {
            result = 1;
            break;
        }
#else
        char outputFile[PATH_MAX];
        if (outputDir && outputDir[0] != 0)
        {
            snprintf(outputFile, PATH_MAX - 1, "%s/%s", outputDir, fileName);
        }
        else
        {
            strncpy(outputFile, fileName, PATH_MAX - 1);
        }

        char tmpPath[PATH_MAX];
        strncpy(tmpPath, fileName, PATH_MAX - 1);

        for (int j = 0; tmpPath[j] != 0; j++)
        {
            if ((tmpPath[j] & 0xc0) == 0x80)
            {
                continue;
            }
            if (tmpPath[j] == '/')
            {
                if (full_path)
                {
                    tmpPath[j] = 0;
                    char outputPath[PATH_MAX];
                    if (outputDir && outputDir[0] != 0)
                        snprintf(outputPath, PATH_MAX - 1, "%s/%s", outputDir, tmpPath);
                    else
                        strncpy(outputPath, tmpPath, PATH_MAX - 1);
                    if (MyCreateDir(outputPath) != 0)
                    {
                        result = 1;
                        break;
                    }
                    tmpPath[j] = '/';
                }
                else
                {
                    if (outputDir && outputDir[0] != 0)
                        snprintf(outputFile, PATH_MAX - 1, "%s/%s", outputDir, tmpPath + j + 1);
                    else
                        strncpy(outputFile, tmpPath + j + 1, PATH_MAX - 1);
                }
            }
        }
        if (result != 0)
        {
            result = 1;
            break;
        }
#endif

        result = ZipReadCurrentFileToOutputFile(handle, outputFile);
        if (result != 0)
        {
#if defined(_WIN32)
            fwprintf(stderr, L"Error: Failed to read file in archive, aborting!\n");
#else
            fprintf(stderr, "Error: Failed to read file in archive, aborting!\n");
#endif
            result = 1;
            goto failed;
        }

#if !defined(_WIN32)
        if ((fileFlags >> 16) & 0xFFFF)
            chmod(outputFile, (fileFlags >> 16) & 0xFFFF);
#endif

#if defined(_WIN32)
        wprintf(L"Ok\n");
#else
        printf("Ok\n");
#endif
        ZipGoToNextFile(handle);
    }

    ZipClose(handle);
    handle = nullptr;
    fflush(stdout);
    fflush(stderr);

    return result;

failed:

    if (handle != nullptr)
        ZipClose(handle);
    handle = nullptr;

    return 1;
}

#ifdef EXPORT_LIBS

#ifdef _WIN32
#define LIB_EXPORT extern "C" __declspec(dllexport)
#else
#define LIB_EXPORT extern "C"
#endif

int ZipUnpackFile(const void *path, const void *output_path, bool full_path)
{
    return ZipUnpack(path, output_path, full_path, false);
}

#endif
