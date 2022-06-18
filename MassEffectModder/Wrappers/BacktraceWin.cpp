/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2022 Pawel Kolodziejski
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

#include <windows.h>
#include <imagehlp.h>
#include <string>
#include <cxxabi.h>

#include "Wrappers.h"

bool GetBackTrace(std::string &output, bool exceptionMode, bool crashMode)
{
    char symbolInfo[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
    auto symbol = reinterpret_cast<PSYMBOL_INFO>(symbolInfo);
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
    symbol->MaxNameLen = MAX_SYM_NAME;

    CONTEXT context;
    memset(&context, 0, sizeof(CONTEXT));
    context.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext(&context);

    STACKFRAME stackFrame;
    memset(&stackFrame, 0, sizeof(STACKFRAME));
    stackFrame.AddrPC.Offset = context.Rip;
    stackFrame.AddrPC.Mode = AddrModeFlat;
    stackFrame.AddrFrame.Offset = context.Rsp;
    stackFrame.AddrFrame.Mode = AddrModeFlat;
    stackFrame.AddrStack.Offset = context.Rsp;
    stackFrame.AddrStack.Mode = AddrModeFlat;

    HANDLE process = GetCurrentProcess();
    SymInitialize(process, nullptr, TRUE);

    int count = 0;
    int current = -1;
    while (StackWalk64(IMAGE_FILE_MACHINE_AMD64, process, GetCurrentThread(), &stackFrame, &context,
        nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr))
    {
        current++;
        int status = -1;
        char *moduleName = nullptr;
        char sourceFile[MAX_PATH], sourceFunc[MAX_PATH];
        unsigned int sourceLine = 0;
        char moduleFilePath[MAX_PATH], tmpBuffer[MAX_PATH];

        strcpy(sourceFunc, "???");
        strcpy(sourceFile, "???");

        DWORD64 moduleBase = SymGetModuleBase64(process, stackFrame.AddrPC.Offset);
        if (moduleBase && GetModuleFileNameA(reinterpret_cast<HINSTANCE>(moduleBase), moduleFilePath, MAX_PATH))
        {
            moduleName = moduleFilePath;
            status = BacktraceGetInfoFromModule(moduleFilePath, stackFrame.AddrPC.Offset,
                                                sourceFile, sourceFunc, &sourceLine);
        }
        if (moduleName)
            BacktraceGetFilename(moduleFilePath, moduleName, MAX_PATH);
        else
            strcpy(moduleFilePath, "???");

        if (crashMode && current <= 6)
            continue;
        if (exceptionMode && current <= 1)
            continue;
        if (!crashMode && !exceptionMode && current <= 0)
            continue;

        if (status == 0)
        {
            if (strcmp(sourceFunc, "WinMain") == 0 ||
                strcmp(sourceFunc, "__tmainCRTStartup") == 0 ||
                strcmp(sourceFunc, "mainCRTStartup") == 0 ||
                strcmp(sourceFunc, "WinMainCRTStartup") == 0)
            {
                continue;
            }
            sprintf(tmpBuffer, "%02d.  ", count + 1);
            output += tmpBuffer;
            int status;
            char *name = abi::__cxa_demangle(sourceFunc, nullptr, nullptr, &status);
            if (name)
            {
                output += std::string(name) + " ";
            }
            else
            {
                output += std::string(sourceFunc) + "() ";
            }

            BacktraceGetFilename(moduleFilePath, sourceFile, MAX_PATH);
            strcpy(sourceFile, moduleFilePath);
            output += "at " + std::string(sourceFile) + ": line " + std::to_string(sourceLine) + "\n";
        }
        else if (status == 1)
        {
            DWORD64 unused = 0;
            if (SymFromAddr(process, stackFrame.AddrPC.Offset, &unused, symbol))
            {
                strcpy(sourceFile, symbol->Name);
            }
            if (strcmp(sourceFile, "BaseThreadInitThunk") == 0 ||
                strcmp(sourceFile, "RtlUserThreadStart") == 0)
            {
                continue;
            }
            sprintf(tmpBuffer, "#%02d.  %s\n", count + 1, sourceFile);
            output += tmpBuffer;
        }
        count++;
    }

    SymCleanup(process);

    return true;
}
