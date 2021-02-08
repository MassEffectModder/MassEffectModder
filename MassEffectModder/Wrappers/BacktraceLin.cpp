/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2021 Pawel Kolodziejski
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
#include <string>
#include <execinfo.h>
#include <cxxabi.h>

#include "Wrappers.h"

using namespace std;

#define MAX_PATH 1024
#define MAX_CALLSTACK 100

bool GetBackTrace(std::string &output, bool exceptionMode, bool crashMode)
{
    void *callstack[MAX_CALLSTACK];
    unsigned long long symbolOffset;
    int status, count = 0;
    char sourceFile[MAX_PATH + 1];

    int numberTraces = backtrace(callstack, MAX_CALLSTACK);
    char **strings = backtrace_symbols(callstack, numberTraces);

    for (int i = 0; i < numberTraces; ++i)
    {
        char address[strlen(strings[i]) + 1];
        char offset[strlen(strings[i]) + 1];
        char moduleFilePath[strlen(strings[i]) + 1];
        char sourceFunc[strlen(strings[i]) + 1];
        sourceFunc[0] = 0;
        address[0] = 0;

        sscanf(strings[i], "%s %s", moduleFilePath, address);
        char *start = strstr(moduleFilePath, "(");
        if (start)
        {
            long pos = start - moduleFilePath;
            moduleFilePath[pos++] = 0;
            start = strstr(++start, "+");
            if (start)
            {
                *start = '\0';
                start++;
                strcpy(sourceFunc, moduleFilePath + pos);
                pos = start - moduleFilePath;
                start = strstr(start, ")");
                if (start)
                {
                    *start = '\0';
                    strcpy(offset, moduleFilePath + pos);
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
            moduleFilePath[0] == 0 ||
            address[0] == 0)
        {
            continue;
        }

        symbolOffset = strtoull(address, nullptr, 16);
        unsigned int sourceLine = 0;
        sourceFile[0] = 0;
        sourceFunc[0] = 0;
        status = BacktraceGetInfoFromModule(moduleFilePath, symbolOffset,
                                   sourceFile, sourceFunc, &sourceLine);
        if (status != 0)
        {
            symbolOffset = strtoull(offset, nullptr, 16);
            status = BacktraceGetInfoFromModule(moduleFilePath, symbolOffset,
                                       sourceFile, sourceFunc, &sourceLine);
            if (status != 0)
            {
                continue;
            }
        }
        if (strcmp(sourceFunc, "_start") == 0 ||
            strcmp(sourceFunc, "__libc_start_main") == 0)
        {
            continue;
        }
        output += std::to_string(count + 1) + ".  ";
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

        BacktraceGetFilename(moduleFilePath, sourceFile, strlen(strings[i]) + 1);
        strcpy(sourceFile, moduleFilePath);
        if (sourceFile[0] != 0)
            output += " at " + std::string(sourceFile);
        if (sourceLine != 0)
            output += ": line " + std::to_string(sourceLine);

        output += "\n";
        count++;
    }
    free(strings);

    return true;
}
