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

#ifndef TREESCAN_H
#define TREESCAN_H

#include "MemTypes.h"
#include "TextureProps.h"
#include "Resources.h"

class MipMaps;

struct MatchedTexture
{
    int exportID;
    QString packageName; // only used while texture scan for ME1
    QString basePackageName; // only used while texture scan for ME1
    bool weakSlave;
    bool slave;
    QString path;
    int linkToMaster;
    uint mipmapOffset;
    QList<uint> crcs;
    QList<uint> masterDataOffset;
    bool removeEmptyMips;
    int numMips;
};

struct FoundTexture
{
    QString name;
    uint crc;
    QList<MatchedTexture> list;
    PixelFormat pixfmt;
    TexProperty::TextureTypes flags;
    int width, height;
};

class TreeScan
{
private:

    static void FindTextures(MeType gameId, QList<FoundTexture> &textures, const QString &packagePath,
                             bool modified, bool ipc);

public:

    TreeScan() = default;
    static void loadTexturesMap(MeType gameId, Resources &resources, QList<FoundTexture> &textures);
    static bool loadTexturesMapFile(QString &path, QList<FoundTexture> &textures, bool ipc);
    static int PrepareListOfTextures(MeType gameId, Resources &resources, QList<FoundTexture> &textures,
                                        bool ipc);
};


#endif
