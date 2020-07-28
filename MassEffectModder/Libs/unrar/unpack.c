/* dmc_unrar - A dependency-free, single-file FLOSS unrar library
 *
 * Copyright (c) 2019 by Pawel Kolodziejski
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

const char *get_filename(dmc_unrar_archive *archive, size_t i) {
    char *filename = 0;
    size_t size = dmc_unrar_get_filename(archive, i, 0, 0);
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

const char *get_filename_no_directory(const char *filename) {
    char *p = 0;
    if (!filename)
        return 0;

    p = strrchr(filename, '/');
    if (!p)
        return filename;

    if (p[1] == '\0')
        return 0;

    return p + 1;
}

#if defined(_WIN32)
static int MyCreateDir(const wchar_t *name)
{
    errno_t error = _waccess_s(name, 0);
    if (error != 0 && errno != ENOENT) {
        fwprintf(stderr, L"Error: failed to check directory: %s\n", name);
        return 1;
    }
    struct _stat s;
    memset(&s, 0, sizeof(struct _stat));
    _wstat(name, &s);
    if (error == 0 && !S_ISDIR(s.st_mode)) {
        fwprintf(stderr, L"Error: output path is not directory: %s\n", name);
        return 1;
    }
    if (error != 0 && !CreateDirectoryW(name, NULL)) {
        fwprintf(stderr, L"Error: failed to create directory: %s\n", name);
        return 1;
    }
    return 0;
}
#else
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

    if (!dmc_unrar_is_rar_path(path))
        return 1;

    dmc_unrar_archive archive;
    dmc_unrar_return return_code;

    return_code = dmc_unrar_archive_init(&archive);
    if (return_code != DMC_UNRAR_OK) {
        fprintf(stderr, "Unrar init failed: %s\n", dmc_unrar_strerror(return_code));
        return 1;
    }

    return_code = dmc_unrar_archive_open_path(&archive, path);
    if (return_code != DMC_UNRAR_OK) {
        fprintf(stderr, "Unrar open failed: %s\n", dmc_unrar_strerror(return_code));
        return 1;
    }

    for (size_t i = 0; i < dmc_unrar_get_file_count(&archive); i++) {
        const char *name = get_filename(&archive, i);
        const dmc_unrar_file *file = dmc_unrar_get_file_stat(&archive, i);

        if (name && !dmc_unrar_file_is_directory(&archive, i)) {
            printf("\"%s\" - %u bytes\n", name, (unsigned int)file->uncompressed_size);
        }

        const char *filename = NULL;
        if (name) {
            if (full_path) {
#if defined(_WIN32)
                int size = strlen(name) + 1;
                wchar_t tmpfile[size];
                mbstowcs(tmpfile, name, size);
                for (int j = 0; tmpfile[j] != 0; j++) {
                    if (tmpfile[j] == '/' && tmpfile[1] != ':') {
                        tmpfile[j] = 0;
                        size = wcslen(output_path) + strlen(name) + 2;
                        wchar_t full_file_path[size];
                        if (output_path[0] != 0)
                            swprintf(full_file_path, size, L"%s/%s", output_path, tmpfile);
                        else
                            wcscpy(full_file_path, tmpfile);
                        if (MyCreateDir(full_file_path) != 0)
                        {
                            status = 1;
                            break;
                        }
                        tmpfile[j] = '/';
                    }
                }
                if (status == 0)
                    filename = name;
#else
                char tmpfile[PATH_MAX - 2];
                strncpy(tmpfile, name, sizeof (tmpfile) - 1);
                for (int j = 0; tmpfile[j] != 0; j++) {
                    if (tmpfile[j] == '/') {
                        tmpfile[j] = 0;
                        char full_file_path[PATH_MAX];
                        if (output_path[0] != 0)
                            snprintf(full_file_path, PATH_MAX, "%s/%s", output_path, tmpfile);
                        else
                            strncpy(full_file_path, tmpfile, sizeof (full_file_path));
                        if (MyCreateDir(full_file_path) != 0)
                        {
                            status = 1;
                            break;
                        }
                        tmpfile[j] = '/';
                    }
                }
                if (status == 0)
                    filename = name;
#endif
            }
            else
                filename = get_filename_no_directory(name);
        }

        if (filename && !dmc_unrar_file_is_directory(&archive, i)) {
            dmc_unrar_return supported = dmc_unrar_file_is_supported(&archive, i);
            if (supported == DMC_UNRAR_OK) {
#if defined(_WIN32)
                size_t size = strlen(filename) + 1;
                wchar_t tmpfile[size];
                mbstowcs(tmpfile, filename, size);
                size = wcslen(output_path) + wcslen(tmpfile) + 2;
                wchar_t filename_path[size];
                if (output_path[0] != 0)
                    swprintf(filename_path, size, L"%s/%s", output_path, tmpfile);
                else
                    mbstowcs(filename_path, filename, strlen(filename));
#else
                char filename_path[PATH_MAX];
                if (output_path[0] != 0)
                    snprintf(filename_path, sizeof (filename_path) - 2, "%s/%s", output_path, filename);
                else
                    strncpy(filename_path, filename, sizeof (filename_path) - 1);
#endif
                dmc_unrar_return extracted = dmc_unrar_extract_file_to_path(&archive, i, filename_path, NULL, true);
                if (extracted != DMC_UNRAR_OK) {
                    fprintf(stderr, "Error: %s\n", dmc_unrar_strerror(extracted));
                    status = 1;
                }
            } else {
                fprintf(stderr, "Not supported: %s\n", dmc_unrar_strerror(supported));
                status = 1;
            }
        }

        if (name)
            free((char *)name);
    }

    dmc_unrar_archive_close(&archive);

    if (status == 0)
        printf("\nEverything is Ok\n");

    return status;
}
