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

#endif
