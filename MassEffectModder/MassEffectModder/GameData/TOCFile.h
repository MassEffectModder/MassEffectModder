/*
 * MassEffectModder
 *
 * Copyright (C) 2018-2022 Pawel Kolodziejski
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

#ifndef TOC_FILE_H
#define TOC_FILE_H

#include <Types/MemTypes.h>

#define TOCTag 0x3AB70C13 // TOC tag

class TOCBinFile
{
    struct FileEntry
    {
        quint64 size;
        QString path;
        quint32 hashFilename;
    };

private:

    static void GenerateMainTocBinFile(MeType gameType);
    static void GenerateDLCsTocBinFiles();
    static void CreateTocBinFile(QString &path, const QVector<FileEntry>& filesList);

public:

    static void UpdateAllTOCBinFiles(MeType gameType);
};

#endif
