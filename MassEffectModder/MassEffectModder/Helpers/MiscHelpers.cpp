/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2021 Pawel Kolodziejski
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
#include <cwchar>
#include <cstdio>

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
    std::fputws((message + "\n").toStdWString().c_str(), stdout);
#else
    std::fputs((message + "\n").toUtf8().data(), stdout);
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
    QString str = BaseName(path);
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

#if defined(_WIN32)
QString getVersionString(const QString &filePath)
{
    QString versionString;
    DWORD infoSize = GetFileVersionInfoSize(filePath.toStdWString().c_str(), nullptr);
    if (infoSize == 0)
    {
        return versionString;
    }

    auto dataInfo = new quint8[infoSize];
    if (!GetFileVersionInfo(filePath.toStdWString().c_str(), 0, infoSize, dataInfo))
    {
        delete[] dataInfo;
        return versionString;
    }

    VS_FIXEDFILEINFO *fileInfo = nullptr;
    UINT puLen;
    if (!VerQueryValue(dataInfo, L"\\", (LPVOID *)&fileInfo, &puLen))
    {
        delete[] dataInfo;
        return versionString;
    }

    versionString =
            QString::number((fileInfo->dwFileVersionMS >> 16) & 0xffff) + "." +
            QString::number((fileInfo->dwFileVersionMS) & 0xffff) + "." +
            QString::number((fileInfo->dwFileVersionLS >> 16) & 0xffff) + "." +
            QString::number((fileInfo->dwFileVersionLS) & 0xffff);
    delete[] dataInfo;

    return versionString;
}
#endif
