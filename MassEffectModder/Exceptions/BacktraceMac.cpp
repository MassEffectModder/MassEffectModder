/*
 * MassEffectModder
 *
 * Copyright (C) 2017 Pawel Kolodziejski <aquadran at users.sourceforge.net>
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

#define MAX_CALLSTACK 100

bool GetBackTrace(std::string &output, bool crashMode = true)
{
    void *callstack[MAX_CALLSTACK];
    char moduleName[1024], address[50], sourceFunc[1024];
    int offset, status, count = 0;

    int numberTraces = backtrace(callstack, MAX_CALLSTACK);
    char **strings = backtrace_symbols(callstack, numberTraces);

    for (int i = 0; i < numberTraces; ++i)
    {
        sscanf(strings[i], "%*s %s %s %s %*s %d",
               const_cast<char *>(moduleName), const_cast<char *>(address), const_cast<char *>(sourceFunc), &offset);
        if (crashMode && i <= 2)
            continue;
        if (!crashMode && i <= 0)
            continue;
        if (strcmp(sourceFunc, "start") == 0)
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
            output += std::string(const_cast<char *>(sourceFunc)) + "()";
        }

        output += " offset " + std::to_string(offset) + "\n";
        count++;
    }
    free(strings);

    return true;
}
