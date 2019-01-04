/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2019 Pawel Kolodziejski <aquadran at users.sourceforge.net>
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

#include <string>
#include <execinfo.h>
#include <cxxabi.h>

using namespace std;

static void getFilename(char *dst, const char *src)
{
    long offset = 0;
    for (auto *ptr = src; *ptr != 0; ptr++)
    {
        if (*ptr == '/' || *ptr == '\\')
            offset = ptr - src + 1;
    }
    strncpy(static_cast<char *>(dst), src + offset, 1024 - 1);
}

#define MAX_CALLSTACK 100

bool GetBackTrace(std::string &output, bool crashMode = true)
{
    void *callstack[MAX_CALLSTACK];
    char moduleName[1024], address[50], offset[50], sourceFunc[1024];
    int status, count = 0;

    int numberTraces = backtrace(static_cast<void **>(callstack), MAX_CALLSTACK);
    char **strings = backtrace_symbols(static_cast<void **>(callstack), numberTraces);

    for (int i = 0; i < numberTraces; ++i)
    {
        strcpy(static_cast<char *>(moduleName), "???");
        strcpy(static_cast<char *>(sourceFunc), "???");
        strcpy(static_cast<char *>(address), "???");
        strcpy(static_cast<char *>(offset), "???");

        char part1[1024];
        sscanf(strings[i], "%s %s", static_cast<char *>(part1), static_cast<char *>(address));
        char *start = strstr(static_cast<char *>(part1), "(");
        if (start)
        {
            long pos = start - static_cast<char *>(part1);
            part1[pos++] = 0;
            getFilename(static_cast<char *>(moduleName), static_cast<char *>(part1));
            start += 1;
            start = strstr(start, "+");
            if (start)
            {
                *start = '\0';
                start++;
                strcpy(static_cast<char *>(sourceFunc), static_cast<char *>(part1) + pos);
                pos = start - static_cast<char *>(part1);
                start = strstr(start, ")");
                if (start)
                {
                    *start = '\0';
                    strcpy(static_cast<char *>(offset), static_cast<char *>(part1) + pos);
                }
            }
        }
        if (address[0] == '[')
        {
            for (auto *ptr = static_cast<char *>(address); *ptr != 0; ptr++)
            {
                *ptr = ptr[1];
                if (*ptr == ']')
                {
                    *ptr = '\0';
                    break;
                }
            }
        }
        if (crashMode && i <= 1)
            continue;
        if (!crashMode && i <= 0)
            continue;
        if (strcmp(static_cast<char *>(sourceFunc), "_start") == 0 ||
            strcmp(static_cast<char *>(sourceFunc), "__libc_start_main") == 0)
            continue;

        output += "#" + std::to_string(count) + "  " + static_cast<char *>(address) + " " + static_cast<char *>(moduleName) + " in ";
        char *funcNewName = abi::__cxa_demangle(static_cast<char *>(sourceFunc), nullptr, nullptr, &status);
        if (status == 0)
        {
            output += funcNewName;
            free(funcNewName);
        }
        else
        {
            output += std::string(static_cast<char *>(sourceFunc)) + "()";
        }

        output += " offset " + std::string(static_cast<char *>(offset)) + "\n";
        count++;
    }
    free(strings);

    return true;
}
