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

#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <Helpers/Crc32.h>
#include <Wrappers.h>
#include <GameData/Package.h>
#include <GameData/GameData.h>
#include <Texture/Texture.h>
#include <Program/ConfigIni.h>
#include <Types/MemTypes.h>

Texture::Texture(Package &package, int exportId, const ByteBuffer &data, bool fixDim)
{
    dataExportId = exportId;
    properties = new Properties(package, data);
    if (data.size() == properties->propertyEndOffset)
        return;

    textureData = new MemoryStream(data, properties->propertyEndOffset, data.size() - properties->propertyEndOffset);
    if (GameData::gameType != MeType::ME3_TYPE)
    {
        textureData->Skip(12); // 12 zeros
        textureData->SkipInt32(); // position in the package
    }

    int numMipMaps = textureData->ReadInt32();
    for (int l = 0; l < numMipMaps; l++)
    {
        TextureMipMap mipmap{};
        mipmap.storageType = (StorageTypes)textureData->ReadInt32();
        mipmap.uncompressedSize = textureData->ReadInt32();
        mipmap.compressedSize = textureData->ReadInt32();
        mipmap.dataOffset = textureData->ReadUInt32();
        if (mipmap.storageType == StorageTypes::pccUnc)
        {
            mipmap.internalOffset = textureData->Position();
            textureData->Skip(mipmap.uncompressedSize);
        }
        if (mipmap.storageType == StorageTypes::pccLZO ||
            mipmap.storageType == StorageTypes::pccZlib)
        {
            mipmap.internalOffset = textureData->Position();
            textureData->Skip(mipmap.compressedSize);
        }

        mipmap.width = textureData->ReadInt32();
        mipmap.height = textureData->ReadInt32();

        if (fixDim)
        {
            if (mipmap.width == 4)
            {
                for (int i = 0; i < mipMapsList.count(); i++)
                {
                    if (mipMapsList[i].width == mipmap.width)
                    {
                        mipmap.width = mipMapsList.last().width / 2;
                        break;
                    }
                }
            }
            if (mipmap.height == 4)
            {
                for (int i = 0; i < mipMapsList.count(); i++)
                {
                    if (mipMapsList[i].height == mipmap.height)
                    {
                        mipmap.height = mipMapsList.last().height / 2;
                        break;
                    }
                }
            }
            if (mipmap.width == 0)
                mipmap.width = 1;
            if (mipmap.height == 0)
                mipmap.height = 1;
        }

        mipMapsList.push_back(mipmap);
    }

    restOfData = ByteBuffer(textureData->Length() - textureData->Position());
    textureData->ReadToBuffer(restOfData.ptr(), textureData->Length() - textureData->Position());

    packagePath = package.packagePath;
    packageName = BaseNameWithoutExt(packagePath).toLower();
    if (GameData::gameType == MeType::ME1_TYPE)
    {
        QString baseName = package.resolvePackagePath(package.exportsTable[exportId].getLinkId()).split('.')[0].toLower();
        bool found = false;
        for (int i = 0; i < mipMapsList.count(); i++)
        {
            if (mipMapsList[i].storageType == StorageTypes::extLZO ||
                mipMapsList[i].storageType == StorageTypes::extZlib ||
                mipMapsList[i].storageType == StorageTypes::extUnc)
            {
                basePackageName = baseName;
                if (basePackageName.length() == 0)
                    CRASH();
                slave = true;
                found = true;
                break;
            }
        }
        if (!found)
        {
            if (baseName.length() != 0 && !properties->exists("NeverStream"))
            {
                if (std::binary_search(g_GameData->packageME1UpperNames.begin(),
                                       g_GameData->packageME1UpperNames.end(),
                                       baseName, comparePath))
                {
                    basePackageName = baseName;
                    weakSlave = true;
                }
            }
        }
    }
}

Texture::~Texture()
{
    delete textureData;
    restOfData.Free();
    delete properties;
    for (int i = 0; i < mipMapsList.count(); i++)
    {
        if (mipMapsList[i].freeNewData)
            mipMapsList[i].newData.Free();
    }
}

void Texture::replaceMipMaps(const QList<TextureMipMap> &newMipMaps)
{
    for (int l = 0; l < mipMapsList.count(); l++)
    {
        if (mipMapsList[l].freeNewData)
            mipMapsList[l].newData.Free();
    }
    mipMapsList.clear();
    mipMapsList = newMipMaps;

    delete textureData;
    textureData = new MemoryStream();
    if (GameData::gameType != MeType::ME3_TYPE)
    {
        textureData->WriteZeros(16);
    }
    textureData->WriteInt32(newMipMaps.count());
    for (int l = 0; l < newMipMaps.count(); l++)
    {
        TextureMipMap mipmap = mipMapsList[l];
        textureData->WriteUInt32(mipmap.storageType);
        textureData->WriteInt32(mipmap.uncompressedSize);
        textureData->WriteInt32(mipmap.compressedSize);
        textureData->WriteUInt32(mipmap.dataOffset);

        if (mipmap.storageType == StorageTypes::pccUnc ||
            mipmap.storageType == StorageTypes::pccLZO ||
            mipmap.storageType == StorageTypes::pccZlib)
        {
            mipmap.internalOffset = textureData->Position();
            textureData->WriteFromBuffer(mipmap.newData);
        }
        mipMapsList[l] = mipmap;
    }
}

uint Texture::getCrcData(ByteBuffer data)
{
    if (data.ptr() == nullptr)
        return 0;
    if (properties->getProperty("Format").getValueName() == "PF_NormalMap_HQ") // only ME1 and ME2
        return ~crc32_16bytes_prefetch(data.ptr(), data.size() / 2);
    return ~crc32_16bytes_prefetch(data.ptr(), data.size());
}

uint Texture::getCrcMipmap(TextureMipMap &mipmap)
{
    ByteBuffer data = getMipMapData(mipmap);
    if (data.ptr() == nullptr)
        return 0;
    uint crc = getCrcData(data);
    data.Free();
    return crc;
}

uint Texture::getCrcTopMipmap()
{
    ByteBuffer data = getTopImageData();
    if (data.ptr() == nullptr)
        return 0;
    uint crc = getCrcData(data);
    data.Free();
    return crc;
}

const Texture::TextureMipMap& Texture::getTopMipmap()
{
    for (int l = 0; l < mipMapsList.count(); l++)
    {
        if (mipMapsList[l].storageType != StorageTypes::empty)
            return mipMapsList[l];
    }
    CRASH();
}

const Texture::TextureMipMap& Texture::getBottomMipmap()
{
    return mipMapsList.last();
}

bool Texture::existMipmap(int width, int height)
{
    for (int l = 0; l < mipMapsList.count(); l++)
    {
        if (mipMapsList[l].width == width && mipMapsList[l].height == height)
            return true;
    }
    return false;
}

const Texture::TextureMipMap& Texture::getMipMapByIndex(int index)
{
    if (mipMapsList.count() == 0 || index < 0 || index >= mipMapsList.count())
        CRASH();

    return mipMapsList[index];
}

void Texture::removeEmptyMips()
{
    for (int l = 0; l < mipMapsList.count(); l++)
    {
        if (mipMapsList[l].storageType == StorageTypes::empty)
        {
            mipMapsList.removeAt(l--);
        }
    }
}

void Texture::removeTopMip()
{
    if (mipMapsList.count() > 1)
    {
        mipMapsList.removeFirst();
    }
}

const Texture::TextureMipMap& Texture::getMipmap(int width, int height)
{
    for (int l = 0; l < mipMapsList.count(); l++)
    {
        if (mipMapsList[l].width == width && mipMapsList[l].height == height)
            return mipMapsList[l];
    }
    CRASH();
}

bool Texture::hasImageData()
{
    return mipMapsList.count() != 0;
}

const ByteBuffer Texture::getTopImageData()
{
    if (mipMapsList.count() == 0)
        return ByteBuffer();

    TextureMipMap m = getTopMipmap();
    return getMipMapData(m);
}

const ByteBuffer Texture::getMipMapDataByIndex(int index)
{
    if (mipMapsList.count() == 0 || index < 0 || index >= mipMapsList.count())
        return ByteBuffer();

    return getMipMapData(mipMapsList[index]);
}

const ByteBuffer Texture::getMipMapData(TextureMipMap &mipmap)
{
    ByteBuffer mipMapData;

    switch (mipmap.storageType)
    {
    case StorageTypes::pccUnc:
        {
            textureData->JumpTo(mipmap.internalOffset);
            mipMapData = textureData->ReadToBuffer(mipmap.uncompressedSize);
            break;
        }
    case StorageTypes::pccLZO:
    case StorageTypes::pccZlib:
        {
            textureData->JumpTo(mipmap.internalOffset);
            mipMapData = Package::decompressData(dynamic_cast<Stream &>(*textureData), mipmap.storageType, mipmap.uncompressedSize, mipmap.compressedSize);
            if (mipMapData.ptr() == nullptr)
            {
                PERROR(QString("\nPackage: ") + packagePath +
                    "\nStorageType: " + QString::number(mipmap.storageType) +
                    "\nExport Id: " + QString::number(dataExportId + 1) +
                    "\nInternal offset: " + QString::number(mipmap.internalOffset) + "\n");
            }
            break;
        }
    case StorageTypes::extUnc:
    case StorageTypes::extLZO:
    case StorageTypes::extZlib:
        {
            QString filename;
            if (GameData::gameType == MeType::ME1_TYPE)
            {
                auto found = g_GameData->mapME1PackageUpperNames.find(basePackageName);
                if (found.key().length() == 0)
                {
                    PERROR((QString("File not found in game: ") + basePackageName + ".*" + "\n").toStdString().c_str());
                    return ByteBuffer();
                }
                filename = g_GameData->GamePath() + g_GameData->packageFiles[found.value()];
            }
            else
            {
                QString archive = properties->getProperty("TextureFileCacheName").getValueName();
                filename = g_GameData->MainData() + "/" + archive + ".tfc";
                if (packagePath.contains("/DLC", Qt::CaseInsensitive))
                {
                    QString DLCArchiveFile = g_GameData->GamePath() + DirName(packagePath) + "/" + archive + ".tfc";
                    if (QFile(DLCArchiveFile).exists())
                        filename = DLCArchiveFile;
                    else if (!QFile(filename).exists())
                    {
                        QStringList files = g_GameData->tfcFiles.filter(QRegExp("*" + archive + ".tfc",
                                                                                Qt::CaseInsensitive, QRegExp::Wildcard));
                        if (files.count() == 1)
                            filename = g_GameData->GamePath() + files.first();
                        else if (files.count() == 0)
                        {
                            if (g_ipc)
                            {
                                ConsoleWrite("[IPC]ERROR_REFERENCED_TFC_NOT_FOUND " + archive + ".tfc");
                                ConsoleSync();
                            }
                            else
                            {
                                PERROR(QString("TFC file not found: ") + archive + ".tfc" + "\n");
                            }
                            return ByteBuffer();
                        }
                        else
                        {
                            QString list;
                            foreach(QString file, files)
                                list += file + "\n";
                            PERROR((QString("More instances of TFC file: ") + archive + ".tfc\n" +
                                       list).toStdString().c_str());
                            return ByteBuffer();
                        }
                    }
                }
            }

            if (!QFile(filename).exists())
            {
                if (g_ipc)
                {
                    ConsoleWrite("[IPC]ERROR_REFERENCED_TFC_NOT_FOUND " + g_GameData->RelativeGameData(filename));
                    ConsoleSync();
                }
                else
                {
                    PERROR(QString("File no found: " + filename + "\n"));
                }
                PERROR(QString("\nPackage: ") + packagePath +
                       "\nStorageType: " + QString::number(mipmap.storageType) +
                       "\nExport Id: " + QString::number(dataExportId + 1) +
                       "\nExternal file offset: " + QString::number(mipmap.dataOffset) + "\n");
                return ByteBuffer();
            }
            auto fs = FileStream(filename, FileMode::Open, FileAccess::ReadOnly);
            fs.JumpTo(mipmap.dataOffset);
            if (mipmap.storageType == StorageTypes::extLZO || mipmap.storageType == StorageTypes::extZlib)
            {
                mipMapData = Package::decompressData(dynamic_cast<Stream &>(fs), mipmap.storageType, mipmap.uncompressedSize, mipmap.compressedSize);
                if (mipMapData.ptr() == nullptr)
                {
                    PERROR(QString("\nFile: ") + filename +
                        "\nPackage: " + packagePath +
                        "\nStorageType: " + QString::number(mipmap.storageType) +
                        "\nExport Id: " + QString::number(dataExportId + 1) +
                        "\nExternal file offset: " + QString::number(mipmap.dataOffset) + "\n");
                    return ByteBuffer();
                }
            }
            else
            {
                mipMapData = fs.ReadToBuffer(mipmap.uncompressedSize);
            }
            break;
        }
    case StorageTypes::empty:
        CRASH();
    }

    return mipMapData;
}

const ByteBuffer Texture::toArray(uint pccTextureDataOffset, bool updateOffset)
{
    MemoryStream newData;
    if (GameData::gameType != MeType::ME3_TYPE)
    {
        newData.WriteZeros(16);
    }
    newData.WriteInt32(mipMapsList.count());
    for (int l = 0; l < mipMapsList.count(); l++)
    {
        TextureMipMap mipmap = mipMapsList[l];
        newData.WriteInt32(mipmap.storageType);
        newData.WriteInt32(mipmap.uncompressedSize);
        newData.WriteInt32(mipmap.compressedSize);
        if (mipmap.storageType == StorageTypes::pccUnc)
        {
            mipmap.dataOffset = newData.Position() + pccTextureDataOffset + 4;
            newData.WriteUInt32(mipmap.dataOffset);
            textureData->JumpTo(mipmap.internalOffset);
            if (updateOffset)
                mipmap.internalOffset = newData.Position();
            newData.CopyFrom(*textureData, mipmap.uncompressedSize);
        }
        else if (mipmap.storageType == StorageTypes::pccLZO ||
                 mipmap.storageType == StorageTypes::pccZlib)
        {
            mipmap.dataOffset = newData.Position() + pccTextureDataOffset + 4;
            newData.WriteUInt32(mipmap.dataOffset);
            textureData->JumpTo(mipmap.internalOffset);
            if (updateOffset)
                mipmap.internalOffset = newData.Position();
            newData.CopyFrom(*textureData, mipmap.compressedSize);
        }
        else
        {
            newData.WriteUInt32(mipmap.dataOffset);
        }
        newData.WriteInt32(mipmap.width);
        newData.WriteInt32(mipmap.height);
        mipMapsList[l] = mipmap;
    }

    newData.WriteFromBuffer(restOfData);

    return newData.ToArray();
}

bool Texture::hasEmptyMips()
{
    for (int l = 0; l < mipMapsList.count(); l++)
    {
        if (mipMapsList[l].storageType == StorageTypes::empty)
            return true;
    }
    return false;
}

int Texture::numNotEmptyMips()
{
    int num = 0;
    for (int l = 0; l < mipMapsList.count(); l++)
    {
        if (mipMapsList[l].storageType != StorageTypes::empty)
            num++;
    }
    return num;
}

bool Texture::HasExternalMips()
{
    for (int l = 0; l < mipMapsList.count(); l++)
    {
        if (mipMapsList[l].storageType == StorageTypes::extUnc ||
            mipMapsList[l].storageType == StorageTypes::extLZO ||
            mipMapsList[l].storageType == StorageTypes::extZlib)
        {
            return true;
        }
    }
    return false;
}
