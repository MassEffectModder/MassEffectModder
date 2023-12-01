/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2022 Pawel Kolodziejski
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

#ifndef MISC_HELPERS_H
#define MISC_HELPERS_H

#include <Types/MemTypes.h>

int DetectAmountMemoryGB();
void ConsoleWrite(const QString &message);
void ConsoleSync();
QString BaseName(const QString &path);
QString DirName(const QString &path);
QString BaseNameWithoutExt(const QString &path);
QString GetFileExtension(const QString &path);
QStringList FilterByFilename(const QStringList &list, const QString &filename);

inline bool AsciiStringEndsWith(const QString &str, const char *endStr, int endStrLen)
{
    int strLen = str.length();
    if (endStrLen > strLen)
        return false;

    auto strData = reinterpret_cast<const ushort *>(str.data());
    for (int l = endStrLen - 1; l >= 0; l--)
    {
        char c = static_cast<char>(strData[l + (strLen - endStrLen)]);
        if (c >= 'A' && c <= 'Z')
            c += 0x20;
        if (c != endStr[l])
            return false;
    }
    return true;
}

inline bool AsciiBaseNameStringStartsWith(const QString &path, const char *startStr, int startStrLen)
{
    auto strData = reinterpret_cast<const ushort *>(path.data());
    int pos = -1;
    for (int l = 0; l < path.length(); l++)
    {
        char c = static_cast<char>(strData[l]);
        if (c == '/')
        {
            pos = l;
        }
    }

    if (startStrLen > (path.length() - (pos + 1)))
        return false;

    for (int l = 0; l < startStrLen; l++)
    {
        char c = static_cast<char>(strData[pos + l + 1]);
        if (c >= 'A' && c <= 'Z')
            c += 0x20;
        if (c != startStr[l])
            return false;
    }
    return true;
}

inline bool AsciiBaseNameStringContains(const QString &path, const char *containsStr, int containsStrLen)
{
    auto strData = reinterpret_cast<const ushort *>(path.data());
    int pos = -1;
    for (int l = 0; l < path.length(); l++)
    {
        char c = static_cast<char>(strData[l]);
        if (c == '/')
        {
            pos = l;
        }
    }

    int baseLen = path.length() - (pos + 1);
    for (int b = 0; b < baseLen; b++)
    {
        if (containsStrLen > (baseLen - b))
            return false;
        bool found = true;
        for (int l = 0; l < containsStrLen; l++)
        {
            char c = static_cast<char>(strData[pos + l + b + 1]);
            if (c >= 'A' && c <= 'Z')
                c += 0x20;
            if (c != containsStr[l])
            {
                found = false;
                break;
            }
        }
        if (found)
            return true;
    }
    return false;
}

inline bool AsciiStringMatchCaseIgnore(const QString &str1, const QString &str2)
{
    if (str1.length() != str2.length())
        return false;
    auto strData1 = reinterpret_cast<const ushort *>(str1.data());
    auto strData2 = reinterpret_cast<const ushort *>(str2.data());
    for (int l = 0; l < str1.length(); l++)
    {
        char c1 = static_cast<char>(strData1[l]);
        char c2 = static_cast<char>(strData2[l]);
        if (c1 >= 'A' && c1 <= 'Z')
            c1 += 0x20;
        if (c2 >= 'A' && c2 <= 'Z')
            c2 += 0x20;
        if (c1 != c2)
            return false;
    }
    return true;
}

inline bool AsciiStringMatch(const QString &str1, const QString &str2)
{
    if (str1.length() != str2.length())
        return false;
    auto strData1 = reinterpret_cast<const ushort *>(str1.data());
    auto strData2 = reinterpret_cast<const ushort *>(str2.data());
    for (int l = 0; l < str1.length(); l++)
    {
        char c1 = static_cast<char>(strData1[l]);
        char c2 = static_cast<char>(strData2[l]);
        if (c1 != c2)
            return false;
    }
    return true;
}

inline int AsciiStringCompare(const QString &str1, const QString &str2)
{
    auto strData1 = reinterpret_cast<const ushort *>(str1.data());
    auto strData2 = reinterpret_cast<const ushort *>(str2.data());
    for (int l = 0; l < MIN(str1.length(), str2.length()); l++)
    {
        char c1 = static_cast<char>(strData1[l]);
        char c2 = static_cast<char>(strData2[l]);
        if (c1 < c2)
            return -1;
        if (c1 > c2)
            return 1;
    }
    if (str1.length() < str2.length())
        return -1;
    if (str1.length() > str2.length())
        return 1;
    return 0;
}

inline int AsciiStringCompareCaseIgnore(const QString &str1, const QString &str2)
{
    auto strData1 = reinterpret_cast<const ushort *>(str1.data());
    auto strData2 = reinterpret_cast<const ushort *>(str2.data());
    for (int l = 0; l < MIN(str1.length(), str2.length()); l++)
    {
        char c1 = static_cast<char>(strData1[l]);
        char c2 = static_cast<char>(strData2[l]);
        if (c1 >= 'A' && c1 <= 'Z')
            c1 += 0x20;
        if (c2 >= 'A' && c2 <= 'Z')
            c2 += 0x20;
        if (c1 < c2)
            return -1;
        if (c1 > c2)
            return 1;
    }
    if (str1.length() < str2.length())
        return -1;
    if (str1.length() > str2.length())
        return 1;
    return 0;
}


bool DetectAdminRights();
bool DetectProcessTranslated ();

QString getVersionString(const QString &filePath);

#endif
