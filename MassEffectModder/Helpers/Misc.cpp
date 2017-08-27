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

#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#elif defined(__linux__)
#include <sys/sysinfo.h>
#else
#error not supported system!
#endif

#define GIGABYTES (1024 * 1024 * 1024)

int DetectAmountMemoryGB()
{
    int amountGB = 0;
#if defined(_WIN32)
    MEMORYSTATUSEX memoryStatusEx;
    memoryStatusEx.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memoryStatusEx);
    amountGB = memoryStatusEx.ullTotalPhys / GIGABYTES;
#elif defined(__APPLE__)
    uint64_t physMem;
    size_t paramLen = sizeof(physMem);
    sysctlbyname("hw.memsize", &physMem, &paramLen, NULL, 0);
    amountGB = physMem / GIGABYTES;
#elif defined(__linux__)
    struct sysinfo sysInfo;
    sysinfo(&sysInfo);
    amountGB = ((uint64_t)sysInfo.totalram * sys_info.mem_unit) / GIGABYTES;
#endif

    return amountGB;
}
