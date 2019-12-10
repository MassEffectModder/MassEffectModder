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

#ifdef _WIN32
#include <direct.h>
#endif
#include <sys/stat.h>
#include <errno.h>


#define kInputBufSize ((size_t)1 << 18)

static const ISzAlloc g_Alloc = { SzAlloc, SzFree };


static void Print(const char *s)
{
  fputs(s, stdout);
}


static int Buf_EnsureSize(CBuf *dest, size_t size)
{
  if (dest->size >= size)
    return 1;
  Buf_Free(dest, &g_Alloc);
  return Buf_Create(dest, size, &g_Alloc);
}

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
        fwprintf(stderr, L"Error: failed to check directory: %s\n", name);
        return 1;
    }
    struct _stat s;
    memset(&s, 0, sizeof(struct _stat));
    _wstat(name, &s);
    if (error == 0 && !S_ISDIR(s.st_mode)) {
        fwprintf(stderr, L"Error: output path is not directory: %s\n", name);
        return 1;
    }
    if (error != 0 && !CreateDirectoryW(name, NULL)) {
        fwprintf(stderr, L"Error: failed to create directory: %s\n", name);
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

static SRes PrintString(const UInt16 *s)
{
  CBuf buf;
  SRes res;
  Buf_Init(&buf);
  res = Utf16_To_Char(&buf, s
#ifndef _USE_UTF8
      , CP_OEMCP
#endif
      );
  if (res == SZ_OK)
    Print((const char *)buf.data);
  Buf_Free(&buf, &g_Alloc);
  return res;
}

static void UInt64ToStr(UInt64 value, char *s, int numDigits)
{
  char temp[32];
  int pos = 0;
  do
  {
    temp[pos++] = (char)('0' + (unsigned)(value % 10));
    value /= 10;
  }
  while (value != 0);

  for (numDigits -= pos; numDigits > 0; numDigits--)
    *s++ = ' ';

  do
    *s++ = temp[--pos];
  while (pos);
  *s = '\0';
}

#define PERIOD_4 (4 * 365 + 1)
#define PERIOD_100 (PERIOD_4 * 25 - 1)
#define PERIOD_400 (PERIOD_100 * 4 + 1)

static void PrintLF()
{
  Print("\n");
}

static void PrintError(char *s)
{
  Print("\nERROR: ");
  Print(s);
  PrintLF();
}

#ifdef USE_WINDOWS_FILE
int sevenzip_unpack(const wchar_t *path, const wchar_t *output_path, int full_path)
#else
int sevenzip_unpack(const char *path, const char *output_path, int full_path)
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
        PrintError("can not open input file");
        return 1;
    }

    FileInStream_CreateVTable(&archiveStream);
    LookToRead2_CreateVTable(&lookStream, False);
    lookStream.buf = NULL;

    res = SZ_OK;
    lookStream.buf = ISzAlloc_Alloc(&allocImp, kInputBufSize);
    if (!lookStream.buf)
        res = SZ_ERROR_MEM;
    else
    {
        lookStream.bufSize = kInputBufSize;
        lookStream.realStream = &archiveStream.vt;
        LookToRead2_Init(&lookStream);
    }

    CrcGenerateTable();

    SzArEx_Init(&db);

    if (res == SZ_OK)
    {
        res = SzArEx_Open(&db, &lookStream.vt, &allocImp, &allocTempImp);
    }

    if (res == SZ_OK)
    {
        UInt32 i;

        /*
         if you need cache, use these 3 variables.
         if you use external function, you can make these variable as static.
        */
        UInt32 blockIndex = 0xFFFFFFFF; /* it can have any value before first call (if outBuffer = 0) */
        Byte *outBuffer = 0; /* it must be 0 before first call for each new archive. */
        size_t outBufferSize = 0;  /* it can have any value before first call (if outBuffer = 0) */

        for (i = 0; i < db.NumFiles; i++)
        {
            size_t offset = 0;
            size_t outSizeProcessed = 0;
            size_t len;
            unsigned isDir = SzArEx_IsDir(&db, i);
            if (isDir && !full_path)
                continue;
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

            res = PrintString(temp);
            if (res != SZ_OK)
                break;

            if (isDir)
            {
                continue;
            }

            res = SzArEx_Extract(&db, &lookStream.vt, i,
                                &blockIndex, &outBuffer, &outBufferSize,
                                &offset, &outSizeProcessed,
                                &allocImp, &allocTempImp);
            if (res != SZ_OK)
                break;

            CSzFile outFile;
            size_t processedSize;
            size_t j;
#ifdef USE_WINDOWS_FILE
            wchar_t *outputDir = (UInt16 *)output_path;
            wchar_t *name = (UInt16 *)temp;
            int size = wcslen(outputDir) + wcslen(name) + 2;
            wchar_t outputPath[size];

            for (j = 0; name[j] != 0; j++)
            {
                if (name[j] == '/')
                {
                    if (full_path)
                    {
                        name[j] = 0;
                        if (outputDir[0] != 0)
                            swprintf(outputPath, size, L"%s/%s", outputDir, name);
                        else
                            wcscpy(outputPath, name);
                        if (MyCreateDir(outputPath) != 0)
                        {
                            res = SZ_ERROR_FAIL;
                            break;
                        }
                        name[j] = '/';
                    }
                    else
                    {
                        if (outputDir[0] != 0)
                            swprintf(outputPath, size, L"%s/%s", outputDir, name + j + 1);
                        else
                            wcscpy(outputPath, name + j + 1);
                    }
                }
            }

            if (full_path)
            {
                if (outputDir[0] != 0)
                    swprintf(outputPath, size, L"%s/%s", outputDir, name);
                else
                    wcscpy(outputPath, name);
            }

            if (OutFile_OpenW(&outFile, outputPath))
            {
                PrintError("can not open output file");
                res = SZ_ERROR_FAIL;
                break;
            }
#else
            CBuf buf;
            SRes res;
            Buf_Init(&buf);
            res = Utf16_To_Char(&buf, temp);
            char FileName[buf.size + 1];
            strcpy(FileName, (const char*)buf.data);
            Buf_Free(&buf, &g_Alloc);

            char full_file_path[strlen(output_path) + strlen(FileName) + 2];
            char tmpfile[strlen(FileName) + 1];
            strcpy(tmpfile, FileName);
            for (j = 0; tmpfile[j] != 0; j++)
            {
                if (tmpfile[j] == '/')
                {
                    if (full_path)
                    {
                        tmpfile[j] = 0;
                        if (output_path[0] != 0)
                            sprintf(full_file_path, "%s/%s", output_path, tmpfile);
                        else
                            strcpy(full_file_path, tmpfile);
                        if (MyCreateDir(full_file_path) != 0)
                        {
                            res = SZ_ERROR_FAIL;
                            break;
                        }
                        tmpfile[j] = '/';
                    }
                    else
                    {
                        if (output_path[0] != 0)
                            sprintf(full_file_path, "%s/%s", output_path, tmpfile + j + 1);
                        else
                            strcpy(full_file_path, tmpfile + j + 1);
                    }
                }
            }
            if (res != SZ_OK)
            {
                res = SZ_ERROR_FAIL;
                break;
            }

            if (output_path[0] != 0)
                sprintf(full_file_path, "%s/%s", output_path, FileName);
            else
                strcpy(full_file_path, FileName);

            if (OutFile_Open(&outFile, full_file_path))
            {
                PrintError("can not open output file");
                res = SZ_ERROR_FAIL;
                break;
            }
#endif

            processedSize = outSizeProcessed;

            if (File_Write(&outFile, outBuffer + offset, &processedSize) != 0 || processedSize != outSizeProcessed)
            {
                PrintError("can not write output file");
                res = SZ_ERROR_FAIL;
                break;
            }

            if (File_Close(&outFile))
            {
                PrintError("can not close output file");
                res = SZ_ERROR_FAIL;
                break;
            }
            PrintLF();
        }

        ISzAlloc_Free(&allocImp, outBuffer);

        SzFree(NULL, temp);
        SzArEx_Free(&db, &allocImp);
        ISzAlloc_Free(&allocImp, lookStream.buf);

        File_Close(&archiveStream.file);
    }

    if (res == SZ_OK)
    {
        Print("\nEverything is Ok\n");
        return 0;
    }

    if (res == SZ_ERROR_UNSUPPORTED)
        PrintError("decoder doesn't support this archive");
    else if (res == SZ_ERROR_MEM)
        PrintError("can not allocate memory");
    else if (res == SZ_ERROR_CRC)
        PrintError("CRC error");
    else
    {
        char s[32];
        UInt64ToStr(res, s, 0);
        PrintError(s);
    }

    return 1;
}
