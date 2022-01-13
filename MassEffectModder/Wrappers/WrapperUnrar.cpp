/*
 * Wrapper Unrar
 *
 * Copyright (C) 2019-2022 Pawel Kolodziejski
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
 */

extern "C" {
#if defined(_WIN32)
int unrar_unpack(const wchar_t *path, const wchar_t *output_path,
                 const wchar_t *filter, int full_path, int ipc);
int unrar_list(const wchar_t *path, int ipc);
#else
int unrar_unpack(const char *path, const char *output_path,
                 const char *filter, int full_path, int ipc);
int unrar_list(const char *path, int ipc);
#endif
}

int RarUnpack(const void *path, const void *output_path, const void *filter, bool full_path, bool ipc)
{
#if defined(_WIN32)
    return unrar_unpack(static_cast<const wchar_t *>(path),
                        static_cast<const wchar_t *>(output_path),
                        static_cast<const wchar_t *>(filter),
                        (int)full_path, (int)ipc);
#else
    return unrar_unpack(static_cast<const char *>(path),
                        static_cast<const char *>(output_path),
                        static_cast<const char *>(filter),
                        (int)full_path, (int)ipc);
#endif
}

int RarList(const void *path, bool ipc)
{
#if defined(_WIN32)
    return unrar_list(static_cast<const wchar_t *>(path), (int)ipc);
#else
    return unrar_list(static_cast<const char *>(path), (int)ipc);
#endif
}
