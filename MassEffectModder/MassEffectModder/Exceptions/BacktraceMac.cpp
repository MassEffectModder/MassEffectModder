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

#define MAX_CALLSTACK 100

bool GetBackTrace(std::string &output, bool exceptionMode, bool crashMode)
{
    void *callstack[MAX_CALLSTACK];
    int offset, status, count = 0;

    int numberTraces = backtrace(callstack, MAX_CALLSTACK);
    char **strings = backtrace_symbols(callstack, numberTraces);

    if (strings == nullptr)
        return false;

    for (int i = 0; i < numberTraces; ++i)
    {
        if (strings[i] == nullptr)
            continue;

        char address[strlen(strings[i]) + 1];
        char sourceFunc[strlen(strings[i]) + 1];
        char moduleName[strlen(strings[i]) + 1];
        std::sscanf(strings[i], "%*s %s %s %s %*s %d",
                    moduleName, address, sourceFunc, &offset);
        if (crashMode && i <= 1)
            continue;
        if (exceptionMode && i <= 0)
            continue;
        if (!crashMode && !exceptionMode && i <= 0)
            continue;
        if (strcmp(sourceFunc, "start") == 0)
            continue;
        if (strcmp(moduleName, "") == 0)
            continue;

        output += std::to_string(count) + "  " + address + " " + moduleName + " in ";
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

        output += " offset " + std::to_string(offset) + "\n";
        count++;
    }
    free(strings);

    return true;
}
