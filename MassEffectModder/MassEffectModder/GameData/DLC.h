/*
 * MassEffectModder
 *
 * Copyright (C) 2018-2019 Pawel Kolodziejski
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

#ifndef DLC_H
#define DLC_H

#include <Helpers/Stream.h>

const quint8 FileListHash[] = { 0xb5, 0x50, 0x19, 0xcb, 0xf9, 0xd3, 0xda, 0x65, 0xd5, 0x5b, 0x32, 0x1c, 0x00, 0x19, 0x69, 0x7c };

class ME3DLC
{

    enum DlcEnums {
        SfarTag          = 0x53464152, // 'SFAR'
        SfarVersion      = 0x00010000,
        LZMATag          = 0x6c7a6d61, // 'lzma'
        HeaderSize       = 0x20,
        EntryHeaderSize  = 0x1e,
        MaxBlockSize     = 0x00010000,
    };

    struct FileEntry
    {
        quint8 filenameHash[16];
        QString filenamePath;
        long uncomprSize;
        int compressedBlockSizesIndex;
        uint numBlocks;
        long dataOffset;
    };

    int filenamesIndex;
    uint filesCount;
    QList<FileEntry> filesList;
    uint maxBlockSize;
    QList<quint16> blockSizes;

    static int HashCompare(FileEntry x, FileEntry y)
    {
        return memcmp(x.filenameHash, y.filenameHash, 16);
    };

    static int getNumberOfFiles(QString &path);
    bool loadHeader(Stream *stream);

public:
    typedef void (*ProgressCallback)(void *handle, int progress, const QString &stage);

    bool extract(QString &SFARfilename, int &currentProgress,
                 int totalNumber, ProgressCallback callback, void *callbackHandle);
    static void unpackAllDLC(ProgressCallback callback, void *callbackHandle);
};

#endif
