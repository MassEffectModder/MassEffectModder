/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2020 Pawel Kolodziejski
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

#include "Wrappers.h"

#define MAX_CALLSTACK 100
#define PATH_MAX      1024

static intptr_t GetLoadAddressShift()
{
    char path[PATH_MAX];
    uint32_t maxLen = PATH_MAX;

    if (_NSGetExecutablePath(path, &maxLen) != 0)
        return 0;

    for (uint32_t i = 0; i < _dyld_image_count(); i++)
    {
        if (strcmp(_dyld_get_image_name(i), path) == 0)
            return _dyld_get_image_vmaddr_slide(i);
    }

    return 0;
}

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
    unsigned long long offset;
    char moduleFilePath[PATH_MAX];
    char sourceFile[PATH_MAX], sourceFunc[PATH_MAX];

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
        std::sscanf(strings[i], "%*d %s %s %s %*s %*d",
                    moduleName, address, sourceFunction);
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

        offset = strtoull(address, nullptr, 16);
        offset -= GetLoadAddressShift();
        unsigned int sourceLine = 0;
        sourceFile[0] = 0;
        status = BacktraceGetInfoFromModule(moduleFilePath, offset,
                                   sourceFile, sourceFunc, &sourceLine);
        if (status == 0)
        {
            BacktraceGetFilename(sourceFile, sourceFile, PATH_MAX - 1);
        }
        output += std::to_string(count + 1) + ".  ";
        char *funcNewName = abi::__cxa_demangle(sourceFunction, nullptr, nullptr, &status);
        if (status == 0)
        {
            output += funcNewName;
            free(funcNewName);
        }
        else
        {
            output += std::string(sourceFunction) + "()";
        }

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
