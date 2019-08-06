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

#include <string>
#include <execinfo.h>
#include <cxxabi.h>
#include <mach-o/dyld.h>

#include "Exceptions/Backtrace.h"

#define MAX_CALLSTACK 100

static void getExecutablePath(char *path, uint32_t maxLen)
{
    if (_NSGetExecutablePath(path, &maxLen) != 0) {
        path[0] = '\0';
    } else {
        char *fullPath = realpath(path, nullptr);
        if (fullPath != nullptr) {
            strncpy(path, fullPath, maxLen);
            free(fullPath);
        }
    }
}

bool GetBackTrace(std::string &output, bool exceptionMode, bool crashMode)
{
    void *callstack[MAX_CALLSTACK];
    int status, count = 0;
    //unsigned long long offset;
    int offset;
    char moduleFilePath[PATH_MAX];
    char /*sourceFile[PATH_MAX], */sourceFunc[PATH_MAX];

    int numberOfTraces = backtrace(callstack, MAX_CALLSTACK);
    char **strings = backtrace_symbols(callstack, numberOfTraces);

    if (strings == nullptr)
        return false;

    getExecutablePath(moduleFilePath, PATH_MAX - 1);
    if (moduleFilePath[0] == 0)
        return false;

    for (int i = 0; i < numberOfTraces; ++i)
    {
        if (strings[i] == nullptr)
            continue;

        char address[strlen(strings[i]) + 1];
        char sourceFunction[strlen(strings[i]) + 1];
        char moduleName[strlen(strings[i]) + 1];
        std::sscanf(strings[i], "%*d %s %s %s %*s %d",
                    moduleName, address, sourceFunction, &offset);
        if (crashMode && i <= 2)
            continue;
        if (exceptionMode && i <= 1)
            continue;
        if (!crashMode && !exceptionMode && i <= 0)
            continue;
        if (strcmp(sourceFunction, "start") == 0 ||
            strcmp(moduleName, "") == 0 ||
            strcmp(moduleName, "???") == 0)
        {
            continue;
        }

        //offset = strtoull(address, nullptr, 16);
        //unsigned int sourceLine = 0;
        //status = BacktraceGetInfoFromModule(moduleFilePath, offset,
        //                           sourceFile, sourceFunc, &sourceLine);
        output += std::to_string(count) + "  ";
        char *funcNewName = abi::__cxa_demangle(sourceFunction, nullptr, nullptr, &status);
        if (status == 0)
        {
            output += funcNewName;
            free(funcNewName);
        }
        else
        {
            output += std::string(sourceFunc) + "()";
        }

        output += " offset " + std::to_string(offset) + "\n";
        //output += " at " + std::string(sourceFile) + ":" + std::to_string(sourceLine) + "\n";
        count++;
    }
    free(strings);

    return true;
}
