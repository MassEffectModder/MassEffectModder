/* The MIT License
 *
 * Copyright (c) 2017-2019 Pawel Kolodziejski
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#include <unistd.h>
#elif defined(__linux__)
#include <sys/sysinfo.h>
#include <unistd.h>
#else
#error not supported system!
#endif

#define GIGABYTES (1024ULL * 1024 * 1024)
#define ALIGN_GIGABYTES (1024ULL * 1024 * 1023)

int DetectAmountMemoryGB()
{
    int amountGB = 0;
#if defined(_WIN32)
    MEMORYSTATUSEX memoryStatusEx;
    memoryStatusEx.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memoryStatusEx);
    amountGB = static_cast<int>((memoryStatusEx.ullTotalPhys + ALIGN_GIGABYTES) / GIGABYTES);
#elif defined(__APPLE__)
    uint64_t physMem;
    size_t paramLen = sizeof(physMem);
    sysctlbyname("hw.memsize", &physMem, &paramLen, nullptr, 0);
    amountGB = static_cast<int>((physMem + ALIGN_GIGABYTES) / GIGABYTES);
#elif defined(__linux__)
    struct sysinfo sysInfo;
    sysinfo(&sysInfo);
    amountGB = static_cast<int>((static_cast<uint64_t>(sysInfo.totalram) * sysInfo.mem_unit + ALIGN_GIGABYTES) / GIGABYTES);
#endif

    return amountGB;
}

#define MAX_MSG_SIZE 4000

void ConsoleWrite(const QString &message)
{
#if defined(_WIN32)
    ::_putws(message.toStdWString().c_str());
#else
    ::puts(message.toStdString().c_str());
#endif
}

void ConsoleSync()
{
    ::fflush(stdout);
}

QString BaseName(const QString &path)
{
    int index = path.lastIndexOf("/");
    if (index == -1)
        return path;
    return path.mid(index + 1, -1);
}

QString DirName(const QString &path)
{
    int index = path.lastIndexOf("/");
    if (index == -1)
        return path;
    return path.left(index);
}

QString BaseNameWithoutExt(const QString &path)
{
    const QString str = BaseName(path);
    int index = str.lastIndexOf(".");
    if (index == -1)
        return str;
    return str.left(index);
}

QString GetFileExtension(const QString &path)
{
    const QString str = BaseName(path);
    int index = str.lastIndexOf(".");
    if (index == -1)
        return "";
    return str.mid(index + 1, -1);
}

bool DetectAdminRights()
{
    bool status;

#if defined(_WIN32)
    SID_IDENTIFIER_AUTHORITY authority = SECURITY_NT_AUTHORITY;
    PSID sid;
    BOOL state = AllocateAndInitializeSid(&authority, 2,
                                          SECURITY_BUILTIN_DOMAIN_RID,
                                          DOMAIN_ALIAS_RID_ADMINS,
                                          0, 0, 0, 0, 0, 0, &sid);
    if (state)
    {
        CheckTokenMembership(nullptr, sid, &state);
        FreeSid(sid);
    }
    status = state;
#else
    status = (geteuid() == 0);
#endif

    return status;
}
