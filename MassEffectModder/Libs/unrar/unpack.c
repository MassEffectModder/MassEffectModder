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

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <errno.h>
#endif

#include "dmc_unrar.c"

const char *get_filename(dmc_unrar_archive *archive, size_t i) {
    size_t size = dmc_unrar_get_filename(archive, i, 0, 0);
    if (!size)
        return NULL;

    char *filename = (char *)malloc(size);
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
    if (!filename)
        return 0;

    char *p = strrchr(filename, '/');
    if (!p)
        return filename;

    if (p[1] == '\0')
        return 0;

    return p + 1;
}

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
#else
                char tmpfile[strlen(name) + 1];
                strcpy(tmpfile, name);
                for (int j = 0; tmpfile[j] != 0; j++) {
                    if (tmpfile[j] == '/') {
                        tmpfile[j] = 0;
                        char full_file_path[strlen(output_path) + strlen(name) + 2];
                        if (output_path[0] != 0)
                            sprintf(full_file_path, "%s/%s", output_path, tmpfile);
                        else
                            strcpy(full_file_path, tmpfile);

                        struct stat s;
                        memset(&s, 0, sizeof(stat));
                        int error = stat(full_file_path, &s);
                        if (error == -1 && errno != ENOENT) {
                            fprintf(stderr, "Error: failed to check directory: %s\n", full_file_path);
                            status = 1;
                            break;
                        }
                        if (error == 0 && !S_ISDIR(s.st_mode)) {
                            fprintf(stderr, "Error: output path is not directory: %s\n", full_file_path);
                            status = 1;
                            break;
                        }
                        if (error == -1 && mkdir(full_file_path, 0755) != 0) {
                            fprintf(stderr, "Error: failed to create directory: %s\n", full_file_path);
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
#else
                char filename_path[strlen(output_path) + strlen(filename) + 2];
                if (output_path[0] != 0)
                    sprintf(filename_path, "%s/%s", output_path, filename);
                else
                    strcpy(filename_path, filename);
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

    printf("\nEverything is Ok\n");

    return status;
}
