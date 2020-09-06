/* dmc_unrar - A dependency-free, single-file FLOSS unrar library
 *
 * Copyright (c) 2019-2020 by Pawel Kolodziejski
 * Copyright (c) 2017 by Sven Hesse (DrMcCoy) <drmccoy@drmccoy.de>
 *
 * dmc_unrar is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 2 of
 * the License, or (at your option) any later version.
 *
 * dmc_unrar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * For the full text of the GNU General Public License version 2,
 * see <https://www.gnu.org/licenses/gpl-2.0.html>
 */

#if defined(_WIN32)
#include <windows.h>
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>

#ifdef _WIN32
#include <direct.h>
#include <sys/stat.h>
#include <wchar.h>
#include <errno.h>
#else
#include <sys/stat.h>
#include <errno.h>
#endif

#include "dmc_unrar.c"

#if defined(_WIN32)
const wchar_t *get_filename_unicode(dmc_unrar_archive *archive, size_t i) {
    int buf_size;
    char *bufName = 0;
    int result;
    size_t size = dmc_unrar_get_filename(archive, i, NULL, 0);
    if (!size)
        return NULL;

    bufName = (char *)malloc(size);
    if (!bufName)
        return NULL;

    size = dmc_unrar_get_filename(archive, i, bufName, size);
    if (!size) {
        free(bufName);
        return NULL;
    }

    dmc_unrar_unicode_make_valid_utf8(bufName);
    if (bufName[0] == '\0') {
        free(bufName);
        return NULL;
    }

    wchar_t *filename;
    /* Calculate the buffer size needed */
    buf_size = MultiByteToWideChar(CP_UTF8, 0, bufName, -1, NULL, 0);
    if (buf_size == 0) {
        free(bufName);
        return NULL;
    }

    /* Allocate that size in the buffer */
    filename = DMC_UNRAR_MALLOC(buf_size * sizeof(wchar_t));
    if (filename == NULL) {
        free(bufName);
        return NULL;
    }

    /* Actually convert the data now */
    result = MultiByteToWideChar(CP_UTF8, 0, bufName, -1, filename, buf_size);
    if (result == 0) {
        free(bufName);
        free(filename);
        return NULL;
    }

    free(bufName);

    return filename;
}

static int MyCreateDir(const wchar_t *name)
{
    errno_t error = _waccess_s(name, 0);
    if (error != 0 && errno != ENOENT) {
        fwprintf(stderr, L"Error: failed to check directory: %ls\n", name);
        return 1;
    }
    struct _stat s;
    memset(&s, 0, sizeof(struct _stat));
    _wstat(name, &s);
    if (error == 0 && !S_ISDIR(s.st_mode)) {
        fwprintf(stderr, L"Error: output path is not directory: %ls\n", name);
        return 1;
    }
    if (error != 0 && !CreateDirectoryW(name, NULL)) {
        fwprintf(stderr, L"Error: failed to create directory: %ls\n", name);
        return 1;
    }
    return 0;
}

#else

const char *get_filename_utf8(dmc_unrar_archive *archive, size_t i) {
    char *filename = 0;
    size_t size = dmc_unrar_get_filename(archive, i, NULL, 0);
    if (!size)
        return NULL;

    filename = (char *)malloc(size);
    if (!filename)
        return NULL;

    size = dmc_unrar_get_filename(archive, i, filename, size);
    if (!size) {
        free(filename);
        return NULL;
    }

    dmc_unrar_unicode_make_valid_utf8(filename);
    if (filename[0] == '\0') {
        free(filename);
        return NULL;
    }

    return filename;
}

static int MyCreateDir(const char *name)
{
    struct stat s;
    memset(&s, 0, sizeof(stat));
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

#if defined(_WIN32)
static int compareExt(wchar_t *filename, const wchar_t *ext)
{
    wchar_t *pExt = wcsrchr(filename, L'.');
    if (pExt == NULL)
        return 0;
    return _wcsicmp(&pExt[1], ext) == 0;
}
#else
static int compareExt(char *filename, const char *ext)
{
    char *pExt = strrchr(filename, '.');
    if (pExt == NULL)
        return 0;
    return strcasecmp(&pExt[1], ext) == 0;
}
#endif

static int g_ipc;
static int lastProgress;
static uint64_t progressUnpackedSize;
static uint64_t totalUnpackedSize;

static void PrintProgressIpc(uint64_t processedBytes)
{
    if (g_ipc)
    {
        progressUnpackedSize += processedBytes;
        int newProgress = progressUnpackedSize * 100 / totalUnpackedSize;
        if (lastProgress != newProgress)
        {
            lastProgress = newProgress;
#if defined(_WIN32)
            wprintf(L"[IPC]TASK_PROGRESS %d\n", newProgress);
#else
            printf("[IPC]TASK_PROGRESS %d\n", newProgress);
#endif
            fflush(stdout);
        }
    }
}

#if defined(_WIN32)
int unrar_unpack(const wchar_t *path, const wchar_t *output_path,
                 const wchar_t *filter, int full_path, int ipc) {
#else
int unrar_unpack(const char *path, const char *output_path,
                 const char *filter, int full_path, int ipc) {
#endif
    int status = 0;

    g_ipc = ipc;
    lastProgress = -1;
    totalUnpackedSize = 0;

    if (!dmc_unrar_is_rar_path((char *)path))
        return 1;

    dmc_unrar_archive archive;
    dmc_unrar_return return_code;

    return_code = dmc_unrar_archive_init(&archive);
    if (return_code != DMC_UNRAR_OK) {
#if defined(_WIN32)
        fwprintf(stderr, L"Error: Unrar init failed: %s\n", dmc_unrar_strerror(return_code));
#else
        fprintf(stderr, "Error: Unrar init failed: %s\n", dmc_unrar_strerror(return_code));
#endif
        return 1;
    }

    return_code = dmc_unrar_archive_open_path(&archive, (char *)path);
    if (return_code != DMC_UNRAR_OK) {
#if defined(_WIN32)
        fwprintf(stderr, L"Error: Failed open archive: %s\n", dmc_unrar_strerror(return_code));
#else
        fprintf(stderr, "Error: Failed open archive: %s\n", dmc_unrar_strerror(return_code));
#endif
        return 1;
    }

    for (size_t i = 0; i < dmc_unrar_get_file_count(&archive); i++) {
        const dmc_unrar_file *file = dmc_unrar_get_file_stat(&archive, i);
        totalUnpackedSize += file->uncompressed_size;
    }

    for (size_t i = 0; i < dmc_unrar_get_file_count(&archive); i++) {
#if defined(_WIN32)
        wchar_t *fileName = (wchar_t *)get_filename_unicode(&archive, i);
#else
        char *fileName = (char *)get_filename_utf8(&archive, i);
#endif
        const dmc_unrar_file *file = dmc_unrar_get_file_stat(&archive, i);

        if (!fileName) {
#if defined(_WIN32)
            fwprintf(stderr, L"%lu of %zu - Failed to get name from archive, aborting\n", (i + 1), dmc_unrar_get_file_count(&archive));
#else
            fprintf(stderr, "%lu of %zu - Failed to get name from archive, aborting\n", (i + 1), dmc_unrar_get_file_count(&archive));
#endif
            continue;
        }

        if (dmc_unrar_file_is_directory(&archive, i)) {
            if (!ipc) {
#if defined(_WIN32)
                wprintf(L"%lu of %zu - %ls - Ok\n", (i + 1), dmc_unrar_get_file_count(&archive), fileName);
#else
                printf("%lu of %zu - %s - Ok\n", (i + 1), dmc_unrar_get_file_count(&archive), fileName);
#endif
            }
            continue;
        }

        if (filter[0] != 0 && !compareExt(fileName, filter)) {
            totalUnpackedSize -= file->uncompressed_size;
            continue;
        }

#if defined(_WIN32)
        const wchar_t *outputDir = output_path;

        if (ipc)
        {
            wprintf(L"[IPC]FILENAME %ls\n", fileName);
            fflush(stdout);
        }
        else
        {
            wprintf(L"%lu of %zu - %ls - size %llu - ", (i + 1), dmc_unrar_get_file_count(&archive), fileName, (unsigned long long)file->uncompressed_size);
        }

        int size = wcslen(fileName) + 1;
        if (size > MAX_PATH) {
            fwprintf(stderr, L"Error: File name too long, aborting!\n");
            status = 1;
            free((void *)fileName);
            break;
        }

        int dest_size = wcslen(outputDir) + size + 1;
        if (dest_size > MAX_PATH) {
            fwprintf(stderr, L"Error: Destination path for file too long, aborting!\n");
            free((void *)fileName);
            status = 1;
            break;
        }

        wchar_t outputFile[MAX_PATH];
        if (outputDir && outputDir[0] != 0)
            swprintf(outputFile, PATH_MAX - 1, L"%ls/%ls", outputDir, fileName);
        else
            wcsncpy(outputFile, fileName, PATH_MAX - 1);

        wchar_t tmpPath[PATH_MAX];
        wcsncpy(tmpPath, fileName, PATH_MAX - 1);
        for (int j = 0; tmpPath[j] != 0; j++)
        {
            if (tmpPath[j] == '/')
            {
                if (full_path)
                {
                    tmpPath[j] = 0;
                    wchar_t outputPath[PATH_MAX];
                    if (outputDir && outputDir[0] != 0)
                        swprintf(outputPath, PATH_MAX - 1, L"%ls/%ls", outputDir, tmpPath);
                    else
                        wcsncpy(outputPath, tmpPath, PATH_MAX - 1);

                    if (MyCreateDir(outputPath) != 0)
                    {
                        status = 1;
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
#else
        const char *outputDir = output_path;

        if (ipc)
        {
            printf("[IPC]FILENAME %s\n", fileName);
            fflush(stdout);
        }
        else
        {
            printf("%lu of %zu - %s - size %llu - ", (i + 1), dmc_unrar_get_file_count(&archive), fileName, (unsigned long long)file->uncompressed_size);
        }

        char outputFile[PATH_MAX];
        if (outputDir && outputDir[0] != 0)
            snprintf(outputFile, PATH_MAX - 1, "%s/%s", outputDir, fileName);
        else
            strncpy(outputFile, fileName, PATH_MAX - 1);

        char tmpPath[PATH_MAX];
        strncpy(tmpPath, fileName, PATH_MAX - 1);
        for (int j = 0; tmpPath[j] != 0; j++)
        {
            if ((tmpPath[j] & 0xc0) == 0x80)
            {
                continue;
            }
            if (tmpPath[j] == '/') {
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
                        status = 1;
                        break;
                    }
                    tmpPath[j] = '/';
                } else {
                    if (outputDir && outputDir[0] != 0)
                        snprintf(outputFile, PATH_MAX - 1, "%s/%s", outputDir, tmpPath + j + 1);
                    else
                        strncpy(outputFile, tmpPath + j + 1, PATH_MAX - 1);
                }
            }
        }
#endif
        if (status == 0) {
            dmc_unrar_return supported = dmc_unrar_file_is_supported(&archive, i);
            if (supported == DMC_UNRAR_OK) {
                dmc_unrar_return extracted = dmc_unrar_extract_file_to_path(&archive, i, (char *)outputFile, NULL, true, PrintProgressIpc);
                if (extracted != DMC_UNRAR_OK) {
#if defined(_WIN32)
                    fwprintf(stderr, L"Error: %s\n", dmc_unrar_strerror(extracted));
#else
                    fprintf(stderr, "Error: %s\n", dmc_unrar_strerror(extracted));
#endif
                    status = 1;
                    break;
                } else {
                    if (!ipc) {
#if defined(_WIN32)
                        wprintf(L"Ok\n");
#else
                        printf("Ok\n");
#endif
                    }
                }
            } else {
#if defined(_WIN32)
                fwprintf(stderr, L"Not supported: %s\n", dmc_unrar_strerror(supported));
#else
                fprintf(stderr, "Not supported: %s\n", dmc_unrar_strerror(supported));
#endif
                status = 1;
                break;
            }
        }

        free((void *)fileName);
    }

    dmc_unrar_archive_close(&archive);

    return status;
}

#if defined(_WIN32)
int unrar_list(const wchar_t *path, int ipc) {
#else
int unrar_list(const char *path, int ipc) {
#endif
    int status = 0;

    g_ipc = ipc;
    lastProgress = -1;
    totalUnpackedSize = 0;

    if (!dmc_unrar_is_rar_path((char *)path))
        return 1;

    dmc_unrar_archive archive;
    dmc_unrar_return return_code;

    return_code = dmc_unrar_archive_init(&archive);
    if (return_code != DMC_UNRAR_OK) {
#if defined(_WIN32)
        fwprintf(stderr, L"Error: Unrar init failed: %s\n", dmc_unrar_strerror(return_code));
#else
        fprintf(stderr, "Error: Unrar init failed: %s\n", dmc_unrar_strerror(return_code));
#endif
        return 1;
    }

    return_code = dmc_unrar_archive_open_path(&archive, (char *)path);
    if (return_code != DMC_UNRAR_OK) {
#if defined(_WIN32)
        fwprintf(stderr, L"Error: Failed open archive: %s\n", dmc_unrar_strerror(return_code));
#else
        fprintf(stderr, "Error: Failed open archive: %s\n", dmc_unrar_strerror(return_code));
#endif
        return 1;
    }

    for (size_t i = 0; i < dmc_unrar_get_file_count(&archive); i++) {
#if defined(_WIN32)
        const wchar_t *fileName = get_filename_unicode(&archive, i);
#else
        const char *fileName = get_filename_utf8(&archive, i);
#endif
        if (!fileName) {
#if defined(_WIN32)
            fwprintf(stderr, L"Failed to get name from archive, aborting\n");
#else
            fprintf(stderr, "Failed to get name from archive, aborting\n");
#endif
            continue;
        }

        if (dmc_unrar_file_is_directory(&archive, i)) {
            continue;
        }

#if defined(_WIN32)
        if (ipc)
        {
            wprintf(L"[IPC]FILENAME %ls\n", fileName);
            fflush(stdout);
        }
        else
        {
            wprintf(L"%ls\n", fileName);
        }

#else
        if (ipc)
        {
            printf("[IPC]FILENAME %s\n", fileName);
            fflush(stdout);
        }
        else
        {
            printf("%s\n", fileName);
        }
#endif

        free((void *)fileName);
    }

    dmc_unrar_archive_close(&archive);

    return status;
}
