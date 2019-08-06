/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2019 Pawel Kolodziejski
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

#define PACKAGE "MEM"
#define PACKAGE_VERSION ""

#include <bfd.h>

void BacktraceGetFilename(char *dst, const char *src, int maxLen)
{
    long offset = 0;
    for (auto *ptr = src; *ptr != 0 && (ptr - src < maxLen); ptr++)
    {
        if (*ptr == '/' || *ptr == '\\')
            offset = ptr - src + 1;
    }
    strncpy(dst, src + offset, maxLen - 1);
}

int BacktraceGetInfoFromModule(char *moduleFilePath, unsigned long long offset, char *sourceFile,
    char *sourceFunc, unsigned int *sourceLine)
{
    bfd *bfdHandle = bfd_openr(moduleFilePath, nullptr);
    if (bfdHandle == nullptr)
        return -1;

    if (!bfd_check_format(bfdHandle, bfd_object))
    {
        bfd_close(bfdHandle);
        return -1;
    }

    if ((bfd_get_file_flags(bfdHandle) & HAS_SYMS) != HAS_SYMS)
    {
        bfd_close(bfdHandle);
        return -1;
    }

    bfd_symbol *symbolsTable;
    unsigned int unused;
    long int numberSymbols = bfd_read_minisymbols(bfdHandle, FALSE,
                                                  reinterpret_cast<void **>(&symbolsTable), &unused);
    if (numberSymbols == 0)
    {
        bfd_close(bfdHandle);
        return -1;
    }

    asection *section = bfdHandle->sections;
    while (section != nullptr)
    {
        if ((bfd_get_section_flags(bfdHandle, section) & SEC_ALLOC) == SEC_ALLOC)
        {
            bfd_vma address = bfd_get_section_vma(bfdHandle, section);
            bfd_size_type sectionSize = bfd_get_section_size(section);
            if (offset >= address && offset < address + sectionSize)
            {
                const char *filename = nullptr, *function = nullptr;
                if (bfd_find_nearest_line(bfdHandle, section, reinterpret_cast<bfd_symbol **>(symbolsTable),
                    offset - address, &filename, &function, sourceLine))
                {
                    if (filename)
                        strcpy(sourceFile, filename);
                    if (function)
                        strcpy(sourceFunc, function);
                    bfd_close(bfdHandle);
                    return 0;
                }
            }
        }
        section = section->next;
    }

    bfd_close(bfdHandle);

    return -1;
}
