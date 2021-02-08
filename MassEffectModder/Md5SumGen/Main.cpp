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

#include "MD5Entries.h"
#include <Helpers/FileStream.h>
#include <Helpers/MemoryStream.h>
#include <Wrappers.h>

bool g_ipc;

bool GetBackTrace(std::string & /*output*/, bool  /*exceptionMode*/, bool  /*crashMode*/)
{
    return true;
}

const unsigned int md5Tag = 0x5435444D;

void generateBinFile(int gameId)
{
    const MD5FileEntry *entries = nullptr;
    int entriesCount = 0;

    switch (gameId)
    {
        case 1:
            entries = entriesME1;
            entriesCount = MD5EntriesME1Size;
            break;
        case 2:
            entries = entriesME2;
            entriesCount = MD5EntriesME2Size;
            break;
        case 3:
            entries = entriesME3;
            entriesCount = MD5EntriesME3Size;
            break;
    }

    MemoryStream stream;

    QList<QString> files;
    for (int p = 0; p < entriesCount; p++)
    {
        bool found = false;
        for (int s = 0; s < files.count(); s++)
        {
            if (entries[p].path == files[s])
            {
                found = true;
                break;
            }
        }
        if (!found)
            files.append(entries[p].path);
     }

    stream.WriteInt32(files.count());
    for (int p = 0; p < files.count(); p++)
    {
        stream.WriteStringASCIINull(files[p]);
    }

    stream.WriteInt32(entriesCount);
    for (int p = 0; p < entriesCount; p++)
    {
        int index = -1;
        for (int s = 0; s < files.count(); s++)
        {
            if (entries[p].path == files[s])
            {
                index = s;
                break;
            }
        }
        stream.WriteInt32(index);
        stream.WriteInt32(entries[p].size);
        stream.WriteFromBuffer(const_cast<quint8 *>(entries[p].md5), 16);
    }
    {
        FileStream fs = FileStream(QString("MD5EntriesME") + QString::number(gameId) + ".bin",
                                   FileMode::Create, FileAccess::WriteOnly);
        fs.WriteUInt32(md5Tag);
        ByteBuffer tmp = stream.ToArray();
        fs.WriteInt32(tmp.size());
        quint8 *compressed = nullptr;
        uint compressedSize = 0;
        LzmaCompress(tmp.ptr(), tmp.size(), &compressed, &compressedSize, 9);
        fs.WriteFromBuffer(compressed, compressedSize);
        delete[] compressed;
    }
    stream.Close();
    files.clear();

    if (gameId == 1)
    {
        // Polish version DB
        entries = entriesME1PL;
        entriesCount = MD5EntriesME1PLSize;
        MemoryStream streamPL;
        for (int p = 0; p < entriesCount; p++)
        {
            bool found = false;
            for (int s = 0; s < files.count(); s++)
            {
                if (entries[p].path == files[s])
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                files.append(entries[p].path);
        }

        streamPL.WriteInt32(files.count());
        for (int p = 0; p < files.count(); p++)
        {
            streamPL.WriteStringASCIINull(files[p]);
        }

        streamPL.WriteInt32(entriesCount);
        for (int p = 0; p < entriesCount; p++)
        {
            int index = -1;
            for (int s = 0; s < files.count(); s++)
            {
                if (entries[p].path == files[s])
                {
                    index = s;
                    break;
                }
            }
            streamPL.WriteInt32(index);
            streamPL.WriteInt32(entries[p].size);
            streamPL.WriteFromBuffer(const_cast<quint8 *>(entries[p].md5), 16);
        }
        {
            FileStream fs = FileStream(QString("MD5EntriesME1PL.bin"),
                                       FileMode::Create, FileAccess::WriteOnly);
            fs.WriteUInt32(md5Tag);
            ByteBuffer tmp = streamPL.ToArray();
            fs.WriteInt32(tmp.size());
            quint8 *compressed = nullptr;
            uint compressedSize = 0;
            LzmaCompress(tmp.ptr(), tmp.size(), &compressed, &compressedSize, 9);
            fs.WriteFromBuffer(compressed, compressedSize);
            delete[] compressed;
        }
    }
}

int main(int argc, char *argv[])
{
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    QCoreApplication application(argc, argv);

    for (int p = 1; p <= 3; p++)
    {
        generateBinFile(p);
    }
}
