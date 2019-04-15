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

#include <cstring>
#include <execinfo.h>
#include <cxxabi.h>

using namespace std;

#define MAX_PATH 1024

static void getFilename(char *dst, const char *src)
{
    long offset = 0;
    for (auto *ptr = src; *ptr != 0 && (ptr - src < MAX_PATH); ptr++)
    {
        if (*ptr == '/' || *ptr == '\\')
            offset = ptr - src + 1;
    }
    strncpy(dst, src + offset, MAX_PATH - 1);
}

#define MAX_CALLSTACK 100

bool GetBackTrace(std::string &output, bool exceptionMode, bool crashMode)
{
    void *callstack[MAX_CALLSTACK];
    char moduleName[MAX_PATH];
    int status, count = 0;

    int numberTraces = backtrace(callstack, MAX_CALLSTACK);
    char **strings = backtrace_symbols(callstack, numberTraces);

    for (int i = 0; i < numberTraces; ++i)
    {
        char part1[strlen(strings[i]) + 1];
        char address[strlen(strings[i]) + 1];
        char offset[strlen(strings[i]) + 1];
        char sourceFunc[strlen(strings[i]) + 1];

        strcpy(moduleName, "???");
        strcpy(sourceFunc, "???");
        strcpy(address, "???");
        strcpy(offset, "???");

        sscanf(strings[i], "%s %s", part1, address);
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
                start++;
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
            for (auto *ptr = address; *ptr != 0; ptr++)
            {
                *ptr = ptr[1];
                if (*ptr == ']')
                {
                    *ptr = '\0';
                    break;
                }
            }
        }
        if (crashMode && i <= 2)
            continue;
        if (exceptionMode && i <= 1)
            continue;
        if (!crashMode && !exceptionMode && i <= 0)
            continue;
        if (strcmp(sourceFunc, "_start") == 0 ||
            strcmp(sourceFunc, "__libc_start_main") == 0 ||
            strcmp(sourceFunc, "???") == 0)
            continue;

        output += std::to_string(count) + "  ";
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
