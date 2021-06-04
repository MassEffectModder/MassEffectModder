/*
 *  Copyright (C) 2003-2005 Pontus Fuchs, Giridhar Pemmasani
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 */

#ifndef PE_WINNT_TYPES_H
#define PE_WINNT_TYPES_H

#include <stdint.h>
#include <stddef.h>

#define DLL_PROCESS_ATTACH              1
#define DLL_PROCESS_DETACH              0
#define DLL_THREAD_ATTACH               2
#define DLL_THREAD_DETACH               3

#define TRUE                            1
#define FALSE                           0

#define HANDLE                          PVOID
#define HMODULE                         PVOID
#define INVALID_HANDLE_VALUE            ((HANDLE)(-1))
#define STATIC                          static
#define VOID                            void
#define WINAPI                          __attribute__((ms_abi))

#define ARRAY_SIZE(x)                   (sizeof((x)) / sizeof((x)[0]))

#define STATUS_WAIT_0                   0
#define STATUS_SUCCESS                  0
#define STATUS_ALERTED                  0x00000101
#define STATUS_TIMEOUT                  0x00000102
#define STATUS_PENDING                  0x00000103
#define STATUS_FAILURE                  0xC0000001
#define STATUS_NOT_IMPLEMENTED          0xC0000002
#define STATUS_INVALID_PARAMETER        0xC000000D
#define STATUS_INVALID_DEVICE_REQUEST   0xC0000010
#define STATUS_MORE_PROCESSING_REQUIRED 0xC0000016
#define STATUS_ACCESS_DENIED            0xC0000022
#define STATUS_BUFFER_TOO_SMALL         0xC0000023
#define STATUS_OBJECT_NAME_INVALID      0xC0000023
#define STATUS_MUTANT_NOT_OWNED         0xC0000046
#define STATUS_RESOURCES                0xC000009A
#define STATUS_DELETE_PENDING           0xC0000056
#define STATUS_INSUFFICIENT_RESOURCES   0xC000009A
#define STATUS_NOT_SUPPORTED            0xC00000BB
#define STATUS_INVALID_PARAMETER_2      0xC00000F0
#define STATUS_NO_MEMORY                0xC0000017
#define STATUS_CANCELLED                0xC0000120
#define STATUS_DEVICE_REMOVED           0xC00002B6
#define STATUS_DEVICE_NOT_CONNECTED     0xC000009D
#define STATUS_BUFFER_OVERFLOW          0x80000005

typedef uint8_t         BOOLEAN, BOOL, *PBOOL;
typedef void            *PVOID, *LPVOID;
typedef uint8_t         BYTE;
typedef uint8_t         *PBYTE;
typedef uint8_t         *LPBYTE;
typedef int8_t          CHAR;
typedef char            *PCHAR;
typedef CHAR            *LPSTR;
typedef const char      *LPCSTR;
typedef uint16_t        WCHAR, *PWCHAR;
typedef WCHAR           *LPWSTR;
typedef const WCHAR     *LPCWSTR, *LPCWCH;
typedef WCHAR           *PWSTR;
typedef uint8_t         UCHAR;
typedef uint8_t         *PUCHAR;
typedef uint16_t        SHORT;
typedef uint16_t        USHORT;
typedef uint16_t        *PUSHORT;
typedef uint16_t        WORD;
typedef int32_t         INT;
typedef uint32_t        UINT;
typedef uint32_t        DWORD, *PDWORD, *LPDWORD;
typedef int32_t         LONG;
typedef uint32_t        ULONG;
typedef uint32_t        *PULONG;
typedef int64_t         LONGLONG, DWORD64, *PDWORD64;
typedef uint64_t        ULONGLONG, *PULONGLONG;
typedef uint64_t        ULONGULONG;
typedef uint64_t        ULONG64;
typedef uint64_t        QWORD, *PQWORD;
typedef HANDLE          *PHANDLE;
typedef LONG            HRESULT;
typedef CHAR            CCHAR;
typedef SHORT           CSHORT;
typedef LONGLONG        LARGE_INTEGER;
typedef size_t          SIZE_T;

/* ULONG_PTR is 32 bits on 32-bit platforms and 64 bits on 64-bit
 * platform, which is same as 'unsigned long' in Linux */
typedef unsigned long   ULONG_PTR;

#endif /* WINNT_TYPES_H */
