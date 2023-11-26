/*
 * Copyright (C) 2021 Pawel Kolodziejski
 * Copyright (C) 2017 Tavis Ormandy
 *
 * Portions of this code are based on ndiswrapper, which included this
 * notice:
 *
 * Copyright (C) 2003-2005 Pontus Fuchs, Giridhar Pemmasani
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <search.h>
#include <stdlib.h>
#include <assert.h>
#include <err.h>
#ifdef linux
#include <asm/prctl.h>
#include <asm/unistd.h>
#endif

#include "pe_linker.h"
#include "log.h"

#define DRIVER_NAME "pelinker"
#define RVA2VA(image, rva, type) (type)(ULONG_PTR)((void *)(image) + (rva))

#ifndef NDEBUG

#define DBGLINKER(fmt, ...) printf("%s (%s:%d): " fmt "\n",     \
                                   DRIVER_NAME, __func__,               \
                                   __LINE__ , ## __VA_ARGS__);
#define TRACE1(fmt, ...) printf("%s (%s:%d): " fmt "\n",        \
                                   DRIVER_NAME, __func__,               \
                                   __LINE__ , ## __VA_ARGS__);
#else

#define DBGLINKER(fmt, ...)
#define TRACE1(fmt, ...)

#endif

#define ERROR(fmt, ...) printf("%s (%s:%d): " fmt "\n", \
                                   DRIVER_NAME, __func__,               \
                                   __LINE__ , ## __VA_ARGS__);

#define TLS_MINIMUM_AVAILABLE 64

DWORD64 PeLocalStorage[TLS_MINIMUM_AVAILABLE] = { 0 };

static void clearexports(struct pe_image *pe)
{
    pe_hdestroy_r(&pe->crtexports);
}

static int get_export(struct pe_image *pe, const char *name, void *result)
{
    ENTRY key = { (char *)(name), 0 }, *item;
    int i;
    void **func = result;

    if (pe->crtexports.size) {
        if (pe_hsearch_r(key, FIND, &item, &pe->crtexports)) {
            *func = item->data;
            return 0;
        }
    }

    for (i = 0; i < pe->num_pe_exports; i++) {
        if (strcmp(pe->pe_exports[i].name, name) == 0) {
            *func = pe->pe_exports[i].addr;
            return 0;
        }
    }

    return -1;
}

/*
 * Find and validate the coff header
 *
 */
int check_nt_hdr(IMAGE_NT_HEADERS *nt_hdr)
{
    WORD attr;
    PIMAGE_OPTIONAL_HEADER opt_hdr;

    /* Validate the "PE\0\0" signature */
    if (nt_hdr->Signature != IMAGE_NT_SIGNATURE) {
        ERROR("is this driver file? bad signature %08x", nt_hdr->Signature);
        return -EINVAL;
    }

    opt_hdr = &nt_hdr->OptionalHeader;

    if (opt_hdr->Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        ERROR("bad magic: %04X", opt_hdr->Magic);
        return -EINVAL;
    }

    /* Validate the image for the current architecture. */
    if (nt_hdr->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) {
        ERROR(" (PE signature %04X not supported)", nt_hdr->FileHeader.Machine);
        return -EINVAL;
    }

    /* Must have attributes */
    attr = IMAGE_FILE_EXECUTABLE_IMAGE;

    if ((nt_hdr->FileHeader.Characteristics & attr) != attr)
        return -EINVAL;

    /* Must be relocatable */
    attr = IMAGE_FILE_RELOCS_STRIPPED;
    if ((nt_hdr->FileHeader.Characteristics & attr))
        return -EINVAL;

    /* Make sure we have at least one section */
    if (nt_hdr->FileHeader.NumberOfSections == 0)
        return -EINVAL;

    if (opt_hdr->SectionAlignment < opt_hdr->FileAlignment) {
        ERROR("alignment mismatch: section: 0x%x, file: 0x%x", opt_hdr->SectionAlignment, opt_hdr->FileAlignment);
        return -EINVAL;
    }

    if ((nt_hdr->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE))
        return IMAGE_FILE_EXECUTABLE_IMAGE;
    if ((nt_hdr->FileHeader.Characteristics & IMAGE_FILE_DLL))
        return IMAGE_FILE_DLL;
    return -EINVAL;
}

static int import(struct pe_image *pe, IMAGE_IMPORT_DESCRIPTOR *dirent, char *dll)
{
    ULONG_PTR *lookup_tbl, *address_tbl;
    char *symname = NULL;
    int i;
    generic_func adr;

    lookup_tbl = RVA2VA(pe->image, dirent->u.OriginalFirstThunk, ULONG_PTR *);
    address_tbl = RVA2VA(pe->image, dirent->FirstThunk, ULONG_PTR *);

    for (i = 0; lookup_tbl[i]; i++) {
        if (IMAGE_SNAP_BY_ORDINAL(lookup_tbl[i])) {
            ERROR("ordinal import not supported: %llu", (uint64_t)lookup_tbl[i]);
            address_tbl[i] = (ULONG_PTR)NULL;
            continue;
        }
        else {
            symname = RVA2VA(pe->image, ((lookup_tbl[i] & ~IMAGE_ORDINAL_FLAG) + 2), char *);
        }
        if (get_export(pe, symname, &adr) < 0) {
            ERROR("unknown symbol: %s:%s", dll, symname);
            address_tbl[i] = (ULONG_PTR)NULL;
            continue;
        } else {
            DBGLINKER("found symbol: %s:%s: addr: %p, rva = %llu",
                      dll, symname, adr, (uint64_t)address_tbl[i]);
            address_tbl[i] = (ULONG_PTR)adr;
        }
    }

    return 0;
}

static int read_exports(struct pe_image *pe)
{
    IMAGE_EXPORT_DIRECTORY *export_dir_table;
    DWORD i;
    uint32_t *name_table;
    uint16_t *ordinal_table;
    PIMAGE_OPTIONAL_HEADER opt_hdr;
    IMAGE_DATA_DIRECTORY *export_data_dir;

    opt_hdr = &pe->nt_hdr->OptionalHeader;
    export_data_dir = &opt_hdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];

    if (export_data_dir->Size == 0) {
        DBGLINKER("no exports");
        return 0;
    }

    export_dir_table = RVA2VA(pe->image, export_data_dir->VirtualAddress, IMAGE_EXPORT_DIRECTORY *);

    name_table = (unsigned int *)(pe->image + export_dir_table->AddressOfNames);
    ordinal_table = (uint16_t *)(pe->image + export_dir_table->AddressOfNameOrdinals);

    pe->pe_exports = calloc(export_dir_table->NumberOfNames, sizeof(struct pe_exports));

    for (i = 0; i < export_dir_table->NumberOfNames; i++) {
        uint32_t address = ((uint32_t *) (pe->image + export_dir_table->AddressOfFunctions))[*ordinal_table];

        if (export_data_dir->VirtualAddress <= address ||
                address >= (export_data_dir->VirtualAddress + export_data_dir->Size)) {
            DBGLINKER("forwarder rva");
        }

        DBGLINKER("export symbol: %s, at %p", (char *)(pe->image + *name_table), pe->image + address);

        pe->pe_exports[pe->num_pe_exports].dll = pe->name;
        pe->pe_exports[pe->num_pe_exports].name = pe->image + *name_table;
        pe->pe_exports[pe->num_pe_exports].addr = pe->image + address;

        pe->num_pe_exports++;
        name_table++;
        ordinal_table++;
    }

    return 0;
}

static int fixup_imports(struct pe_image *pe)
{
    int i;
    char *name;
    int ret = 0;
    IMAGE_IMPORT_DESCRIPTOR *dirent;
    IMAGE_DATA_DIRECTORY *import_data_dir;

    import_data_dir = &pe->opt_hdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    dirent = RVA2VA(pe->image, import_data_dir->VirtualAddress, IMAGE_IMPORT_DESCRIPTOR *);

    for (i = 0; dirent[i].Name; i++) {
        name = RVA2VA(pe->image, dirent[i].Name, char*);
        DBGLINKER("imports from dll: %s", name);
        ret += import(pe, &dirent[i], name);
    }

    return ret;
}

static int fixup_reloc(void *image, IMAGE_NT_HEADERS *nt_hdr)
{
    ULONG_PTR base;
    ULONG_PTR size;
    IMAGE_BASE_RELOCATION *fixup_block;
    IMAGE_DATA_DIRECTORY *base_reloc_data_dir;
    PIMAGE_OPTIONAL_HEADER opt_hdr;

    opt_hdr = &nt_hdr->OptionalHeader;
    base = opt_hdr->ImageBase;
    base_reloc_data_dir = &opt_hdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    if (base_reloc_data_dir->Size == 0)
        return 0;

    fixup_block = RVA2VA(image, base_reloc_data_dir->VirtualAddress, IMAGE_BASE_RELOCATION *);
    DBGLINKER("fixup_block=%p, image=%p", fixup_block, image);
    DBGLINKER("fixup_block info: %x %d", fixup_block->VirtualAddress, fixup_block->SizeOfBlock);

    while (fixup_block->SizeOfBlock) {
        DWORD i;
        WORD fixup, offset;

        size = (fixup_block->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);

        for (i = 0; i < size; i++) {
            fixup = fixup_block->TypeOffset[i];
            offset = fixup & 0xfff;
            switch ((fixup >> 12) & 0x0f) {
            case IMAGE_REL_BASED_ABSOLUTE:
                break;

            case IMAGE_REL_BASED_HIGHLOW: {
                    uint32_t addr;
                    uint32_t *loc =
                            RVA2VA(image,
                                   fixup_block->VirtualAddress +
                                   offset, uint32_t *);
                    addr = RVA2VA(image, (*loc - base), uint32_t);
                    *loc = addr;
                }
                break;

            case IMAGE_REL_BASED_DIR64: {
                    uint64_t addr;
                    uint64_t *loc = RVA2VA(image, fixup_block->VirtualAddress + offset, uint64_t *);
                    addr = RVA2VA(image, (*loc - base), uint64_t);
                    DBGLINKER("relocation: *%p (Val:%llX)= %llx", loc, *loc, addr);
                    *loc = addr;
                }
                break;

            default:
                ERROR("unknown fixup: %08X", (fixup >> 12) & 0x0f);
                return -EOPNOTSUPP;
                break;
            }
        }

        fixup_block = (IMAGE_BASE_RELOCATION *)((void *)fixup_block + fixup_block->SizeOfBlock);
    };

    return 0;
}

/* Expand the image in memory if necessary. The image on disk does not
 * necessarily maps the image of the driver in memory, so we have to
 * re-write it in order to fulfill the sections alignments. The
 * advantage to do that is that rva_to_va becomes a simple
 * addition. */
#define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))

static int fix_pe_image(struct pe_image *pe)
{
    void *image;
    IMAGE_SECTION_HEADER *sect_hdr;
    int i, sections;
    DWORD image_size;

    if (pe->size == pe->opt_hdr->SizeOfImage) {
        /* Nothing to do */
        return 0;
    }

    image_size = pe->opt_hdr->SizeOfImage;

    image      = mmap((PVOID)(pe->opt_hdr->ImageBase),
                      image_size + getpagesize(),
                      PROT_READ | PROT_WRITE | PROT_EXEC,
                      MAP_ANONYMOUS | MAP_PRIVATE,
                      -1,
                      0);

    if (image == MAP_FAILED) {
        ERROR("failed to mmap desired space for image: %d bytes, image base %#llx, %m",
            image_size, pe->opt_hdr->ImageBase);
        return -ENOMEM;
    }

    memset(image, 0, image_size);

    /* Copy all the headers, ie everything before the first section. */

    sections = pe->nt_hdr->FileHeader.NumberOfSections;
    sect_hdr = IMAGE_FIRST_SECTION(pe->nt_hdr);

    DBGLINKER("copying headers: %u bytes", sect_hdr->PointerToRawData);

    memcpy(image, pe->image, sect_hdr->PointerToRawData);

    /* Copy all the sections */
    for (i = 0; i < sections; i++) {
        DBGLINKER("Copy section %s from %x to %x",
                  sect_hdr->Name, sect_hdr->PointerToRawData,
                  sect_hdr->VirtualAddress);
        if (sect_hdr->VirtualAddress+sect_hdr->SizeOfRawData > image_size) {
            ERROR("Invalid section %s in driver", sect_hdr->Name);
            munmap(image, image_size + getpagesize());
            return -EINVAL;
        }

        memcpy(image+sect_hdr->VirtualAddress,
               pe->image + sect_hdr->PointerToRawData,
               sect_hdr->SizeOfRawData);
        sect_hdr++;
    }

    /* If the original is still there, clean it up.*/
    munmap(pe->image, pe->size);

    pe->image = image;
    pe->size = image_size;

    /* Update our internal pointers */
    pe->nt_hdr = (IMAGE_NT_HEADERS *)(pe->image + ((IMAGE_DOS_HEADER *)pe->image)->e_lfanew);
    pe->opt_hdr = &pe->nt_hdr->OptionalHeader;

    DBGLINKER("set nt headers: nt_hdr=%p, opt_hdr=%p, image=%p",
              pe->nt_hdr, pe->opt_hdr, pe->image);

    return 0;
}

static int link_pe_image(struct pe_image *pe)
{
    IMAGE_DOS_HEADER *dos_hdr = pe->image;

    if (pe->size < sizeof(IMAGE_DOS_HEADER)) {
        TRACE1("image too small: %zu", pe->size);
        return -EINVAL;
    }

    pe->nt_hdr = (IMAGE_NT_HEADERS *)(pe->image + dos_hdr->e_lfanew);
    pe->opt_hdr = &pe->nt_hdr->OptionalHeader;

    pe->type = check_nt_hdr(pe->nt_hdr);
    if (pe->type <= 0) {
        TRACE1("type <= 0");
        return -EINVAL;
    }

    if (fix_pe_image(pe)) {
        TRACE1("bad PE image");
        return -EINVAL;
    }

    if (read_exports(pe)) {
        TRACE1("read exports failed");
        return -EINVAL;
    }

    if (fixup_reloc(pe->image, pe->nt_hdr)) {
        TRACE1("fixup reloc failed");
        return -EINVAL;
    }

    if (fixup_imports(pe)) {
        TRACE1("fixup imports failed");
        return -EINVAL;
    }
    pe->entry = RVA2VA(pe->image, pe->opt_hdr->AddressOfEntryPoint, void *);
    TRACE1("entry is at %p, rva at %08X", pe->entry, pe->opt_hdr->AddressOfEntryPoint);

    /* Check if there were enough data directories for a TLS section.*/
    if (pe->opt_hdr->NumberOfRvaAndSizes >= IMAGE_DIRECTORY_ENTRY_TLS) {
        /* Normally, we would be expected to allocate a TLS slot,
           place the number into *TlsData->AddressOfIndex, and make
           it a pointer to RawData, and then process the callbacks.

           We don't support threads, so it seems safe to just
           pre-allocate a slot and point it straight to the
           template data.
        */
        PIMAGE_TLS_DIRECTORY TlsData = RVA2VA(pe->image,
                                              pe->opt_hdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress,
                                              IMAGE_TLS_DIRECTORY *);

        /* This means that slot 0 is reserved.*/
        PeLocalStorage[0] = (DWORD64) TlsData->RawDataStart;
    }

    return 0;
}

#ifdef linux
static ULONG TlsBitmapData[32];
static RTL_BITMAP TlsBitmap = {
    .SizeOfBitMap = sizeof(TlsBitmapData) * CHAR_BIT,
    .Buffer = (PVOID) &TlsBitmapData[0],
};

static bool setup_nt_threadinfo(void)
{
    static PEB ProcessEnvironmentBlock = {
        .TlsBitmap          = &TlsBitmap,
    };
    static TEB ThreadEnvironment = {
        .Tib.Self                   = &ThreadEnvironment.Tib,
        .ThreadLocalStoragePointer  = &PeLocalStorage,
        .ProcessEnvironmentBlock    = &ProcessEnvironmentBlock,
    };

    long set_tib_syscall_result = syscall(__NR_arch_prctl, ARCH_SET_GS, &ThreadEnvironment);
    if (set_tib_syscall_result != 0) {
        int error = errno;
        l_error("Failed to set the thread area. Error: %u", error);
        return false;
    }
    return true;
}
#endif

struct pe_image *LoadLibrary(const char *filename)
{
    int fd = -1;
    struct stat buf;
    struct pe_image *peimage;

    assert(filename);

    peimage = calloc(1, sizeof(struct pe_image));
    if (!peimage) {
        l_error("failed to alloc pe library instance %s, %m", filename);
        goto error;
    }
    if (strlen(filename) >= sizeof(peimage->name)) {
        goto error;
    }
    strcpy(peimage->name, filename);
    peimage->size = 0;
    peimage->image = MAP_FAILED;

    if ((fd = open(filename, O_RDWR)) < 0) {
        l_error("failed to open pe library %s, %m", filename);
        goto error;
    }

    /* Stat the file descriptor to determine filesize.*/
    if (fstat(fd, &buf) < 0) {
        l_error("failed to stat the specified pe library %s, %m", filename);
        goto error;
    }

    /* Attempt to map the file.*/
    peimage->size  = buf.st_size;
    peimage->image = mmap(NULL, peimage->size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, fd, 0);
    if (peimage->image == MAP_FAILED) {
        l_error("failed to map library %s, %m", filename);
        goto error;
    }

    /* If that succeeded, we can proceed.*/
    l_debug("successfully mapped %s@%p", filename, peimage->image);

    /* File descriptor no longer required.*/
    close(fd);

    // patch __alloca_probe function to return from function immediatelly
    // this is windows specific function to probe stack size
    // and not compatible on non Windows systems
    *(uint *)(peimage->image + 0x0000BC8A0) = 0x00CCCCC3; // retn

    PeLoadCrtExports(peimage);

    if (link_pe_image(peimage) != 0) {
        goto error;
    }

#ifdef linux
    if (!setup_nt_threadinfo()) {
        goto error;
    }
#endif

    if (!peimage->entry((PVOID)'ODLE', DLL_PROCESS_ATTACH, NULL)) {
        l_error("failed execution of library entry %s, %m", filename);
        goto error;
    }

    return peimage;

error:
    if (peimage && peimage->num_pe_exports) {
        clearexports(peimage);
        free(peimage->pe_exports);
        peimage->num_pe_exports = 0;
    }

    if (fd >= 0)
        close(fd);

    if (peimage && peimage->image != MAP_FAILED)
        munmap(peimage->image, peimage->size);

    if (peimage)
        free(peimage);

    return NULL;
}

void *GetProcAddress(struct pe_image *pe, const char *name)
{
    void *ptr = NULL;

    get_export(pe, name, &ptr);

    return ptr;
}

bool FreeLibrary(struct pe_image *pe)
{
    if (!pe)
        return false;

    // on macOS debug free might report some unknown heaps
    // skip it detach dll, library is loaded one time per process.
    // heaps will be released after process exit
    //if (pe->entry)
    //    pe->entry((PVOID)'ODLE', DLL_PROCESS_DETACH, NULL);

    if (pe->num_pe_exports) {
        clearexports(pe);
        free(pe->pe_exports);
        pe->num_pe_exports = 0;
    }

    munmap(pe->image, pe->size);

    free(pe);

    return true;
}
