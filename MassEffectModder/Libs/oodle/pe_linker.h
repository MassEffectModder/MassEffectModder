/*
 * This file is an excerpt of winnt.h from WINE, which bears the
 * following copyright:
 *
 * Win32 definitions for Windows NT
 *
 * Copyright 1996 Alexandre Julliard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#ifndef __PE_LINKER_H
#define __PE_LINKER_H

#include <stdlib.h>
#include <search.h>
#include "winnt_types.h"
#include "search_hsearch_r.h"

#define __packed        __attribute__((packed))

#define debugbreak()    __asm__("int3")

typedef void (*generic_func)(void);

/*
 * File formats definitions
 */
typedef struct _IMAGE_DOS_HEADER {
    WORD  e_magic;      /* 00: MZ Header signature */
    WORD  e_cblp;       /* 02: Bytes on last page of file */
    WORD  e_cp;         /* 04: Pages in file */
    WORD  e_crlc;       /* 06: Relocations */
    WORD  e_cparhdr;    /* 08: Size of header in paragraphs */
    WORD  e_minalloc;   /* 0a: Minimum extra paragraphs needed */
    WORD  e_maxalloc;   /* 0c: Maximum extra paragraphs needed */
    WORD  e_ss;         /* 0e: Initial (relative) SS value */
    WORD  e_sp;         /* 10: Initial SP value */
    WORD  e_csum;       /* 12: Checksum */
    WORD  e_ip;         /* 14: Initial IP value */
    WORD  e_cs;         /* 16: Initial (relative) CS value */
    WORD  e_lfarlc;     /* 18: File address of relocation table */
    WORD  e_ovno;       /* 1a: Overlay number */
    WORD  e_res[4];     /* 1c: Reserved words */
    WORD  e_oemid;      /* 24: OEM identifier (for e_oeminfo) */
    WORD  e_oeminfo;    /* 26: OEM information; e_oemid specific */
    WORD  e_res2[10];   /* 28: Reserved words */
    DWORD e_lfanew;     /* 3c: Offset to extended header */
} IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

#define IMAGE_NT_SIGNATURE     0x00004550 /* PE00 */

/* These defines describe the meanings of the bits in the
   Characteristics field */

#define IMAGE_FILE_RELOCS_STRIPPED      0x0001 /* No relocation info */
#define IMAGE_FILE_EXECUTABLE_IMAGE     0x0002
#define IMAGE_FILE_DLL                  0x2000

/* These are the settings of the Machine field. */
#define IMAGE_FILE_MACHINE_AMD64        0x8664

/* Possible Magic values */
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC        0x020b

/* Directory Entries, indices into the DataDirectory array */

#define IMAGE_DIRECTORY_ENTRY_EXPORT            0
#define IMAGE_DIRECTORY_ENTRY_IMPORT            1
#define IMAGE_DIRECTORY_ENTRY_BASERELOC         5
#define IMAGE_DIRECTORY_ENTRY_TLS               9

typedef struct _IMAGE_FILE_HEADER {
  WORD  Machine;
  WORD  NumberOfSections;
  DWORD TimeDateStamp;
  DWORD PointerToSymbolTable;
  DWORD NumberOfSymbols;
  WORD  SizeOfOptionalHeader;
  WORD  Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

typedef struct _IMAGE_DATA_DIRECTORY {
  DWORD VirtualAddress;
  DWORD Size;
} IMAGE_DATA_DIRECTORY, *PIMAGE_DATA_DIRECTORY;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16

typedef struct _IMAGE_OPTIONAL_HEADER64 {

    /* Standard fields */

    WORD  Magic;
    BYTE  MajorLinkerVersion;
    BYTE  MinorLinkerVersion;
    DWORD SizeOfCode;
    DWORD SizeOfInitializedData;
    DWORD SizeOfUninitializedData;
    DWORD AddressOfEntryPoint;
    DWORD BaseOfCode;

    /* NT additional fields */
    ULONGLONG ImageBase;
    DWORD SectionAlignment;
    DWORD FileAlignment;
    WORD  MajorOperatingSystemVersion;
    WORD  MinorOperatingSystemVersion;
    WORD  MajorImageVersion;
    WORD  MinorImageVersion;
    WORD  MajorSubsystemVersion;
    WORD  MinorSubsystemVersion;
    DWORD Win32VersionValue;
    DWORD SizeOfImage;
    DWORD SizeOfHeaders;
    DWORD CheckSum;
    WORD  Subsystem;
    WORD  DllCharacteristics;
    ULONGLONG SizeOfStackReserve;
    ULONGLONG SizeOfStackCommit;
    ULONGLONG SizeOfHeapReserve;
    ULONGLONG SizeOfHeapCommit;
    DWORD LoaderFlags;
    DWORD NumberOfRvaAndSizes;
    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER64, *PIMAGE_OPTIONAL_HEADER64;

typedef IMAGE_OPTIONAL_HEADER64 IMAGE_OPTIONAL_HEADER;
typedef PIMAGE_OPTIONAL_HEADER64 PIMAGE_OPTIONAL_HEADER;

typedef struct _IMAGE_NT_HEADERS64 {
  DWORD Signature; /* "PE"\0\0 */       /* 0x00 */
  IMAGE_FILE_HEADER FileHeader;         /* 0x04 */
  IMAGE_OPTIONAL_HEADER64 OptionalHeader;       /* 0x18 */
} IMAGE_NT_HEADERS64, *PIMAGE_NT_HEADERS64;

typedef IMAGE_NT_HEADERS64 IMAGE_NT_HEADERS;
typedef PIMAGE_NT_HEADERS64 PIMAGE_NT_HEADERS;

#define IMAGE_SIZEOF_SHORT_NAME 8

typedef struct _IMAGE_SECTION_HEADER {
    BYTE  Name[IMAGE_SIZEOF_SHORT_NAME];
    union {
        DWORD PhysicalAddress;
        DWORD VirtualSize;
    } Misc;
    DWORD VirtualAddress;
    DWORD SizeOfRawData;
    DWORD PointerToRawData;
    DWORD PointerToRelocations;
    DWORD PointerToLinenumbers;
    WORD  NumberOfRelocations;
    WORD  NumberOfLinenumbers;
    DWORD Characteristics;
} IMAGE_SECTION_HEADER, *PIMAGE_SECTION_HEADER;

#define IMAGE_SIZEOF_SECTION_HEADER 40

#define IMAGE_FIRST_SECTION(ntheader) \
    ((PIMAGE_SECTION_HEADER)((LPBYTE)&((PIMAGE_NT_HEADERS)(ntheader))->OptionalHeader + \
    ((PIMAGE_NT_HEADERS)(ntheader))->FileHeader.SizeOfOptionalHeader))

/* Export module directory */

typedef struct _IMAGE_EXPORT_DIRECTORY {
    DWORD   Characteristics;
    DWORD   TimeDateStamp;
    WORD    MajorVersion;
    WORD    MinorVersion;
    DWORD   Name;
    DWORD   Base;
    DWORD   NumberOfFunctions;
    DWORD   NumberOfNames;
    DWORD   AddressOfFunctions;
    DWORD   AddressOfNames;
    DWORD   AddressOfNameOrdinals;
} IMAGE_EXPORT_DIRECTORY,*PIMAGE_EXPORT_DIRECTORY;

/* Import module directory */

typedef struct __packed _IMAGE_IMPORT_DESCRIPTOR {
    union {
        DWORD   Characteristics; /* 0 for terminating null
                                  * import descriptor */
        DWORD   OriginalFirstThunk; /* RVA to original unbound
                                     * IAT */
    } u;
    DWORD   TimeDateStamp;  /* 0 if not bound,
                             * -1 if bound, and real date\time stamp
                             *    in IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT
                             * (new BIND)
                             * otherwise date/time stamp of DLL bound to
                             * (Old BIND)
                             */
    DWORD   ForwarderChain; /* -1 if no forwarders */
    DWORD   Name;
    /* RVA to IAT (if bound this IAT has actual addresses) */
    DWORD   FirstThunk;
} IMAGE_IMPORT_DESCRIPTOR,*PIMAGE_IMPORT_DESCRIPTOR;

#define IMAGE_ORDINAL_FLAG64             0x8000000000000000UL
#define IMAGE_SNAP_BY_ORDINAL64(Ordinal) ((Ordinal & IMAGE_ORDINAL_FLAG64) != 0)
#define IMAGE_ORDINAL_FLAG               IMAGE_ORDINAL_FLAG64
#define IMAGE_SNAP_BY_ORDINAL            IMAGE_SNAP_BY_ORDINAL64

typedef struct _IMAGE_BASE_RELOCATION
{
    DWORD   VirtualAddress;
    DWORD   SizeOfBlock;
    WORD    TypeOffset[0];
} IMAGE_BASE_RELOCATION,*PIMAGE_BASE_RELOCATION;

#define IMAGE_REL_BASED_ABSOLUTE                0
#define IMAGE_REL_BASED_HIGHLOW                 3
#define IMAGE_REL_BASED_DIR64                   10

typedef struct _IMAGE_TLS_DIRECTORY {
    PVOID RawDataStart;
    PVOID RawDataEnd;
    PDWORD AddressOfIndex;
    PVOID AddressOfCallbacks;
    DWORD SizeOfZeroFill;
    DWORD Characteristics;
} IMAGE_TLS_DIRECTORY, *PIMAGE_TLS_DIRECTORY;

typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID;

typedef struct _NT_TIB {
    PVOID ExceptionList;
    PVOID StackBase;
    PVOID StackLimit;
    PVOID SubSystemTib;
    ULONG Version;
    PVOID UserPointer;
    PVOID Self;
} NT_TIB, *PNT_TIB;

typedef struct _RTL_BITMAP {
    ULONG  SizeOfBitMap;
    LPBYTE Buffer;
} RTL_BITMAP, *PRTL_BITMAP;

typedef struct _LIST_ENTRY {
  struct _LIST_ENTRY *Flink;
  struct _LIST_ENTRY *Blink;
} LIST_ENTRY, *PLIST_ENTRY, PRLIST_ENTRY;

typedef struct _PEB_LDR_DATA
{
    ULONG               Length;
    BOOLEAN             Initialized;
    PVOID               SsHandle;
    LIST_ENTRY          InLoadOrderModuleList;
    LIST_ENTRY          InMemoryOrderModuleList;
    LIST_ENTRY          InInitializationOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _PEB
{
    BOOLEAN                      InheritedAddressSpace;           /* 0x00 */
    BOOLEAN                      ReadImageFileExecOptions;        /* 0x01 */
    BOOLEAN                      BeingDebugged;                   /* 0x02 */
    BOOLEAN                      SpareBool;                       /* 0x03 */
    HANDLE                       Mutant;                          /* 0x04 */
    HMODULE                      ImageBaseAddress;                /* 0x08 */
    PPEB_LDR_DATA                LdrData;                         /* 0x0c */
    PVOID                        ProcessParameters;               /* 0x10 */
    PVOID                        SubSystemData;                   /* 0x14 */
    HANDLE                       ProcessHeap;                     /* 0x18 */
    PVOID                        FastPebLock;                     /* 0x1c */
    PVOID                        FastPebLockRoutine;              /* 0x20 */
    PVOID                        FastPebUnlockRoutine;            /* 0x24 */
    ULONG                        EnvironmentUpdateCount;          /* 0x28 */
    PVOID                        KernelCallbackTable;             /* 0x2c */
    PVOID                        EventLogSection;                 /* 0x30 */
    PVOID                        EventLog;                        /* 0x34 */
    PVOID                        FreeList;                        /* 0x38 */
    ULONG                        TlsExpansionCounter;             /* 0x3c */
    PRTL_BITMAP                  TlsBitmap;                       /* 0x40 */
    ULONG                        TlsBitmapBits[2];                /* 0x44 */
    PVOID                        ReadOnlySharedMemoryBase;        /* 0x4c */
    PVOID                        ReadOnlySharedMemoryHeap;        /* 0x50 */
    PVOID                       *ReadOnlyStaticServerData;        /* 0x54 */
    PVOID                        AnsiCodePageData;                /* 0x58 */
    PVOID                        OemCodePageData;                 /* 0x5c */
    PVOID                        UnicodeCaseTableData;            /* 0x60 */
    ULONG                        NumberOfProcessors;              /* 0x64 */
    ULONG                        NtGlobalFlag;                    /* 0x68 */
    BYTE                         Spare2[4];                       /* 0x6c */
    LARGE_INTEGER                CriticalSectionTimeout;          /* 0x70 */
    ULONG                        HeapSegmentReserve;              /* 0x78 */
    ULONG                        HeapSegmentCommit;               /* 0x7c */
    ULONG                        HeapDeCommitTotalFreeThreshold;  /* 0x80 */
    ULONG                        HeapDeCommitFreeBlockThreshold;  /* 0x84 */
    ULONG                        NumberOfHeaps;                   /* 0x88 */
    ULONG                        MaximumNumberOfHeaps;            /* 0x8c */
    PVOID                       *ProcessHeaps;                    /* 0x90 */
    PVOID                        GdiSharedHandleTable;            /* 0x94 */
    PVOID                        ProcessStarterHelper;            /* 0x98 */
    PVOID                        GdiDCAttributeList;              /* 0x9c */
    PVOID                        LoaderLock;                      /* 0xa0 */
    ULONG                        OSMajorVersion;                  /* 0xa4 */
    ULONG                        OSMinorVersion;                  /* 0xa8 */
    ULONG                        OSBuildNumber;                   /* 0xac */
    ULONG                        OSPlatformId;                    /* 0xb0 */
    ULONG                        ImageSubSystem;                  /* 0xb4 */
    ULONG                        ImageSubSystemMajorVersion;      /* 0xb8 */
    ULONG                        ImageSubSystemMinorVersion;      /* 0xbc */
    ULONG                        ImageProcessAffinityMask;        /* 0xc0 */
    ULONG                        GdiHandleBuffer[34];             /* 0xc4 */
    ULONG                        PostProcessInitRoutine;          /* 0x14c */
    PRTL_BITMAP                  TlsExpansionBitmap;              /* 0x150 */
    ULONG                        TlsExpansionBitmapBits[32];      /* 0x154 */
    ULONG                        SessionId;                       /* 0x1d4 */
} PEB, *PPEB;

typedef struct _TEB {
    NT_TIB      Tib;
    PVOID       EnvironmentPointer;
    CLIENT_ID   Cid;
    PVOID       ActiveRpcInfo;
    PVOID       ThreadLocalStoragePointer;
    PPEB        ProcessEnvironmentBlock;
} TEB;

struct pe_exports {
    char *dll;
    char *name;
    generic_func addr;
};

struct pe_image {
    char name[1024];
    BOOL WINAPI (*entry)(PVOID hinstDLL, DWORD fdwReason, PVOID lpvReserved);
    void *image;
    void *dl_handle;
    size_t size;
    int type;
    struct pe_exports *pe_exports;
    int num_pe_exports;
    struct pe_hsearch_data crtexports;
    IMAGE_NT_HEADERS *nt_hdr;
    IMAGE_OPTIONAL_HEADER *opt_hdr;
};

void PeLoadCrtExports(struct pe_image *pe);
void *LoadLibrary(const char *filename);
bool FreeLibrary(void *handle);
bool NativeLibrary(void *peimage);
void *GetProcAddress(void *peimage, const char *name);

#endif
