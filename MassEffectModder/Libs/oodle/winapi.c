/*
 * Copyright (C) 2021 Pawel Kolodziejski
 * Copyright (C) 2017 Tavis Ormandy
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <search.h>
#ifdef __APPLE__
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#include "search_hsearch_r.h"
#include "winnt_types.h"
#include "pe_linker.h"
#include "log.h"

static struct pe_hsearch_data *crtexports = NULL;

static char *string_from_wchar(const void *wcharbuf, size_t len)
{
    uint16_t *inbuf = (uint16_t *)wcharbuf;
    uint8_t *outbuf = NULL;
    void *buf;
    size_t count    = 0;

    if (wcharbuf == NULL)
        return NULL;

    buf = outbuf = malloc(len + 1);

    while ((*outbuf++ = *inbuf++)) {
        if (++count >= len) {
            *outbuf = '\0';
            break;
        }
    }

    return buf;
}

static size_t CountWideChars(const void *wcharbuf)
{
    size_t i = 0;
    const uint16_t *p = wcharbuf;

    if (!p) return 0;

    while (*p++)
        i++;

    return i;
}

static char *CreateAnsiFromWide(const void *wcharbuf)
{
    return string_from_wchar(wcharbuf, CountWideChars(wcharbuf) * 2);
}

static DWORD LastError;

static DWORD WINAPI GetLastError(void)
{
    DebugLog("GetLastError() => %#x", LastError);

    return LastError;
}

static VOID WINAPI SetLastError(DWORD dwErrCode)
{
    DebugLog("SetLastError(%#x)", dwErrCode);
    LastError = dwErrCode;
}

static VOID WINAPI DeleteCriticalSection(PVOID lpCriticalSection)
{
    DebugLog("");
}

static VOID WINAPI EnterCriticalSection(PVOID lpCriticalSection)
{
    DebugLog("");
}

static VOID WINAPI LeaveCriticalSection(PVOID lpCriticalSection)
{
    DebugLog("");
}

static BOOL WINAPI InitializeCriticalSectionAndSpinCount(PVOID lpCriticalSection, DWORD dwSpinCount)
{
    DebugLog("");
    return TRUE;
}

static BOOL WINAPI IsDebuggerPresent()
{
    DebugLog("");
    return FALSE;
}

static VOID WINAPI OutputDebugStringA(LPCSTR lpOutputString)
{
    DebugLog(lpOutputString);
}

static VOID WINAPI OutputDebugStringW(LPCWSTR lpOutputString)
{
    char *str = CreateAnsiFromWide(lpOutputString);
    DebugLog(str);
    free(str);
}

static PVOID WINAPI EncodePointer(PVOID Ptr)
{
    DebugLog("%p", Ptr);

    return (PVOID)((uintptr_t)(Ptr) ^ ~0);
}

static PVOID WINAPI DecodePointer(PVOID Ptr)
{
    DebugLog("%p", Ptr);
    return (PVOID)((uintptr_t)(Ptr) ^ ~0);
}

#define ERROR_ENVVAR_NOT_FOUND 203

WCHAR EnvironmentStrings[] =
    L"ALLUSERSPROFILE=AllUsersProfile\0"
    L"ALLUSERSAPPDATA=AllUsersAppdata\0";

static PVOID WINAPI GetEnvironmentStringsW(void)
{
    DebugLog("");
    return EnvironmentStrings;
}

static BOOL WINAPI FreeEnvironmentStringsW(PVOID lpszEnvironmentBlock)
{
    DebugLog("%p", lpszEnvironmentBlock);
    return TRUE;
}

#define EXCEPTION_EXECUTE_HANDLER           1

static PVOID WINAPI RaiseException(DWORD dwExceptionCode, DWORD dwExceptionFlags, DWORD nNumberOfArguments, PVOID Arguments)
{
    /* No need to log C++ Exceptions, this is the common case. */
    if (dwExceptionCode != 0xE06D7363) {
        LogMessage("%#x, %#x, %u, %p", dwExceptionCode, dwExceptionFlags, nNumberOfArguments, Arguments);
    }
    debugbreak();
    return NULL;
}

static PVOID WINAPI SetUnhandledExceptionFilter(PVOID lpTopLevelExceptionFilter) {
    DebugLog("");
    return NULL;
}

static LONG WINAPI UnhandledExceptionFilter(PVOID *ExceptionInfo) {
    DebugLog("");
    return EXCEPTION_EXECUTE_HANDLER;
}

static BOOL WINAPI GetConsoleMode(HANDLE hConsoleHandle, LPDWORD lpMode)
{
    DebugLog("%p", hConsoleHandle);
    return FALSE;
}

#define ERROR_FILE_NOT_FOUND 2

static HANDLE WINAPI CreateFileW(PWCHAR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, PVOID lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    FILE *FileHandle;
    char *filename = CreateAnsiFromWide(lpFileName);

    DebugLog("%p [%s], %#x, %#x, %p, %#x, %#x, %p", lpFileName, filename, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

    free(filename);

    SetLastError(ERROR_FILE_NOT_FOUND);
    return INVALID_HANDLE_VALUE;
}

static BOOL WINAPI SetFilePointerEx(HANDLE hFile, uint64_t liDistanceToMove,  uint64_t *lpNewFilePointer, DWORD dwMoveMethod)
{
    DebugLog("%p, %llu, %p, %u", hFile, liDistanceToMove, lpNewFilePointer, dwMoveMethod);
    return TRUE;
}

static BOOL WINAPI CloseHandle(HANDLE hObject)
{
    DebugLog("%p", hObject);
    if (hObject != (HANDLE) 'EVNT'
     && hObject != INVALID_HANDLE_VALUE
     && hObject != (HANDLE) 'SEMA')
        fclose(hObject);
    return TRUE;
}

static BOOL WINAPI FlushFileBuffers(HANDLE hObject)
{
    DebugLog("%p", hObject);
    if (hObject != (HANDLE) 'EVNT'
     && hObject != INVALID_HANDLE_VALUE
     && hObject != (HANDLE) 'SEMA')
        fflush(hObject);
    return TRUE;
}

static BOOL WINAPI WriteFile(HANDLE hFile, PVOID lpBuffer, DWORD nNumberOfBytesToWrite, PDWORD lpNumberOfBytesWritten, PVOID lpOverlapped)
{
    DebugLog("%p", hFile);
    return TRUE;
}

typedef struct {
  DWORD  cb;
  PVOID lpReserved;
  PVOID lpDesktop;
  PVOID lpTitle;
  DWORD  dwX;
  DWORD  dwY;
  DWORD  dwXSize;
  DWORD  dwYSize;
  DWORD  dwXCountChars;
  DWORD  dwYCountChars;
  DWORD  dwFillAttribute;
  DWORD  dwFlags;
  WORD   wShowWindow;
  WORD   cbReserved2;
  PVOID  lpReserved2;
  HANDLE hStdInput;
  HANDLE hStdOutput;
  HANDLE hStdError;
} STARTUPINFO, *LPSTARTUPINFO;

static VOID WINAPI GetStartupInfoW(LPSTARTUPINFO lpStartupInfo)
{
    memset(lpStartupInfo, 0, sizeof *lpStartupInfo);
    DebugLog("GetStartupInfoW(%p)", lpStartupInfo);
}

static PVOID WINAPI GetCommandLineA(void)
{
    DebugLog("");
    return "totallylegit.exe notfake very real";
}

#define STD_INPUT_HANDLE (-10)
#define STD_OUTPUT_HANDLE (-11)
#define STD_ERROR_HANDLE (-12)

#define FILE_TYPE_CHAR 0x0002

static HANDLE WINAPI GetStdHandle(DWORD nStdHandle)
{
    DebugLog("%d", nStdHandle);

    switch (nStdHandle) {
        case STD_INPUT_HANDLE:
            return (HANDLE) 0;
        case STD_OUTPUT_HANDLE:
            return (HANDLE) 1;
        case STD_ERROR_HANDLE:
            return (HANDLE) 2;
        default:
            break;
    }

    return INVALID_HANDLE_VALUE;
}

static BOOL WINAPI SetStdHandle(DWORD nStdHandle, HANDLE hHandle)
{
    DebugLog("%d %d", nStdHandle, hHandle);

    switch (nStdHandle) {
        case STD_INPUT_HANDLE:
        case STD_OUTPUT_HANDLE:
        case STD_ERROR_HANDLE:
            break;
        default:
            return FALSE;
    }

    return TRUE;
}

static DWORD WINAPI GetFileType(HANDLE hFile)
{
    DebugLog("%p", hFile);
    return FILE_TYPE_CHAR;
}

static BOOL WINAPI WriteConsoleW(HANDLE hConsoleOutput, const VOID *lpBuffer, DWORD nNumberOfCharsToWrite, LPDWORD lpNumberOfCharsWritten, LPVOID  lpReserved) {
    PVOID tmpStr = calloc(nNumberOfCharsToWrite + 1, sizeof(USHORT));
    memcpy(tmpStr, lpBuffer, nNumberOfCharsToWrite);
    char *str = CreateAnsiFromWide(tmpStr);
    if (str) {
        puts(str);
        *lpNumberOfCharsWritten = nNumberOfCharsToWrite;
        free(str);
    } else {
        *lpNumberOfCharsWritten = 0;
    }
    free(tmpStr);
    return TRUE;
}

#define HEAP_ZERO_MEMORY 8

static HANDLE WINAPI GetProcessHeap(void)
{
    DebugLog("");
    return (HANDLE) 'HEAP';
}

static PVOID WINAPI HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes)
{
    PVOID Buffer;

    DebugLog("%p, %#x, %u", hHeap, dwFlags, dwBytes);

    if (dwFlags & HEAP_ZERO_MEMORY) {
        Buffer = calloc(dwBytes, 1);
    } else {
        Buffer = malloc(dwBytes);
    }

    return Buffer;
}

static BOOL WINAPI HeapFree(HANDLE hHeap, DWORD dwFlags, PVOID lpMem)
{
    DebugLog("%p, %#x, %p", hHeap, dwFlags, lpMem);
    free(lpMem);
    return TRUE;
}

static SIZE_T WINAPI HeapSize(HANDLE hHeap, DWORD dwFlags, PVOID lpMem)
{
    DebugLog("%p, %#x, %p", hHeap, dwFlags, lpMem);

#ifdef __APPLE__
    return (SIZE_T)malloc_size(lpMem);
#else
    return (SIZE_T)malloc_usable_size(lpMem);
#endif
}

static PVOID WINAPI HeapReAlloc(HANDLE hHeap, DWORD dwFlags, PVOID lpMem, SIZE_T dwBytes)
{
    DebugLog("%p, %#x, %p, %#x", hHeap, dwFlags, lpMem, dwBytes);
    return realloc(lpMem, dwBytes);
}

static BOOL WINAPI IsProcessorFeaturePresent(DWORD ProcessorFeature)
{
    DebugLog("IsProcessorFeaturePresent(%u) => FALSE (Unknown)", ProcessorFeature);
    return FALSE;
}

static HANDLE WINAPI LoadLibraryExW(PVOID lpFileName, HANDLE hFile, DWORD dwFlags)
{
    char *name = CreateAnsiFromWide(lpFileName);
    DebugLog("%p [%s], %p, %#x", lpFileName, name, hFile, dwFlags);
    free(name);
    return (HANDLE) 'LOAD';
}

static PVOID WINAPI GetProcAddressLocal(HANDLE hModule, PCHAR lpProcName)
{
    ENTRY key = { lpProcName, 0 }, *item;

    DebugLog("%p [%s]", hModule, lpProcName);

    if (hModule == (HANDLE) NULL || hModule == (HANDLE) 'LOAD' || hModule == (HANDLE) 'KERN') {
        return NULL;
    }

    if (pe_hsearch_r(key, FIND, &item, crtexports)) {
        return item->data;
    }

    DebugLog("FIXME: %s unresolved", lpProcName);
    return NULL;
}

static HANDLE WINAPI GetModuleHandleW(PVOID lpModuleName)
{
    char *name = CreateAnsiFromWide(lpModuleName);

    DebugLog("%p [%s]", lpModuleName, name);

    free(name);

    if (lpModuleName && memcmp(lpModuleName, L"KERNEL32.DLL", sizeof(L"KERNEL32.DLL")) == 0)
        return (HANDLE) 'KERN';

    if (lpModuleName && memcmp(lpModuleName, L"kernel32.dll", sizeof(L"kernel32.dll")) == 0)
        return (HANDLE) 'KERN';

    return (HANDLE) NULL;
}

static DWORD WINAPI GetModuleFileNameA(HANDLE hModule, PCHAR lpFilename, DWORD nSize)
{
    DebugLog("%p, %p, %u", hModule, lpFilename, nSize);
    strncpy(lpFilename, "C:\\dummy\\fakename.exe", nSize);
    return strlen(lpFilename);
}

static DWORD WINAPI GetModuleFileNameW(HANDLE hModule, PWCHAR lpFilename, DWORD nSize)
{
    DebugLog("%p, %p, %u", hModule, lpFilename, nSize);

    if (nSize > strlen("C:\\dummy\\fakename.exe")) {
        memcpy(lpFilename, L"C:\\dummy\\fakename.exe", sizeof(L"C:\\dummy\\fakename.exe"));
    }

    return strlen("C:\\dummy\\fakename.exe");
}

static BOOL WINAPI GetModuleHandleExW(DWORD dwFlags,
                                      LPCWSTR lpModuleName,
                                      HMODULE *phModule)
{
    char *name = CreateAnsiFromWide(lpModuleName);

    DebugLog("%p [%s]", lpModuleName, name);

    free(name);

    if (lpModuleName && memcmp(lpModuleName, L"KERNEL32.DLL", sizeof(L"KERNEL32.DLL")) == 0)
        *phModule = (HANDLE) 'KERN';

    if (lpModuleName && memcmp(lpModuleName, L"kernel32.dll", sizeof(L"kernel32.dll")) == 0)
        *phModule = (HANDLE) 'KERN';

    *phModule = (HANDLE) NULL;

    return TRUE;
}

#define MAX_DEFAULTCHAR 2
#define MAX_LEADBYTES 12

typedef struct {
  UINT MaxCharSize;
  BYTE DefaultChar[MAX_DEFAULTCHAR];
  BYTE LeadByte[MAX_LEADBYTES];
} CPINFO, *LPCPINFO;

static UINT WINAPI GetACP(void)
{
    DebugLog("");
    return 65001;   /* UTF-8 */
}

static UINT WINAPI GetConsoleCP(void)
{
    DebugLog("");
    return 65001;   /* UTF-8 */
}

static UINT WINAPI GetOEMCP(void)
{
    DebugLog("");
    return 65001;   /* UTF-8 */
}

static BOOL WINAPI IsValidCodePage(UINT CodePage)
{
    DebugLog("%u", CodePage);
    return TRUE;
}

static BOOL WINAPI GetCPInfo(UINT CodePage, LPCPINFO lpCPInfo)
{
    DebugLog("%u, %p", CodePage, lpCPInfo);

    memset(lpCPInfo, 0, sizeof *lpCPInfo);

    lpCPInfo->MaxCharSize       = 1;
    lpCPInfo->DefaultChar[0]    = '?';
    return TRUE;
}

static int WINAPI LCMapStringW(DWORD Locale, DWORD dwMapFlags, PVOID lpSrcStr, int cchSrc, PVOID lpDestStr, int cchDest)
{
    DebugLog("%u, %#x, %p, %d, %p, %d", Locale, dwMapFlags, lpSrcStr, cchSrc, lpDestStr, cchDest);
    return 1;
}

static VOID WINAPI ExitProcess(UINT code)
{
    DebugLog("");
}

static HANDLE WINAPI GetCurrentProcess(VOID)
{
    DebugLog("");
    return (HANDLE)-1;
}

static DWORD WINAPI GetCurrentThreadId(VOID)
{
    DebugLog("");
    return getpid();
}

static DWORD WINAPI GetCurrentProcessId(VOID)
{
    DebugLog("");
    return getpid();
}

static BOOL WINAPI TerminateProcess(HANDLE hProcess, UINT uExitCode)
{
    DebugLog("%p, %d", hProcess, uExitCode);
    if ((ULONG_PTR)hProcess != (ULONG_PTR)-1)
        return FALSE;

    exit(uExitCode);
    return TRUE;
}

static VOID WINAPI Sleep(DWORD dwMilliseconds)
{
    DebugLog("");
}

static PVOID WINAPI RtlPcToFileHeader(PVOID PcValue, PVOID *BaseOfImage) {
    DebugLog("%p %p", PcValue, BaseOfImage);
    return NULL;
}

static VOID WINAPI RtlCaptureContext(PVOID ContextRecord) {
    DebugLog("%p", ContextRecord);
}

static PVOID WINAPI RtlLookupFunctionEntry(DWORD64 ControlPc, PDWORD64 ImageBase, PVOID HistoryTable) {
    DebugLog("%#x %#x %p", ControlPc, ImageBase, HistoryTable);
    return NULL;
}

static VOID WINAPI RtlUnwindEx(PVOID TargetFrame, PVOID TargetIp, PVOID ExceptionRecord, PVOID ReturnValue, PVOID ContextRecord, PVOID HistoryTable) {
    DebugLog("%p %p %p %p %p %p", TargetFrame, TargetIp, ExceptionRecord, ReturnValue, ContextRecord, HistoryTable);
}

static PVOID WINAPI RtlVirtualUnwind(DWORD HandlerType, DWORD64 ImageBase, DWORD64 ControlPc, PVOID FunctionEntry, PVOID ContextRecord,
                                     PVOID *HandlerData, PDWORD64 EstablisherFrame, PVOID ContextPointers) {
    DebugLog("%d %#x %#x %p %p %p %#x %p", HandlerType, ImageBase, ControlPc, FunctionEntry, ContextRecord, HandlerData, EstablisherFrame, ContextPointers);
    return NULL;
}

#define MB_ERR_INVALID_CHARS 8
#define MB_PRECOMPOSED 1

static int WINAPI MultiByteToWideChar(UINT CodePage, DWORD dwFlags, PCHAR lpMultiByteStr, int cbMultiByte, PUSHORT lpWideCharStr, int cchWideChar)
{
    size_t i;

    DebugLog("%u, %#x, %p, %u, %p, %u", CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);

    if ((dwFlags & ~(MB_ERR_INVALID_CHARS | MB_PRECOMPOSED)) != 0) {
        LogMessage("Unsupported Conversion Flags %#x", dwFlags);
    }

    if (CodePage != 0 && CodePage != 65001) {
        DebugLog("Unsupported CodePage %u", CodePage);
    }

    if (cbMultiByte == 0)
        return 0;

    if (cbMultiByte == -1)
        cbMultiByte = strlen(lpMultiByteStr) + 1;

    if (cchWideChar == 0)
        return cbMultiByte;

    /* cbMultibyte is the number of *bytes* to process.
       cchWideChar is the number of output *chars* expected. */
    if (cbMultiByte > cchWideChar) {
        return 0;
    }

    for (i = 0; i < cbMultiByte; i++) {
        lpWideCharStr[i] = (uint8_t) lpMultiByteStr[i];
        if (dwFlags & MB_ERR_INVALID_CHARS) {
            if (!isascii(lpMultiByteStr[i]) || iscntrl(lpMultiByteStr[i])) {
                lpWideCharStr[i] = '?';
            }
        }
    }

    return i;
}

static int WINAPI WideCharToMultiByte(UINT CodePage, DWORD dwFlags, PVOID lpWideCharStr, int cchWideChar,
                                      PVOID lpMultiByteStr, int cbMultiByte, PVOID lpDefaultChar, PVOID lpUsedDefaultChar)
{
    char *ansi = NULL;

    DebugLog("%u, %#x, %p, %d, %p, %d, %p, %p", CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cbMultiByte, lpDefaultChar, lpUsedDefaultChar);

    if (cchWideChar != -1) {
        PVOID tmpStr = calloc(cchWideChar + 1, sizeof(USHORT));
        memcpy(tmpStr, lpWideCharStr, cchWideChar);
        ansi = CreateAnsiFromWide(tmpStr);
        free(tmpStr);
    } else {
        ansi = CreateAnsiFromWide(lpWideCharStr);
    }

    if (ansi == NULL) {
        return 0;
    }

    DebugLog("cchWideChar == %d, Ansi: [%s]", cchWideChar, ansi);

    if (lpMultiByteStr && strlen(ansi) < cbMultiByte) {
        strcpy(lpMultiByteStr, ansi);
        free(ansi);
        return strlen(lpMultiByteStr) + 1;
    } else if (!lpMultiByteStr && cbMultiByte == 0) {
        int len = strlen(ansi) + 1;
        free(ansi);
        return len;
    }

    free(ansi);

    return 0;
}

static BOOL WINAPI GetStringTypeW(DWORD dwInfoType, PUSHORT lpSrcStr, int cchSrc, PUSHORT lpCharType)
{
    DebugLog("%u, %p, %d, %p", dwInfoType, lpSrcStr, cchSrc, lpCharType);
    memset(lpCharType, 1, cchSrc * sizeof(USHORT));
    return FALSE;
}

static VOID WINAPI GetSystemTimeAsFileTime(PVOID lpSystemTimeAsFileTime)
{
    DebugLog("");
    memset(lpSystemTimeAsFileTime, 0, sizeof(DWORD64));
}

static BOOL WINAPI QueryPerformanceCounter(LARGE_INTEGER *lpPerformanceCount)
{
    DebugLog("");

    SetLastError(0);

    struct timespec tm;
    if (clock_gettime(CLOCK_MONOTONIC_RAW, &tm) != 0)
        return FALSE;

    lpPerformanceCount = tm.tv_nsec;

    return TRUE;
}

#ifndef TLS_OUT_OF_INDEXES
# define TLS_OUT_OF_INDEXES 0xFFFFFFFF
#endif

#define TLS_MINIMUM_AVAILABLE 64

static DWORD TlsIndex = 1;
extern DWORD64 PeLocalStorage[TLS_MINIMUM_AVAILABLE];

static DWORD WINAPI TlsAlloc(void)
{
    DebugLog("");
    if (TlsIndex >= ARRAY_SIZE(PeLocalStorage) - 1) {
        DebugLog("TlsAlloc() => %#x", TlsIndex);
        return TLS_OUT_OF_INDEXES;
    }

    return TlsIndex++;
}

static BOOL WINAPI TlsSetValue(DWORD dwTlsIndex, DWORD64 lpTlsValue)
{
    DebugLog("TlsSetValue(%u, %#x)", dwTlsIndex, lpTlsValue);

    if (dwTlsIndex < ARRAY_SIZE(PeLocalStorage)) {
        PeLocalStorage[dwTlsIndex] = lpTlsValue;
        return TRUE;
    }

    DebugLog("dwTlsIndex higher than current maximum");

    return FALSE;
}

static DWORD64 WINAPI TlsGetValue(DWORD dwTlsIndex)
{
    DebugLog("");
    if (dwTlsIndex < ARRAY_SIZE(PeLocalStorage)) {
        return PeLocalStorage[dwTlsIndex];
    }

    return 0;
}

static BOOL WINAPI TlsFree(DWORD dwTlsIndex)
{
    DebugLog("");
    if (dwTlsIndex < ARRAY_SIZE(PeLocalStorage)) {
        PeLocalStorage[dwTlsIndex] = (DWORD64)NULL;
        return TRUE;
    }

    return FALSE;
}

static const ENTRY crtExportsTable[] =
{
    { "CloseHandle", CloseHandle },
    { "CreateFileW", CreateFileW },
    { "DecodePointer", DecodePointer },
    { "DeleteCriticalSection", DeleteCriticalSection },
    { "EncodePointer", EncodePointer },
    { "EnterCriticalSection", EnterCriticalSection },
    { "ExitProcess", ExitProcess },
    { "FlushFileBuffers", FlushFileBuffers },
    { "FreeEnvironmentStringsW", FreeEnvironmentStringsW },
    { "GetACP", GetACP },
    { "GetCPInfo", GetCPInfo },
    { "GetCommandLineA", GetCommandLineA },
    { "GetConsoleCP", GetConsoleCP },
    { "GetConsoleMode", GetConsoleMode },
    { "GetCurrentProcess", GetCurrentProcess },
    { "GetCurrentProcessId", GetCurrentProcessId },
    { "GetCurrentThreadId", GetCurrentThreadId },
    { "GetEnvironmentStringsW", GetEnvironmentStringsW },
    { "GetFileType", GetFileType },
    { "GetLastError", GetLastError },
    { "GetModuleFileNameA", GetModuleFileNameA },
    { "GetModuleFileNameW", GetModuleFileNameW },
    { "GetModuleHandleExW", GetModuleHandleExW },
    { "GetModuleHandleW", GetModuleHandleW },
    { "GetOEMCP", GetOEMCP },
    { "GetProcAddress", GetProcAddressLocal },
    { "GetProcessHeap", GetProcessHeap },
    { "GetStartupInfoW", GetStartupInfoW },
    { "GetStdHandle", GetStdHandle },
    { "GetStringTypeW", GetStringTypeW },
    { "GetSystemTimeAsFileTime", GetSystemTimeAsFileTime },
    { "HeapAlloc", HeapAlloc },
    { "HeapFree", HeapFree },
    { "HeapReAlloc", HeapReAlloc },
    { "HeapSize", HeapSize },
    { "IsDebuggerPresent", IsDebuggerPresent },
    { "InitializeCriticalSectionAndSpinCount", InitializeCriticalSectionAndSpinCount },
    { "IsProcessorFeaturePresent", IsProcessorFeaturePresent },
    { "IsValidCodePage", IsValidCodePage },
    { "LCMapStringW", LCMapStringW },
    { "LeaveCriticalSection", LeaveCriticalSection },
    { "LoadLibraryExW", LoadLibraryExW },
    { "MultiByteToWideChar", MultiByteToWideChar },
    { "OutputDebugStringA", OutputDebugStringA },
    { "OutputDebugStringW", OutputDebugStringW },
    { "QueryPerformanceCounter", QueryPerformanceCounter },
    { "RaiseException", RaiseException },
    { "RtlCaptureContext", RtlCaptureContext },
    { "RtlLookupFunctionEntry", RtlLookupFunctionEntry },
    { "RtlPcToFileHeader", RtlPcToFileHeader },
    { "RtlUnwindEx", RtlUnwindEx },
    { "RtlVirtualUnwind", RtlVirtualUnwind },
    { "SetFilePointerEx", SetFilePointerEx },
    { "SetLastError", SetLastError },
    { "SetStdHandle", SetStdHandle },
    { "SetUnhandledExceptionFilter", SetUnhandledExceptionFilter },
    { "Sleep", Sleep },
    { "TerminateProcess", TerminateProcess },
    { "TlsAlloc", TlsAlloc },
    { "TlsFree", TlsFree },
    { "TlsGetValue", TlsGetValue },
    { "TlsSetValue", TlsSetValue },
    { "UnhandledExceptionFilter", UnhandledExceptionFilter },
    { "WideCharToMultiByte", WideCharToMultiByte },
    { "WriteConsoleW", WriteConsoleW },
    { "WriteFile", WriteFile },
};

void PeLoadCrtExports(struct pe_image *pe)
{
    pe_hcreate_r(1024, &pe->crtexports);
    crtexports = &pe->crtexports;

    for (uint32_t i = 0; i < ARRAY_SIZE(crtExportsTable); i++)
    {
        ENTRY e = { crtExportsTable[i].key, crtExportsTable[i].data }, *ep;
        pe_hsearch_r(e, ENTER, &ep, &pe->crtexports);
    }
}
