/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2018 Pawel Kolodziejski <aquadran at users.sourceforge.net>
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
    for (char *ptr = const_cast<char *>(src); *ptr != 0; ptr++)
    {
        if (*ptr == '/' || *ptr == '\\')
            offset = ptr - src + 1;
    }
    strncpy(dst, src + offset, 1024);
}

#define MAX_CALLSTACK 100

bool GetBackTrace(std::string &output, bool crashMode = true)
{
    void *callstack[MAX_CALLSTACK];
    char moduleName[1024], address[50], offset[50], sourceFunc[1024];
    int status, count = 0;

    int numberTraces = backtrace(callstack, MAX_CALLSTACK);
    char **strings = backtrace_symbols(callstack, numberTraces);

    for (int i = 0; i < numberTraces; ++i)
    {
        strcpy(moduleName, "???");
        strcpy(sourceFunc, "???");
        strcpy(address, "???");
        strcpy(offset, "???");

        char part1[1024];
        sscanf(strings[i], "%s %s", const_cast<char *>(part1), const_cast<char *>(address));
        char *start = strstr(part1, "(");
        if (start)
        {
            long pos = start - part1;
            part1[pos++] = 0;
            getFilename(moduleName, part1);
            start += 1;
            start = strstr(start, "+");
            if (start)
            {
                *start = '\0';
                start += 1;
                strcpy(sourceFunc, part1 + pos);
                pos = start - part1;
                start = strstr(start, ")");
                if (start)
                {
                    *start = '\0';
                    strcpy(offset, part1 + pos);
                }
            }
        }
        if (address[0] == '[')
        {
            for (char *ptr = const_cast<char *>(address); *ptr != 0; ptr++)
            {
                *ptr = *(ptr + 1);
                if (*ptr == ']')
                {
                    *ptr = '\0';
                    break;
                }
            }
        }
        if (crashMode && i <= 2)
            continue;
        if (!crashMode && i <= 0)
            continue;
        if (strcmp(sourceFunc, "_start") == 0 ||
            strcmp(sourceFunc, "__libc_start_main") == 0)
            continue;

        output += "#" + std::to_string(count) + "  " + address + " " + moduleName + " in ";
        char *funcNewName = abi::__cxa_demangle(sourceFunc, nullptr, nullptr, &status);
        if (status == 0)
        {
            output += funcNewName;
            free(funcNewName);
        }
        else
        {
            output += std::string(sourceFunc) + "()";
        }

        output += " offset " + std::string(offset) + "\n";
        count++;
    }
    free(strings);

    return true;
}
