/* 2019 : Pawel Kolodziejski : Public domain */
/* 2018-08-04 : Igor Pavlov : Public domain */

#include "Precomp.h"

#include <stdio.h>
#include <string.h>

#include "CpuArch.h"

#include "7z.h"
#include "7zAlloc.h"
#include "7zBuf.h"
#include "7zCrc.h"
#include "7zFile.h"

#include "LzmaDec.h"
#include "Lzma2Dec.h"

#ifdef _WIN32
#include <direct.h>
#endif
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>

#define kInputBufSize ((size_t)1 << 18)

static const ISzAlloc g_Alloc = { SzAlloc, SzFree };

#ifndef _WIN32
static int Buf_EnsureSize(CBuf *dest, size_t size)
{
  if (dest->size >= size)
    return 1;
  Buf_Free(dest, &g_Alloc);
  return Buf_Create(dest, size, &g_Alloc);
}
#endif

#ifndef _WIN32
#define _USE_UTF8
#endif

/* #define _USE_UTF8 */

#ifdef _USE_UTF8

#define _UTF8_START(n) (0x100 - (1 << (7 - (n))))

#define _UTF8_RANGE(n) (((UInt32)1) << ((n) * 5 + 6))

#define _UTF8_HEAD(n, val) ((Byte)(_UTF8_START(n) + ((val) >> (6 * (n)))))
#define _UTF8_CHAR(n, val) ((Byte)(0x80 + (((val) >> (6 * (n))) & 0x3F)))

static size_t Utf16_To_Utf8_Calc(const UInt16 *src, const UInt16 *srcLim)
{
  size_t size = 0;
  for (;;)
  {
    UInt32 val;
    if (src == srcLim)
      return size;

    size++;
    val = *src++;

    if (val < 0x80)
      continue;

    if (val < _UTF8_RANGE(1))
    {
      size++;
      continue;
    }

    if (val >= 0xD800 && val < 0xDC00 && src != srcLim)
    {
      UInt32 c2 = *src;
      if (c2 >= 0xDC00 && c2 < 0xE000)
      {
        src++;
        size += 3;
        continue;
      }
    }

    size += 2;
  }
}

static Byte *Utf16_To_Utf8(Byte *dest, const UInt16 *src, const UInt16 *srcLim)
{
  for (;;)
  {
    UInt32 val;
    if (src == srcLim)
      return dest;

    val = *src++;

    if (val < 0x80)
    {
      *dest++ = (char)val;
      continue;
    }

    if (val < _UTF8_RANGE(1))
    {
      dest[0] = _UTF8_HEAD(1, val);
      dest[1] = _UTF8_CHAR(0, val);
      dest += 2;
      continue;
    }

    if (val >= 0xD800 && val < 0xDC00 && src != srcLim)
    {
      UInt32 c2 = *src;
      if (c2 >= 0xDC00 && c2 < 0xE000)
      {
        src++;
        val = (((val - 0xD800) << 10) | (c2 - 0xDC00)) + 0x10000;
        dest[0] = _UTF8_HEAD(3, val);
        dest[1] = _UTF8_CHAR(2, val);
        dest[2] = _UTF8_CHAR(1, val);
        dest[3] = _UTF8_CHAR(0, val);
        dest += 4;
        continue;
      }
    }

    dest[0] = _UTF8_HEAD(2, val);
    dest[1] = _UTF8_CHAR(1, val);
    dest[2] = _UTF8_CHAR(0, val);
    dest += 3;
  }
}

static SRes Utf16_To_Utf8Buf(CBuf *dest, const UInt16 *src, size_t srcLen)
{
  size_t destLen = Utf16_To_Utf8_Calc(src, src + srcLen);
  destLen += 1;
  if (!Buf_EnsureSize(dest, destLen))
    return SZ_ERROR_MEM;
  *Utf16_To_Utf8(dest->data, src, src + srcLen) = 0;
  return SZ_OK;
}

#endif

#ifndef _WIN32
static SRes Utf16_To_Char(CBuf *buf, const UInt16 *s
#ifndef _USE_UTF8
    , UINT codePage
#endif
    )
{
  unsigned len = 0;
  for (len = 0; s[len] != 0; len++)
      ;


#ifndef _USE_UTF8
  {
    unsigned size = len * 3 + 100;
    if (!Buf_EnsureSize(buf, size))
      return SZ_ERROR_MEM;
    {
      buf->data[0] = 0;
      if (len != 0)
      {
        char defaultChar = '_';
        BOOL defUsed;
        unsigned numChars = 0;
        numChars = WideCharToMultiByte(codePage, 0, s, len, (char *)buf->data, size, &defaultChar, &defUsed);
        if (numChars == 0 || numChars >= size)
          return SZ_ERROR_FAIL;
        buf->data[numChars] = 0;
      }
      return SZ_OK;
    }
  }
#else
  return Utf16_To_Utf8Buf(buf, s, len);
#endif
}
#endif

#ifdef _WIN32
#ifndef USE_WINDOWS_FILE
  static UINT g_FileCodePage = CP_ACP;
#endif
#define MY_FILE_CODE_PAGE_PARAM ,g_FileCodePage
#else
#define MY_FILE_CODE_PAGE_PARAM
#endif

#ifdef USE_WINDOWS_FILE
static int MyCreateDir(const wchar_t *name)
{
    errno_t error = _waccess_s(name, 0);
    if (error != 0 && errno != ENOENT) {
        fwprintf(stderr, L"Error: failed to check directory: %ls\n", name);
        return 1;
    }
    struct _stat s;
    memset(&s, 0, sizeof(struct _stat));
    _wstat(name, &s);
    if (error == 0 && !S_ISDIR(s.st_mode)) {
        fwprintf(stderr, L"Error: output path is not directory: %ls\n", name);
        return 1;
    }
    if (error != 0 && !CreateDirectoryW(name, NULL)) {
        fwprintf(stderr, L"Error: failed to create directory: %ls\n", name);
        return 1;
    }
    return 0;
}
#else
static int MyCreateDir(const char *name)
{
    struct stat s;
    memset(&s, 0, sizeof(stat));
    int error = stat(name, &s);
    if (error == -1 && errno != ENOENT) {
        fprintf(stderr, "Error: failed to check directory: %s\n", name);
        return 1;
    }
    if (error == 0 && !S_ISDIR(s.st_mode)) {
        fprintf(stderr, "Error: output path is not directory: %s\n", name);
        return 1;
    }
    if (error == -1 && mkdir(name, 0755) != 0) {
        fprintf(stderr, "Error: failed to create directory: %s\n", name);
        return 1;
    }
    return 0;
}
#endif

#define PERIOD_4 (4 * 365 + 1)
#define PERIOD_100 (PERIOD_4 * 25 - 1)
#define PERIOD_400 (PERIOD_100 * 4 + 1)

#define OUT_BUF_SIZE (1 << 20)

#if defined(_WIN32)
static int compareExt(wchar_t *filename, const wchar_t *ext)
{
    wchar_t *pExt = wcsrchr(filename, L'.');
    if (pExt == NULL)
        return 0;
    return _wcsicmp(&pExt[1], ext) == 0;
}
#else
static int compareExt(char *filename, const char *ext)
{
    char *pExt = strrchr(filename, '.');
    if (pExt == NULL)
        return 0;
    return strcasecmp(&pExt[1], ext) == 0;
}
#endif

static int g_ipc;
static int lastProgress;
static UInt64 progressUnpackedSize;
static UInt64 totalUnpackedSize;
static int totalFiles;
#ifdef USE_WINDOWS_FILE
static const wchar_t *filterExtension;
#else
static const char *filterExtension;
#endif

static void PrintProgressIpc(UInt64 processedBytes)
{
    if (g_ipc)
    {
        progressUnpackedSize += processedBytes;
        int newProgress = progressUnpackedSize * 100 / totalUnpackedSize;
        if (lastProgress != newProgress)
        {
            lastProgress = newProgress;
#if defined(_WIN32)
            wprintf(L"[IPC]TASK_PROGRESS %d\n", newProgress);
#else
            printf("[IPC]TASK_PROGRESS %d\n", newProgress);
#endif
            fflush(stdout);
        }
    }
}

static void PrintfCurrentFile(SzArEx_StreamOutEntry *streamOutInfo)
{
    if (filterExtension[0] == 0 || compareExt(streamOutInfo->path, filterExtension))
    {
        if (g_ipc)
        {
#if defined(_WIN32)
            wprintf(L"[IPC]FILENAME %ls\n", streamOutInfo->path);
#else
            printf("[IPC]FILENAME %s\n", streamOutInfo->path);
#endif
            fflush(stdout);
        }
        else
        {
#if defined(_WIN32)
            wprintf(L"%d of %d - %ls - size %llu\n", (streamOutInfo->entryIndex + 1),
                   totalFiles, streamOutInfo->path, streamOutInfo->UnpackSize);
#else
            printf("%d of %d - %s - size %llu\n", (streamOutInfo->entryIndex + 1),
                   totalFiles, streamOutInfo->path, streamOutInfo->UnpackSize);
#endif
        }
    }
}

#ifdef USE_WINDOWS_FILE
int sevenzip_unpack(const wchar_t *path, const wchar_t *output_path,
                    const wchar_t *filter, int full_path, int ipc)
#else
int sevenzip_unpack(const char *path, const char *output_path,
                    const char *filter, int full_path, int ipc)
#endif
{
    ISzAlloc allocImp;
    ISzAlloc allocTempImp;
    CFileInStream archiveStream;
    CLookToRead2 lookStream;
    CSzArEx db;
    SRes res;
    UInt16 *temp = NULL;
    size_t tempSize = 0;
    SzArEx_StreamOutEntry *streamOutInfo = NULL;

    g_ipc = ipc;
    lastProgress = -1;
    totalUnpackedSize = 0;
    progressUnpackedSize = 0;

#if defined(_WIN32) && !defined(USE_WINDOWS_FILE)
    g_FileCodePage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
#endif

    allocImp = g_Alloc;
    allocTempImp = g_Alloc;

#ifdef USE_WINDOWS_FILE
    if (InFile_OpenW(&archiveStream.file, path))
#else
    if (InFile_Open(&archiveStream.file, path))
#endif
    {
#ifdef USE_WINDOWS_FILE
        fwprintf(stderr, L"Error: Failed to open file!\n");
#else
        fprintf(stderr, "Error: Failed to open file!\n");
#endif
        return 1;
    }

    FileInStream_CreateVTable(&archiveStream);
    LookToRead2_CreateVTable(&lookStream, False);
    lookStream.buf = NULL;

    res = SZ_OK;
    lookStream.buf = ISzAlloc_Alloc(&allocImp, kInputBufSize);
    if (!lookStream.buf)
    {
        return 1;
    }

    lookStream.bufSize = kInputBufSize;
    lookStream.realStream = &archiveStream.vt;
    LookToRead2_Init(&lookStream);

    CrcGenerateTable();

    SzArEx_Init(&db);

    res = SzArEx_Open(&db, &lookStream.vt, &allocImp, &allocTempImp);
    if (res != SZ_OK)
    {
        if (res == SZ_ERROR_UNSUPPORTED)
#ifdef USE_WINDOWS_FILE
            fwprintf(stderr, L"Error: Decoder doesn't support this archive!");
#else
            fprintf(stderr, "Error: Decoder doesn't support this archive!");
#endif
        else if (res == SZ_ERROR_MEM)
#ifdef USE_WINDOWS_FILE
            fwprintf(stderr, L"Error: Can not allocate memory!");
#else
            fprintf(stderr, "Error: Can not allocate memory!");
#endif
        else if (res == SZ_ERROR_CRC)
#ifdef USE_WINDOWS_FILE
            fwprintf(stderr, L"Error: CRC error!");
#else
            fprintf(stderr, "Error: CRC error!");
#endif
        else
#ifdef USE_WINDOWS_FILE
            fwprintf(stderr, L"Error: Failed to open archive!\n");
#else
            fprintf(stderr, "Error: Failed to open archive!\n");
#endif
        return 1;
    }

    UInt32 i, f;
    size_t j;

    totalFiles = db.NumFiles;
    filterExtension = filter;
    streamOutInfo = (SzArEx_StreamOutEntry *)SzAlloc(NULL, (db.NumFiles + 1) * sizeof(SzArEx_StreamOutEntry));
    if (streamOutInfo == NULL)
    {
        return 1;
    }
    memset(streamOutInfo, 0, db.NumFiles * sizeof(SzArEx_StreamOutEntry));
    streamOutInfo[db.NumFiles].folderIndex = 0xFFFFFFFF;

    for (i = 0; i < db.NumFiles; i++)
    {
        size_t len;
        streamOutInfo[i].entryIndex = i;
        streamOutInfo[i].isDir = SzArEx_IsDir(&db, i);
        if (streamOutInfo[i].isDir)
        {
            continue;
        }
        len = SzArEx_GetFileNameUtf16(&db, i, NULL);

        if (len > tempSize)
        {
            SzFree(NULL, temp);
            tempSize = len;
            temp = (UInt16 *)SzAlloc(NULL, tempSize * sizeof(temp[0]));
            if (!temp)
            {
                res = SZ_ERROR_MEM;
                break;
            }
        }

        SzArEx_GetFileNameUtf16(&db, i, temp);

#ifdef USE_WINDOWS_FILE
        wchar_t *fileName = (UInt16 *)temp;
#else
        CBuf buf;
        Buf_Init(&buf);
        res = Utf16_To_Char(&buf, temp);
        char fileName[buf.size + 1];
        strcpy(fileName, (const char*)buf.data);
        Buf_Free(&buf, &g_Alloc);
#endif

#ifdef USE_WINDOWS_FILE
        wchar_t *outputDir = (UInt16 *)output_path;

        int size = wcslen(fileName) + 1;
        if (size > MAX_PATH)
        {
            fwprintf(stderr, L"Error: File name too long, aborting!\n");
            res = SZ_ERROR_FAIL;
            break;
        }

        int dest_size = wcslen(outputDir) + size + 1;
        if (dest_size > MAX_PATH)
        {
            fwprintf(stderr, L"Error: Destination path for file too long, aborting!\n");
            res = SZ_ERROR_FAIL;
            break;
        }

        wchar_t outputFile[PATH_MAX];
        if (outputDir && outputDir[0] != 0)
        {
            swprintf(outputFile, PATH_MAX - 1, L"%ls/%ls", outputDir, fileName);
        }
        else
        {
            wcsncpy(outputFile, fileName, PATH_MAX - 1);
        }

        if (filter[0] == 0 || compareExt(outputFile, filter))
        {
            wchar_t tmpPath[PATH_MAX];
            wcsncpy(tmpPath, fileName, MAX_PATH - 1);
            for (j = 0; tmpPath[j] != 0; j++)
            {
                if (tmpPath[j] == '/')
                {
                    if (full_path)
                    {
                        tmpPath[j] = 0;
                        wchar_t outputPath[PATH_MAX];
                        if (outputDir && outputDir[0] != 0)
                            swprintf(outputPath, PATH_MAX - 1, L"%ls/%ls", outputDir, tmpPath);
                        else
                            wcsncpy(outputPath, tmpPath, PATH_MAX - 1);
                        if (MyCreateDir(outputPath) != 0)
                        {
                            res = SZ_ERROR_FAIL;
                            break;
                        }
                        tmpPath[j] = '/';
                    }
                    else
                    {
                        if (outputDir && outputDir[0] != 0)
                            swprintf(outputFile, PATH_MAX - 1, L"%ls/%ls", outputDir, tmpPath + j + 1);
                        else
                            wcsncpy(outputFile, tmpPath + j + 1, PATH_MAX - 1);
                    }
                }
            }
        }
        else
        {
            streamOutInfo[i].filterOut = 1;
        }
        if (res != SZ_OK)
        {
            res = SZ_ERROR_FAIL;
            break;
        }

        wcsncpy(streamOutInfo[i].path, outputFile, PATH_MAX - 1);
#else
        char *outputDir = (char *)output_path;

        char outputFile[PATH_MAX];
        if (outputDir && outputDir[0] != 0)
        {
            snprintf(outputFile, PATH_MAX - 1, "%s/%s", outputDir, fileName);
        }
        else
        {
            strncpy(outputFile, fileName, PATH_MAX - 1);
        }

        if (filter[0] == 0 || compareExt(outputFile, filter))
        {
            char tmpPath[PATH_MAX];
            strncpy(tmpPath, fileName, PATH_MAX - 1);
            for (j = 0; tmpPath[j] != 0; j++)
            {
                if ((tmpPath[j] & 0xc0) == 0x80)
                {
                    continue;
                }
                if (tmpPath[j] == '/')
                {
                    if (full_path)
                    {
                        tmpPath[j] = 0;
                        char outputPath[PATH_MAX];
                        if (outputDir && outputDir[0] != 0)
                            snprintf(outputPath, PATH_MAX - 1, "%s/%s", outputDir, tmpPath);
                        else
                            strncpy(outputPath, tmpPath, PATH_MAX - 1);
                        if (MyCreateDir(outputPath) != 0)
                        {
                            res = SZ_ERROR_FAIL;
                            break;
                        }
                        tmpPath[j] = '/';
                    }
                    else
                    {
                        if (outputDir && outputDir[0] != 0)
                            snprintf(outputFile, PATH_MAX - 1, "%s/%s", (char *)outputDir, tmpPath + j + 1);
                        else
                            strncpy(outputFile, tmpPath + j + 1, PATH_MAX - 1);
                    }
                }
            }
        }
        else
        {
            streamOutInfo[i].filterOut = 1;
        }
        if (res != SZ_OK)
        {
            res = SZ_ERROR_FAIL;
            break;
        }

        strncpy(streamOutInfo[i].path, outputFile, PATH_MAX - 1);
#endif

        FileOutStream_CreateVTable(&streamOutInfo[i].outStream);
        File_Construct(&streamOutInfo[i].outStream.file);

        if (!streamOutInfo[i].filterOut)
        {
#ifdef USE_WINDOWS_FILE
            if (OutFile_OpenW(&streamOutInfo[i].outStream.file, streamOutInfo[i].path))
#else
            if (OutFile_Open(&streamOutInfo[i].outStream.file, streamOutInfo[i].path))
#endif
            {
                res = SZ_ERROR_WRITE;
                break;
            }
        }

        streamOutInfo[i].folderIndex = db.FileToFolder[i];
        streamOutInfo[i].UnpackPosition = db.UnpackPositions[i];
        streamOutInfo[i].UnpackSize = SzArEx_GetFileSize(&db, i);
        if (SzBitWithVals_Check(&db.CRCs, i))
        {
            streamOutInfo[i].CRC = db.CRCs.Vals[i];
        }
        else
        {
            streamOutInfo[i].CRC = CRC_INIT_VAL;
        }
        totalUnpackedSize += streamOutInfo[i].UnpackSize;
    }

    if (filter[0] != 0)
    {
        for (i = 0; i < db.NumFiles; i++)
        {
            if (streamOutInfo[i].folderIndex != 0xffffffff &&
                streamOutInfo[i].filterOut)
            {
                UInt32 l, f = streamOutInfo[i].folderIndex;
                int extractFolder = 0;
                for (l = 0; l < db.NumFiles; l++)
                {
                    if (streamOutInfo[l].folderIndex == f &&
                       !streamOutInfo[l].filterOut)
                    {
                        extractFolder = 1;
                        break;
                    }
                }
                if (!extractFolder)
                {
                    for (l = 0; l < db.NumFiles; l++)
                    {
                        if (streamOutInfo[l].folderIndex == f)
                        {
                            streamOutInfo[l].folderIndex = 0xffffffff;
                            totalUnpackedSize -= streamOutInfo[l].UnpackSize;
                        }
                    }
                }
            }
        }
    }

    for (f = 0; ; f++)
    {
        int foundFolder = 0;
        for (i = 0; i < db.NumFiles; i++)
        {
            if (streamOutInfo[i].folderIndex > f &&
                streamOutInfo[i].folderIndex != 0xffffffff)
            {
                foundFolder = 1;
            }
            if ((streamOutInfo[i].folderIndex != f && streamOutInfo[i].folderIndex != 0xffffffff) ||
                 streamOutInfo[i].folderIndex == 0xffffffff)
            {
                continue;
            }
            if (streamOutInfo[i].isDir)
            {
                if (!ipc)
                {
#if defined(_WIN32)
                    wprintf(L"%d of %d - %ls\n", (i + 1), db.NumFiles, streamOutInfo[i].path);
#else
                    printf("%d of %d - %s\n", (i + 1), db.NumFiles, streamOutInfo[i].path);
#endif
                }
            }
            else
            {
                res = SzArEx_ExtractFolderToStream(&db, &lookStream.vt, f,
                                                   &streamOutInfo[i], &allocTempImp,
                                                   PrintProgressIpc, PrintfCurrentFile);
                if (res == SZ_ERROR_UNSUPPORTED)
                {
                    UInt32 blockIndex = 0xFFFFFFFF;
                    Byte *outBuffer = 0;
                    size_t outBufferSize = 0;
                    size_t offset = 0;
                    size_t outSizeProcessed = 0;

                    PrintfCurrentFile(&streamOutInfo[i]);

                    res = SzArEx_Extract(&db, &lookStream.vt, i,
                                         &blockIndex, &outBuffer, &outBufferSize,
                                         &offset, &outSizeProcessed,
                                         &allocImp, &allocTempImp);
                    if (res == SZ_OK)
                    {
                        PrintProgressIpc(outSizeProcessed);

#ifdef USE_WINDOWS_FILE
                        if (streamOutInfo[i].outStream.file.handle != INVALID_HANDLE_VALUE)
#else
                        if (streamOutInfo[i].outStream.file.fd != -1)
#endif
                        {
                            SizeT outProcessed = outSizeProcessed;
                            if (streamOutInfo[i].outStream.vt.Write(&streamOutInfo[i].outStream.vt, outBuffer, outProcessed) != outProcessed)
                            {
                                res = SZ_ERROR_WRITE;
                            }
                        }
                        ISzAlloc_Free(&allocImp, outBuffer);
                    }
                }

                if (res == SZ_OK)
                {
                    foundFolder = 1;
                }
                break;
            }
        }
        if (!foundFolder)
            break;
    }

    for (i = 0; i < db.NumFiles; i++)
    {
        File_Close(&streamOutInfo[i].outStream.file);
    }

    SzFree(NULL, streamOutInfo);
    SzFree(NULL, temp);
    SzArEx_Free(&db, &allocImp);
    ISzAlloc_Free(&allocImp, lookStream.buf);

    if (res == SZ_OK)
    {
        return 0;
    }

    return 1;
}

#ifdef USE_WINDOWS_FILE
int sevenzip_list(const wchar_t *path, int ipc)
#else
int sevenzip_list(const char *path, int ipc)
#endif
{
    ISzAlloc allocImp;
    ISzAlloc allocTempImp;
    CFileInStream archiveStream;
    CLookToRead2 lookStream;
    CSzArEx db;
    SRes res;
    UInt16 *temp = NULL;
    size_t tempSize = 0;

    g_ipc = ipc;
    lastProgress = -1;
    totalUnpackedSize = 0;

#if defined(_WIN32) && !defined(USE_WINDOWS_FILE)
    g_FileCodePage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
#endif

    allocImp = g_Alloc;
    allocTempImp = g_Alloc;

#ifdef USE_WINDOWS_FILE
    if (InFile_OpenW(&archiveStream.file, path))
#else
    if (InFile_Open(&archiveStream.file, path))
#endif
    {
#ifdef USE_WINDOWS_FILE
        fwprintf(stderr, L"Error: Failed to open file!\n");
#else
        fprintf(stderr, "Error: Failed to open file!\n");
#endif
        return 1;
    }

    FileInStream_CreateVTable(&archiveStream);
    LookToRead2_CreateVTable(&lookStream, False);
    lookStream.buf = NULL;

    res = SZ_OK;
    lookStream.buf = ISzAlloc_Alloc(&allocImp, kInputBufSize);
    if (!lookStream.buf)
    {
        return 1;
    }

    lookStream.bufSize = kInputBufSize;
    lookStream.realStream = &archiveStream.vt;
    LookToRead2_Init(&lookStream);

    CrcGenerateTable();

    SzArEx_Init(&db);

    res = SzArEx_Open(&db, &lookStream.vt, &allocImp, &allocTempImp);
    if (res != SZ_OK)
    {
        if (res == SZ_ERROR_UNSUPPORTED)
#ifdef USE_WINDOWS_FILE
            fwprintf(stderr, L"Error: Decoder doesn't support this archive!");
#else
            fprintf(stderr, "Error: Decoder doesn't support this archive!");
#endif
        else if (res == SZ_ERROR_MEM)
#ifdef USE_WINDOWS_FILE
            fwprintf(stderr, L"Error: Can not allocate memory!");
#else
            fprintf(stderr, "Error: Can not allocate memory!");
#endif
        else if (res == SZ_ERROR_CRC)
#ifdef USE_WINDOWS_FILE
            fwprintf(stderr, L"Error: CRC error!");
#else
            fprintf(stderr, "Error: CRC error!");
#endif
        else
#ifdef USE_WINDOWS_FILE
            fwprintf(stderr, L"Error: Failed to open archive!\n");
#else
            fprintf(stderr, "Error: Failed to open archive!\n");
#endif
        return 1;
    }

    UInt32 i;

    for (i = 0; i < db.NumFiles; i++)
    {
        size_t len;
        if (SzArEx_IsDir(&db, i))
        {
            continue;
        }
        len = SzArEx_GetFileNameUtf16(&db, i, NULL);

        if (len > tempSize)
        {
            SzFree(NULL, temp);
            tempSize = len;
            temp = (UInt16 *)SzAlloc(NULL, tempSize * sizeof(temp[0]));
            if (!temp)
            {
                res = SZ_ERROR_MEM;
                break;
            }
        }

        SzArEx_GetFileNameUtf16(&db, i, temp);

#ifdef USE_WINDOWS_FILE
        wchar_t *fileName = (UInt16 *)temp;
#else
        CBuf buf;
        Buf_Init(&buf);
        res = Utf16_To_Char(&buf, temp);
        char fileName[buf.size + 1];
        strcpy(fileName, (const char*)buf.data);
        Buf_Free(&buf, &g_Alloc);
#endif

        if (ipc)
        {
#if defined(_WIN32)
            wprintf(L"[IPC]FILENAME %ls\n", fileName);
#else
            printf("[IPC]FILENAME %s\n", fileName);
#endif
            fflush(stdout);
        }
        else
        {
#if defined(_WIN32)
            wprintf(L"%ls\n", fileName);
#else
            printf("%s\n", fileName);
#endif
        }
    }

    SzFree(NULL, temp);
    SzArEx_Free(&db, &allocImp);
    ISzAlloc_Free(&allocImp, lookStream.buf);

    if (res == SZ_OK)
    {
        return 0;
    }

    return 1;
}
