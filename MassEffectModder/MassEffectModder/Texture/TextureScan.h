/*
 * MassEffectModder
 *
 * Copyright (C) 2018-2021 Pawel Kolodziejski
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
#include <Texture/Texture.h>
#include <GameData/Properties.h>
#include <Resources/Resources.h>

struct Texture4kNormEntry
{
    QString path;
    QString textureName;
    int exportId;
};

struct TextureMapPackageEntry
{
    int exportID;
    QString path;
    QList<uint> crcs;
    int numMips;
    bool movieTexture;
};

struct TextureMapEntry
{
    QString name;
    uint crc;
    QList<TextureMapPackageEntry> list;
    PixelFormat pixfmt;
    TextureType type;
    int width, height;
};

class TreeScan
{
private:

    static void FindTextures(QList<TextureMapEntry> &textures,
                             const QString &packagePath, bool modified);

public:

    typedef void (*ProgressCallback)(void *handle, int progress, const QString &stage);

    TreeScan() = default;
    static void loadTexturesMap(MeType gameId, Resources &resources, QList<TextureMapEntry> &textures);
    static bool loadTexturesMapFile(QString &path, QList<TextureMapEntry> &textures, bool ignoreCheck = false);
    static bool PrepareListOfTextures(MeType gameId, Resources &resources,
                                     QList<TextureMapEntry> &textures, bool saveMapFile,
                                     ProgressCallback callback, void *callbackHandle);
    static bool IsBlankTexture(uint crc);
};


#endif
