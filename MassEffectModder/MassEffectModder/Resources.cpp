/*
 * MassEffectModder
 *
 * Copyright (C) 2018 Pawel Kolodziejski <aquadran at users.sourceforge.net>
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

#include "Resources.h"
#include "Helpers/MemoryStream.h"
#include "Helpers/FileStream.h"
#include "Wrappers.h"

void Resources::loadMD5Tables()
{
    if (MD5tablesLoaded)
        CRASH();

    ByteBuffer decompressed;
    ByteBuffer compressed;
    {
        auto tmp = FileStream(":/Resources/MD5EntriesME1.bin", FileMode::Open, FileAccess::ReadOnly);
        tmp.SkipInt32();
        decompressed = ByteBuffer(tmp.ReadInt32());
        compressed = tmp.ReadToBuffer((uint)tmp.Length() - 8);
        uint dstLen = decompressed.size();
        ZlibDecompress(compressed.ptr(), compressed.size(), decompressed.ptr(), &dstLen);
        if (decompressed.size() != dstLen)
            CRASH();
    }

    {
        auto tmp = MemoryStream(decompressed);
        int count = tmp.ReadInt32();
        for (int l = 0; l < count; l++)
        {
            QString pkg;
            tmp.ReadStringASCIINull(pkg);
            pkg.replace(QChar('\\'), QChar('/'), Qt::CaseInsensitive);
            tablePkgsME1.push_back(pkg);
        }
        count = tmp.ReadInt32();
        for (int l = 0; l < count; l++)
        {
            MD5FileEntry entry{};
            entry.path = tablePkgsME1[tmp.ReadInt32()].toLower();
            entry.size = tmp.ReadInt32();
            tmp.ReadToBuffer(entry.md5, 16);
            entriesME1.push_back(entry);
        }
    }
    decompressed.Free();
    compressed.Free();


    {
        FileStream tmp = FileStream(":/Resources/MD5EntriesME1PL.bin", FileMode::Open, FileAccess::ReadOnly);
        tmp.SkipInt32();
        decompressed = ByteBuffer(tmp.ReadInt32());
        compressed = tmp.ReadToBuffer((uint)tmp.Length() - 8);
        uint dstLen = decompressed.size();
        ZlibDecompress(compressed.ptr(), compressed.size(), decompressed.ptr(), &dstLen);
        if (decompressed.size() != dstLen)
            CRASH();
    }
    {
        MemoryStream tmp = MemoryStream(decompressed);
        int count = tmp.ReadInt32();
        for (int l = 0; l < count; l++)
        {
            QString pkg;
            tmp.ReadStringASCIINull(pkg);
            pkg.replace(QChar('\\'), QChar('/'), Qt::CaseInsensitive);
            tablePkgsME1PL.push_back(pkg);
        }
        count = tmp.ReadInt32();
        for (int l = 0; l < count; l++)
        {
            MD5FileEntry entry{};
            entry.path = tablePkgsME1PL[tmp.ReadInt32()].toLower();
            entry.size = tmp.ReadInt32();
            tmp.ReadToBuffer(entry.md5, 16);
            entriesME1.push_back(entry);
        }
        std::sort(entriesME1.begin(), entriesME1.end(), SortComparePath);
    }
    decompressed.Free();
    compressed.Free();


    {
        FileStream tmp = FileStream(":/Resources/MD5EntriesME2.bin", FileMode::Open, FileAccess::ReadOnly);
        tmp.SkipInt32();
        decompressed = ByteBuffer(tmp.ReadInt32());
        compressed = tmp.ReadToBuffer((uint)tmp.Length() - 8);
        uint dstLen = decompressed.size();
        ZlibDecompress(compressed.ptr(), compressed.size(), decompressed.ptr(), &dstLen);
        if (decompressed.size() != dstLen)
            CRASH();
    }
    {
        MemoryStream tmp = MemoryStream(decompressed);
        int count = tmp.ReadInt32();
        for (int l = 0; l < count; l++)
        {
            QString pkg;
            tmp.ReadStringASCIINull(pkg);
            pkg.replace(QChar('\\'), QChar('/'), Qt::CaseInsensitive);
            tablePkgsME2.push_back(pkg);
        }
        count = tmp.ReadInt32();
        for (int l = 0; l < count; l++)
        {
            MD5FileEntry entry{};
            entry.path = tablePkgsME2[tmp.ReadInt32()].toLower();
            entry.size = tmp.ReadInt32();
            tmp.ReadToBuffer(entry.md5, 16);
            entriesME2.push_back(entry);
        }
        std::sort(entriesME2.begin(), entriesME2.end(), SortComparePath);
    }
    decompressed.Free();
    compressed.Free();


    {
        FileStream tmp = FileStream(":/Resources/MD5EntriesME3.bin", FileMode::Open, FileAccess::ReadOnly);
        tmp.SkipInt32();
        decompressed = ByteBuffer(tmp.ReadInt32());
        compressed = tmp.ReadToBuffer((uint)tmp.Length() - 8);
        uint dstLen = decompressed.size();
        ZlibDecompress(compressed.ptr(), compressed.size(), decompressed.ptr(), &dstLen);
        if (decompressed.size() != dstLen)
            CRASH();
    }
    {
        MemoryStream tmp = MemoryStream(decompressed);
        int count = tmp.ReadInt32();
        for (int l = 0; l < count; l++)
        {
            QString pkg;
            tmp.ReadStringASCIINull(pkg);
            pkg.replace(QChar('\\'), QChar('/'), Qt::CaseInsensitive);
            tablePkgsME3.push_back(pkg);
        }
        count = tmp.ReadInt32();
        for (int l = 0; l < count; l++)
        {
            MD5FileEntry entry{};
            entry.path = tablePkgsME3[tmp.ReadInt32()].toLower();
            entry.size = tmp.ReadInt32();
            tmp.ReadToBuffer(entry.md5, 16);
            entriesME3.push_back(entry);
        }
        std::sort(entriesME3.begin(), entriesME3.end(), SortComparePath);
    }
    decompressed.Free();
    compressed.Free();

    MD5tablesLoaded = true;
}

void Resources::unloadMD5Tables()
{
    if (!MD5tablesLoaded)
        return;

    tablePkgsME1.clear();
    tablePkgsME1PL.clear();
    tablePkgsME2.clear();
    tablePkgsME3.clear();
    entriesME1.clear();
    entriesME2.clear();
    entriesME3.clear();

    MD5tablesLoaded = false;
}
