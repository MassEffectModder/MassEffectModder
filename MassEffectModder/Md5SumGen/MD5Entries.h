/*
 * MassEffectModder
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

#ifndef MD5_ENTRIES_H
#define MD5_ENTRIES_H

struct MD5FileEntry
{
    const char *path;
    const unsigned char md5[16];
    const int size;
};

extern const MD5FileEntry entriesME1[];
extern const MD5FileEntry entriesME1PL[];
extern const MD5FileEntry entriesME2[];
extern const MD5FileEntry entriesME3[];
extern const int MD5EntriesME1Size;
extern const int MD5EntriesME1PLSize;
extern const int MD5EntriesME2Size;
extern const int MD5EntriesME3Size;

#endif
