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

#ifndef TREESCAN_H
#define TREESCAN_H

#include <Types/MemTypes.h>
#include <Texture/TextureProperty.h>
#include <Resources/Resources.h>

struct TextureMapPackageEntry
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

struct TextureMapEntry
{
    QString name;
    uint crc;
    QList<TextureMapPackageEntry> list;
    PixelFormat pixfmt;
    TextureProperty::TextureTypes flags;
    int width, height;
};

class TreeScan
{
private:

    static void FindTextures(MeType gameId, QList<TextureMapEntry> &textures,
                             const QString &packagePath, bool modified);

public:

    typedef void (*ProgressCallback)(void *handle, int progress, const QString &stage);

    TreeScan() = default;
    static void loadTexturesMap(MeType gameId, Resources &resources, QList<TextureMapEntry> &textures);
    static bool loadTexturesMapFile(QString &path, QList<TextureMapEntry> &textures, bool ignoreCheck = false);
    static bool PrepareListOfTextures(MeType gameId, Resources &resources,
                                     QList<TextureMapEntry> &textures, bool removeEmptyMips,
                                     bool saveMapFile,
                                     ProgressCallback callback, void *callbackHandle);
};


#endif
