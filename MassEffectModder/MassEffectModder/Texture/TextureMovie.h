/*
 * MassEffectModder
 *
 * Copyright (C) 2019-2022 Pawel Kolodziejski
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

#ifndef TEXTURE_MOVIE_H
#define TEXTURE_MOVIE_H

#include <Helpers/FileStream.h>
#include <Helpers/MemoryStream.h>
#include <GameData/Package.h>
#include <GameData/Properties.h>

class TextureMovie
{
private:

    MemoryStream *textureData = nullptr;
    QString packagePath;
    Properties *properties;
    int dataExportId;
    StorageTypes storageType;
    quint32 uncompressedSize;
    quint32 compressedSize;
    quint32 dataOffset;

public:

    TextureMovie(Package &package, int exportId, const ByteBuffer &data);
    ~TextureMovie();
    Properties& getProperties() { return *properties; }
    StorageTypes getStorageType() { return storageType; }
    quint32 getUncompressedSize() { return uncompressedSize; }
    quint32 getDataOffset() { return dataOffset; }
    void replaceMovieData(ByteBuffer data, uint offset);
    bool hasTextureData() { return textureData != nullptr; }
    const ByteBuffer getData();
    uint getCrcData();
    const ByteBuffer toArray();
};

#endif
