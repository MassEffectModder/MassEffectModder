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

#ifndef MIPMAPS_H
#define MIPMAPS_H

#include "MemTypes.h"
#include "TreeScan.h"
#include "Helpers/ByteBuffer.h"
#include "Helpers/Stream.h"

struct FileMod
{
    uint tag;
    QString name;
    long offset;
    long size;
};

struct RemoveMipsEntry
{
    QString pkgPath;
    QList<int> exportIDs;
};


class MipMaps
{
private:

    QList<QString> pkgsToRepack;
    QList<QString> pkgsToMarker;

public:

    static Stream *compressData(ByteBuffer inputData);
    static ByteBuffer decompressData(Stream *stream, long compressedSize);
    void extractTextureToPng(QString &outputFile, QString &packagePath, int exportID);

    QList<RemoveMipsEntry> *prepareListToRemove(QList<FoundTexture> *textures);
    void removeMipMapsME1(int phase, QList<FoundTexture> *textures, bool ipc);
    void removeMipMapsME1(int phase, QList<FoundTexture> *textures, Package &package,
                          QList<RemoveMipsEntry> *list, int removeEntry, bool ipc);
    void removeMipMapsME2ME3(QList<FoundTexture> *textures, bool ipc, bool repack);
    void removeMipMapsME2ME3(Package &package,
                             QList<RemoveMipsEntry> *list, int removeEntry, bool repack);
};


#endif
