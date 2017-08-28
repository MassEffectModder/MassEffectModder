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

#define PACKAGE "MEM"
#define PACKAGE_VERSION ""

#include <windows.h>
#include <imagehlp.h>
#include <string>

#include <binutils/bfd.h>
#include <libiberty/demangle.h>

int getInfoFromModule(char *moduleFilePath, DWORD64 offset, const char **sourceFile,
    const char **sourceFunc, unsigned int *sourceLine)
{
    bfd *bfdHandle = bfd_openr(moduleFilePath, NULL);
    if (bfdHandle == NULL)
        return -1;

    if (!bfd_check_format(bfdHandle, bfd_object))
    {
        bfd_close(bfdHandle);
        return -1;
    }

    char **matching;
    if (!bfd_check_format_matches(bfdHandle, bfd_object, &matching))
    {
        if (bfd_get_error() == bfd_error_file_ambiguously_recognized)
            free(matching);
        bfd_close(bfdHandle);
        return -1;
    }

    if ((bfd_get_file_flags(bfdHandle) & HAS_SYMS) != HAS_SYMS)
    {
        bfd_close(bfdHandle);
        return 1;
    }

    bfd_symbol *symbolsTable;
    unsigned int unused;
    long int numberSymbols = bfd_read_minisymbols(bfdHandle, FALSE, (void **)&symbolsTable, &unused);
    if (numberSymbols == 0)
    {
        numberSymbols = bfd_read_minisymbols(bfdHandle, TRUE, (void **)&symbolsTable, &unused);
        if (numberSymbols <= 0)
        {
            bfd_close(bfdHandle);
            return 1;
        }
    }

    asection *section = bfdHandle->sections;
    while (section != NULL)
    {
        if ((bfd_get_section_flags(bfdHandle, section) & SEC_ALLOC) == SEC_ALLOC)
        {
            bfd_vma address = bfd_get_section_vma(bfdHandle, section);
            bfd_size_type sectionSize = bfd_get_section_size(section);
            if (offset >= address && offset < address + sectionSize)
            {
                if (bfd_find_nearest_line(bfdHandle, section, (bfd_symbol **)symbolsTable,
                    offset - address, sourceFile, sourceFunc, sourceLine))
                {
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

static void getFilename(char *dst, const char *src)
{
    int offset = 0;
    for (char *ptr = (char *)src; *ptr != 0; ptr++)
    {
        if (*ptr == '/' || *ptr == '\\')
            offset = ptr - src + 1;
    }
    strncpy(dst, src + offset, MAX_PATH);
}


bool GetBackTrace(std::string &output, bool crashMode = true)
{
    bfd_init();

    char symbolInfo[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
    PSYMBOL_INFO symbol = (PSYMBOL_INFO)symbolInfo;
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
    SymInitialize(process, NULL, TRUE);

    int count = 0;
    int current = -1;
    while (StackWalk64(IMAGE_FILE_MACHINE_AMD64, process, GetCurrentThread(), &stackFrame, &context,
        NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL))
    {
        current++;
        int status = -1;
        const char *moduleName = "???";
        const char *sourceFile, *sourceFunc;
        unsigned int sourceLine;
        char moduleFilePath[MAX_PATH];
        char tmpBuffer[MAX_PATH];

        DWORD64 moduleBase = SymGetModuleBase64(process, stackFrame.AddrPC.Offset);
        if (moduleBase && GetModuleFileNameA((HINSTANCE)moduleBase, moduleFilePath, MAX_PATH))
        {
            moduleName = moduleFilePath;
            status = getInfoFromModule(moduleFilePath, stackFrame.AddrPC.Offset, &sourceFile, &sourceFunc, &sourceLine);
        }
        if (moduleName)
            getFilename(moduleFilePath, moduleName);
        else
            strcpy(moduleFilePath, "???");

        if (crashMode && current <= 6)
            continue;
        if (!crashMode && current <= 0)
            continue;
        if (status == 0)
        {
            if (!sourceFunc)
            {
                sprintf(tmpBuffer, "#%02d  0x%016llx %s at ???", count, stackFrame.AddrPC.Offset, moduleName);
                output += tmpBuffer;
            }
            else
            {
                if (strcmp(sourceFunc, "WinMain") == 0 ||
                      strcmp(sourceFunc, "__tmainCRTStartup") == 0 ||
                      strcmp(sourceFunc, "WinMainCRTStartup") == 0)
                    continue;
                sprintf(tmpBuffer, "#%02d  0x%016llx %s in ", count, stackFrame.AddrPC.Offset, moduleName);
                output += tmpBuffer;
                char *name = cplus_demangle(sourceFunc, DMGL_PARAMS | DMGL_ANSI | DMGL_VERBOSE | DMGL_TYPES);
                if (name)
                {
                    output += name;
                    output += " ";
                }
                else
                {
                    output += sourceFunc;
                    output += "() ";
                }

                if (moduleName)
                    getFilename(moduleFilePath, moduleName);
                else
                    strcpy(moduleFilePath, "???");
                sourceFile = moduleFilePath;
                output += "at ";
                output += sourceFile;
                output += ":";
                output += std::to_string(sourceLine);
            }
            output += "\n";
        }
        else if (status == 1)
        {
            DWORD64 unused = 0;
            if (SymFromAddr(process, stackFrame.AddrPC.Offset, &unused, symbol))
                sourceFile = symbol->Name;
            else
                sourceFile = "???";
            if (strcmp(sourceFile, "BaseThreadInitThunk") == 0 ||
                strcmp(sourceFile, "RtlUserThreadStart") == 0)
                continue;
            sprintf(tmpBuffer, "#%02d  0x%016llx %s at %s\n", count, stackFrame.AddrPC.Offset, moduleName, sourceFile);
            output += tmpBuffer;
        }
        count++;
    }

    SymCleanup(process);

    return true;
}
