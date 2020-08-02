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
int unrar_unpack(const wchar_t *path, const wchar_t *output_path, int full_path) {
#else
int unrar_unpack(const char *path, const char *output_path, int full_path) {
#endif
    int status = 0;

    if (!dmc_unrar_is_rar_path((char *)path))
        return 1;

    dmc_unrar_archive archive;
    dmc_unrar_return return_code;

    return_code = dmc_unrar_archive_init(&archive);
    if (return_code != DMC_UNRAR_OK) {
        fprintf(stderr, "Unrar init failed: %s\n", dmc_unrar_strerror(return_code));
        return 1;
    }

    return_code = dmc_unrar_archive_open_path(&archive, (char *)path);
    if (return_code != DMC_UNRAR_OK) {
        fprintf(stderr, "Unrar open failed: %s\n", dmc_unrar_strerror(return_code));
        return 1;
    }

    for (size_t i = 0; i < dmc_unrar_get_file_count(&archive); i++) {
#if defined(_WIN32)
        const wchar_t *fileName = get_filename_unicode(&archive, i);
#else
        const char *fileName = get_filename_utf8(&archive, i);
#endif
        const dmc_unrar_file *file = dmc_unrar_get_file_stat(&archive, i);

        if (!fileName || dmc_unrar_file_is_directory(&archive, i)) {
            continue;
        }

#if defined(_WIN32)
        wprintf(L"\"%ls\" - %u bytes\n", fileName, (unsigned int)file->uncompressed_size);
#else
        printf("\"%s\" - %u bytes\n", fileName, (unsigned int)file->uncompressed_size);
#endif

#if defined(_WIN32)
        const wchar_t *outputDir = output_path;

        int size = wcslen(fileName) + 1;
        if (size > MAX_PATH) {
            free((void *)fileName);
            continue;
        }

        int dest_size = wcslen(outputDir) + size + 1;
        if (dest_size > MAX_PATH) {
            free((void *)fileName);
            continue;
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
        char outputFile[PATH_MAX];
        if (outputDir && outputDir[0] != 0)
            snprintf(outputFile, PATH_MAX - 1, "%s/%s", outputDir, fileName);
        else
            strncpy(outputFile, fileName, PATH_MAX - 1);

        char tmpPath[PATH_MAX];
        strncpy(tmpPath, fileName, PATH_MAX - 1);
        for (int j = 0; tmpPath[j] != 0; j++) {
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
                dmc_unrar_return extracted = dmc_unrar_extract_file_to_path(&archive, i, (char *)outputFile, NULL, true);
                if (extracted != DMC_UNRAR_OK) {
                    fprintf(stderr, "Error: %s\n", dmc_unrar_strerror(extracted));
                    status = 1;
                }
            } else {
                fprintf(stderr, "Not supported: %s\n", dmc_unrar_strerror(supported));
                status = 1;
            }
        }

        free((void *)fileName);
    }

    dmc_unrar_archive_close(&archive);

    if (status == 0)
        printf("\nEverything is Ok\n");

    return status;
}
