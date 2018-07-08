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

#include <QFileInfo>

#include "Exceptions/SignalHandler.h"
#include "Helpers/Misc.h"
#include "Helpers/Logs.h"
#include "Helpers/ParallelCRC.h"
#include "Wrappers.h"

#include "Package.h"
#include "Texture.h"
#include "ConfigIni.h"
#include "GameData.h"
#include "MemTypes.h"

Texture::Texture(Package &package, int exportId, quint8 *data, int length, bool fixDim)
{
    properties = new TexProperty(package, data);
    if (length == properties->propertyEndOffset)
        return;

    textureData = new MemoryStream(data, properties->propertyEndOffset, length - properties->propertyEndOffset);
    if (GameData::gameType != MeType::ME3_TYPE)
    {
        textureData->Skip(12); // 12 zeros
        textureData->SkipInt32(); // position in the package
    }

    int numMipMaps = textureData->ReadInt32();
    for (int l = 0; l < numMipMaps; l++)
    {
        MipMap mipmap{};
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

    restOfData = new quint8[textureData->Length() - textureData->Position()];
    textureData->ReadToBuffer(restOfData, textureData->Length() - textureData->Position());

    packagePath = package.packagePath;
    packageName = BaseNameWithoutExt(packagePath).toUpper();
    if (GameData::gameType == MeType::ME1_TYPE)
    {
        QString baseName = package.resolvePackagePath(package.exportsTable[exportId].getLinkId()).split('.')[0].toUpper();
        bool found = false;
        for (int i = 0; i < mipMapsList.count(); i++)
        {
            if (mipMapsList[i].storageType == StorageTypes::extLZO ||
                mipMapsList[i].storageType == StorageTypes::extZlib ||
                mipMapsList[i].storageType == StorageTypes::extUnc)
            {
                found = true;
            }
        }
        if (found)
        {
            basePackageName = baseName;
            if (basePackageName == "")
                CRASH();
            slave = true;
        }
        else
        {
            if (baseName != "" && !properties->exists("NeverStream"))
            {
                for (int i = 0; i < GameData::packageFiles.count(); i++)
                {
                    if (baseName.compare(BaseNameWithoutExt(GameData::packageFiles[i]), Qt::CaseInsensitive))
                    {
                        basePackageName = baseName;
                        weakSlave = true;
                        break;
                    }
                }
            }
        }
    }
}

Texture::~Texture()
{
    delete textureData;
    delete[] mipMapData;
    delete[] restOfData;
    delete properties;
    for (int i = 0; i < mipMapsList.count(); i++)
    {
        delete[] mipMapsList[i].newData;
    }
}

void Texture::replaceMipMaps(QList<MipMap> &newMipMaps)
{
    for (int l = 0; l < mipMapsList.count(); l++)
    {
        delete[] mipMapsList[l].newData;
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
        MipMap mipmap = mipMapsList[l];
        textureData->WriteUInt32(mipmap.storageType);
        textureData->WriteInt32(mipmap.uncompressedSize);
        textureData->WriteInt32(mipmap.compressedSize);
        textureData->WriteUInt32(mipmap.dataOffset);

        if (mipmap.storageType == StorageTypes::pccUnc ||
            mipmap.storageType == StorageTypes::pccLZO ||
            mipmap.storageType == StorageTypes::pccZlib)
        {
            mipmap.internalOffset = textureData->Position();
            textureData->WriteFromBuffer(mipmap.newData, mipmap.newDataLength);
        }
        mipMapsList[l] = mipmap;
    }
}

quint8 *Texture::compressTexture(quint8 *inputData, uint length, StorageTypes type, qint64 &compressedSize)
{
    MemoryStream ouputStream;
    compressedSize = 0;
    uint dataBlockLeft = length;
    uint newNumBlocks = (length + maxBlockSize - 1) / maxBlockSize;
    QList<Package::ChunkBlock> blocks{};
    auto inputStream = new MemoryStream(inputData, length);
    // skip blocks header and table - filled later
    ouputStream.Seek(SizeOfChunk + SizeOfChunkBlock * newNumBlocks, SeekOrigin::Begin);

    for (uint b = 0; b < newNumBlocks; b++)
    {
        Package::ChunkBlock block{};
        block.uncomprSize = qMin((uint)maxBlockSize, dataBlockLeft);
        dataBlockLeft -= block.uncomprSize;
        inputStream->ReadToBuffer(block.uncompressedBuffer, block.uncomprSize);
        blocks.push_back(block);
    }
    delete inputStream;

    for (int b = 0; b < blocks.count(); b++)
    {
        Package::ChunkBlock block = blocks[b];
        if (type == StorageTypes::extLZO || type == StorageTypes::pccLZO)
            LzoCompress(block.uncompressedBuffer, block.uncomprSize, &block.compressedBuffer, &block.comprSize);
        else if (type == StorageTypes::extZlib || type == StorageTypes::pccZlib)
            ZlibCompress(block.uncompressedBuffer, block.uncomprSize, &block.compressedBuffer, &block.comprSize);
        else
            CRASH_MSG("Compression type not expected!");
        if (block.comprSize == 0)
            CRASH_MSG("Compression failed!");
        blocks[b] = block;
    };

    for (int b = 0; b < blocks.count(); b++)
    {
        const Package::ChunkBlock& block = blocks[b];
        ouputStream.WriteFromBuffer(block.compressedBuffer, block.comprSize);
        compressedSize += block.comprSize;
    }

    ouputStream.SeekBegin();
    ouputStream.WriteUInt32(textureTag);
    ouputStream.WriteUInt32(maxBlockSize);
    ouputStream.WriteUInt32(compressedSize);
    ouputStream.WriteInt32(length);
    for (int b = 0; b < blocks.count(); b++)
    {
        const Package::ChunkBlock& block = blocks[b];
        ouputStream.WriteUInt32(block.comprSize);
        ouputStream.WriteUInt32(block.uncomprSize);
    }

    return ouputStream.ToArray(compressedSize);
}

quint8 *Texture::decompressTexture(MemoryStream &stream, StorageTypes type, int uncompressedSize, int compressedSize)
{
    auto data = new quint8[uncompressedSize];
    uint blockTag = stream.ReadUInt32();
    if (blockTag != textureTag)
        CRASH();
    uint blockSize = stream.ReadUInt32();
    if (blockSize != maxBlockSize)
        CRASH();
    uint compressedChunkSize = stream.ReadUInt32();
    uint uncompressedChunkSize = stream.ReadUInt32();
    if (uncompressedChunkSize != (uint)uncompressedSize)
        CRASH();

    uint blocksCount = (uncompressedChunkSize + maxBlockSize - 1) / maxBlockSize;
    if ((compressedChunkSize + SizeOfChunk + SizeOfChunkBlock * blocksCount) != (uint)compressedSize)
        CRASH();

    QList<Package::ChunkBlock> blocks{};
    for (uint b = 0; b < blocksCount; b++)
    {
        Package::ChunkBlock block{};
        block.comprSize = stream.ReadUInt32();
        block.uncomprSize = stream.ReadUInt32();
        blocks.push_back(block);
    }

    for (int b = 0; b < blocks.count(); b++)
    {
        Package::ChunkBlock block = blocks[b];
        block.compressedBuffer = new quint8[blocks[b].comprSize];
        stream.ReadToBuffer(block.compressedBuffer, blocks[b].comprSize);
        block.uncompressedBuffer = new quint8[maxBlockSize * 2];
        blocks[b] = block;
    }

    for (int b = 0; b < blocks.count(); b++)
    {
        uint dstLen = maxBlockSize * 2;
        Package::ChunkBlock block = blocks[b];
        if (type == StorageTypes::extLZO || type == StorageTypes::pccLZO)
            LzoDecompress(block.compressedBuffer, block.comprSize, block.uncompressedBuffer, &dstLen);
        else if (type == StorageTypes::extZlib || type == StorageTypes::pccZlib)
            ZlibDecompress(block.compressedBuffer, block.comprSize, block.uncompressedBuffer, &dstLen);
        else
            CRASH_MSG("Compression type not expected!");
        if (dstLen != block.uncomprSize)
            CRASH_MSG("Decompressed data size not expected!");
    };

    int dstPos = 0;
    for (int b = 0; b < blocks.count(); b++)
    {
        memcpy(data + dstPos, blocks[b].uncompressedBuffer, blocks[b].uncomprSize);
        dstPos += blocks[b].uncomprSize;
        delete blocks[b].compressedBuffer;
        delete blocks[b].uncompressedBuffer;
    }

    return data;
}

uint Texture::getCrcData(quint8 *data, int length)
{
    if (data == nullptr)
        return 0;
    if (properties->getProperty("Format").valueName == "PF_NormalMap_HQ") // only ME1 and ME2
        return (uint)~ParallelCRC::Compute(data, 0, length / 2);
    return (uint)~ParallelCRC::Compute(data, length);
}

uint Texture::getCrcMipmap(MipMap &mipmap)
{
    int length = 0;
    quint8 *data = getMipMapData(mipmap, length);
    if (data == nullptr)
        return 0;
    return getCrcData(data, length);
}

uint Texture::getCrcTopMipmap()
{
    int length = 0;
    quint8 *data = getTopImageData(length);
    if (data == nullptr)
        return 0;
    return getCrcData(data, length);
}

const Texture::MipMap& Texture::getTopMipmap()
{
    for (int l = 0; l < mipMapsList.count(); l++)
    {
        if (mipMapsList[l].storageType != StorageTypes::empty)
            return mipMapsList[l];
    }
    CRASH();
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

const Texture::MipMap& Texture::getMipmap(int width, int height)
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
    return mipMapsList.count() == 0;
}

quint8 *Texture::getTopImageData(int &length)
{
    if (mipMapsList.count() == 0)
        return nullptr;

    if (mipMapData != nullptr)
        return mipMapData;

    MipMap m = getTopMipmap();
    return getMipMapData(m, length);
}

quint8 *Texture::getMipMapDataByIndex(int index, int &length)
{
    if (mipMapsList.count() == 0 || index < 0 || index > mipMapsList.count())
        return nullptr;

    return getMipMapData(mipMapsList[index], length);
}

quint8 *Texture::getMipMapData(MipMap &mipmap, int &length)
{
    switch (mipmap.storageType)
    {
    case StorageTypes::pccUnc:
        {
            textureData->JumpTo(mipmap.internalOffset);
            delete[] mipMapData;
            mipMapData = new quint8[mipmap.uncompressedSize];
            textureData->ReadToBuffer(mipMapData, mipmap.uncompressedSize);
            break;
        }
    case StorageTypes::pccLZO:
    case StorageTypes::pccZlib:
        {
            textureData->JumpTo(mipmap.internalOffset);
            delete[] mipMapData;
            mipMapData = decompressTexture(*textureData, mipmap.storageType, mipmap.uncompressedSize, mipmap.compressedSize);
            break;
        }
    case StorageTypes::extUnc:
    case StorageTypes::extLZO:
    case StorageTypes::extZlib:
        {
            QString filename;
            if (GameData::gameType == MeType::ME1_TYPE)
            {
                for (int i = 0; i < GameData::packageFiles.count(); i++)
                {
                    if (basePackageName.compare(BaseNameWithoutExt(GameData::packageFiles[i]), Qt::CaseInsensitive))
                    {
                        filename = GameData::packageFiles[i];
                        break;
                    }
                }
                if (filename == "")
                    CRASH_MSG((QString("File not found in game: ") + basePackageName + ".*").toStdString().c_str());
            }
            else
            {
                QString archive = properties->getProperty("TextureFileCacheName").valueName;
                filename = GameData::MainData() + archive + ".tfc";
                if (packagePath.contains("/DLC"), Qt::CaseInsensitive)
                {
                    QString DLCArchiveFile = DirName(packagePath) + archive + ".tfc";
                    if (QFile(DLCArchiveFile).exists())
                        filename = DLCArchiveFile;
                    else if (!QFile(filename).exists())
                    {
                        QStringList files = QDir(GameData::bioGamePath(), archive + ".tfc", QDir::NoSort, QDir::Files | QDir::NoSymLinks).entryList();
                        if (files.count() == 1)
                            filename = files[0];
                        else if (files.count() == 0)
                        {
                            DLCArchiveFile = DirName(DLCArchiveFile) + "Textures_" +
                                    BaseName(DirName(DirName(packagePath)) + ".tfc");
                            if (QFile(DLCArchiveFile).exists())
                                filename = DLCArchiveFile;
                            else
                                filename = GameData::MainData() + "Textures.tfc";
                        }
                        else
                            CRASH();
                    }
                }
            }

            auto fs = new FileStream(filename, FileMode::Open, FileAccess::ReadOnly);
            if (!fs->isOpen())
            {
                CRASH_MSG((QString("Problem with access to file: ") + filename).toStdString().c_str());
            }
            fs->JumpTo(mipmap.dataOffset);
            if (mipmap.storageType == StorageTypes::extLZO || mipmap.storageType == StorageTypes::extZlib)
            {
                auto buffer = new quint8[mipmap.compressedSize];
                fs->ReadToBuffer(buffer, mipmap.compressedSize);
                MemoryStream tmpStream = MemoryStream(buffer, mipmap.compressedSize);
                mipMapData = decompressTexture(tmpStream, mipmap.storageType, mipmap.uncompressedSize, mipmap.compressedSize);
                delete[] buffer;
            }
            else
            {
                delete mipMapData;
                mipMapData = new quint8[mipmap.uncompressedSize];
                fs->ReadToBuffer(mipMapData, mipmap.uncompressedSize);
            }
            delete fs;
            break;
        }
    case StorageTypes::empty:
        CRASH();
    }

    length = mipmap.uncompressedSize;
    return mipMapData;
}

quint8 *Texture::toArray(uint pccTextureDataOffset, qint64 &length, bool updateOffset)
{
    MemoryStream newData;
    if (GameData::gameType != MeType::ME3_TYPE)
    {
        newData.WriteZeros(16);
    }
    newData.WriteInt32(mipMapsList.count());
    for (int l = 0; l < mipMapsList.count(); l++)
    {
        MipMap mipmap = mipMapsList[l];
        newData.WriteInt32(mipmap.storageType);
        newData.WriteInt32(mipmap.uncompressedSize);
        newData.WriteInt32(mipmap.compressedSize);
        if (mipmap.storageType == StorageTypes::pccUnc)
        {
            newData.WriteUInt32(0);
            textureData->JumpTo(mipmap.internalOffset);
            if (updateOffset)
                mipmap.internalOffset = newData.Position();
            newData.CopyFrom(textureData, mipmap.uncompressedSize);
        }
        else if (mipmap.storageType == StorageTypes::pccLZO ||
            mipmap.storageType == StorageTypes::pccZlib)
        {
            mipmap.dataOffset = newData.Position() + pccTextureDataOffset + 4;
            newData.WriteUInt32(mipmap.dataOffset);
            textureData->JumpTo(mipmap.internalOffset);
            if (updateOffset)
                mipmap.internalOffset = newData.Position();
            newData.CopyFrom(textureData, mipmap.compressedSize);
        }
        else
        {
            newData.WriteUInt32(mipmap.dataOffset);
        }
        newData.WriteInt32(mipmap.width);
        newData.WriteInt32(mipmap.height);
        mipMapsList[l] = mipmap;
    }

    newData.WriteFromBuffer(restOfData, restOfDataSize);

    return newData.ToArray(length);
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
