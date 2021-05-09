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

#ifndef TEXTURE_H
#define TEXTURE_H

#include <Helpers/FileStream.h>
#include <Helpers/MemoryStream.h>
#include <GameData/Package.h>
#include <GameData/Properties.h>

class Texture
{
private:

    MemoryStream *textureData = nullptr;
    ByteBuffer restOfData;
    QString packagePath;
    Properties *properties;

public:

    struct TextureMipMap
    {
        StorageTypes storageType = StorageTypes::empty;
        int uncompressedSize{};
        int compressedSize{};
        uint dataOffset{};
        uint internalOffset{};
        int width{};
        int height{};
        ByteBuffer newData;
        bool freeNewData{};
    };

    QList<TextureMipMap> mipMapsList;
    QString packageName;
    bool slave{};
    bool weakSlave{};
    int dataExportId;

    Texture(Package &package, int exportId, const ByteBuffer &data, bool fixDim = true);
    ~Texture();
    void replaceMipMaps(const QList<TextureMipMap> &newMipMaps);
    Properties& getProperties() { return *properties; }
    uint getCrcData(ByteBuffer data);
    uint getCrcMipmap(TextureMipMap &mipmap);
    uint getCrcTopMipmap();
    const TextureMipMap& getMipMapByIndex(int index);
    const TextureMipMap& getTopMipmap();
    const TextureMipMap& getBottomMipmap();
    bool existMipmap(int width, int height);
    const TextureMipMap& getMipmap(int width, int height);
    bool hasImageData();
    const ByteBuffer getTopImageData();
    const ByteBuffer getMipMapDataByIndex(int index);
    const ByteBuffer getMipMapData(TextureMipMap &mipmap);
    void removeEmptyMips();
    void removeTopMip();
    bool hasEmptyMips();
    int numNotEmptyMips();
    int getNumMipmaps() { return mipMapsList.count(); }
    bool HasExternalMips();
    const ByteBuffer toArray(uint pccTextureDataOffset, bool updateOffset = true);
    void Dispose();
};

#endif
