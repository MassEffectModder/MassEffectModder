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

#include "Exceptions/SignalHandler.h"
#include "Helpers/MiscHelpers.h"
#include "Helpers/Logs.h"
#include "Helpers/Crc32.h"
#include "Wrappers.h"

#include "Package.h"
#include "Texture.h"
#include "ConfigIni.h"
#include "GameData.h"
#include "MemTypes.h"

Texture::Texture(Package &package, int exportId, const ByteBuffer &data, bool fixDim)
{
    properties = new TexProperty(package, data);
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
            if (basePackageName.length() == 0)
                CRASH();
            slave = true;
        }
        else
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

const ByteBuffer Texture::compressTexture(const ByteBuffer &inputData, StorageTypes type)
{
    MemoryStream ouputStream;
    qint64 compressedSize = 0;
    uint dataBlockLeft = inputData.size();
    uint newNumBlocks = (inputData.size() + maxBlockSize - 1) / maxBlockSize;
    QList<Package::ChunkBlock> blocks{};
    {
        auto inputStream = MemoryStream(inputData);
        // skip blocks header and table - filled later
        ouputStream.Seek(SizeOfChunk + SizeOfChunkBlock * newNumBlocks, SeekOrigin::Begin);

        for (uint b = 0; b < newNumBlocks; b++)
        {
            Package::ChunkBlock block{};
            block.uncomprSize = qMin((uint)maxBlockSize, dataBlockLeft);
            dataBlockLeft -= block.uncomprSize;
            block.uncompressedBuffer = new quint8[block.uncomprSize];
            if (block.uncompressedBuffer == nullptr)
                CRASH_MSG((QString("Out of memory! - amount: ") +
                           QString::number(block.uncomprSize)).toStdString().c_str());
            inputStream.ReadToBuffer(block.uncompressedBuffer, block.uncomprSize);
            blocks.push_back(block);
        }
    }

    #pragma omp parallel for
    for (int b = 0; b < blocks.count(); b++)
    {
        Package::ChunkBlock block = blocks[b];
        if (type == StorageTypes::extLZO || type == StorageTypes::pccLZO)
        {
            if (LzoCompress(block.uncompressedBuffer, block.uncomprSize, &block.compressedBuffer, &block.comprSize) == -100)
                CRASH_MSG("Out of memory!");
        }
        else if (type == StorageTypes::extZlib || type == StorageTypes::pccZlib)
        {
            if (ZlibCompress(block.uncompressedBuffer, block.uncomprSize, &block.compressedBuffer, &block.comprSize, 1) == -100)
                CRASH_MSG("Out of memory!");
        }
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
    ouputStream.WriteInt32(inputData.size());
    for (int b = 0; b < blocks.count(); b++)
    {
        const Package::ChunkBlock& block = blocks[b];
        ouputStream.WriteUInt32(block.comprSize);
        ouputStream.WriteUInt32(block.uncomprSize);
        delete[] block.compressedBuffer;
        delete[] block.uncompressedBuffer;
    }

    return ouputStream.ToArray();
}

const ByteBuffer Texture::decompressTexture(Stream &stream, StorageTypes type, int uncompressedSize, int compressedSize)
{
    auto data = ByteBuffer(uncompressedSize);
    uint blockTag = stream.ReadUInt32();
    if (blockTag != textureTag)
    {
        PERROR(QString("Data texture tag wrong!\n"));
        return ByteBuffer();
    }
    uint blockSize = stream.ReadUInt32();
    if (blockSize != maxBlockSize)
    {
        PERROR(QString("Data texture block size is wrong!\n"));
        return ByteBuffer();
    }
    uint compressedChunkSize = stream.ReadUInt32();
    uint uncompressedChunkSize = stream.ReadUInt32();
    if (uncompressedChunkSize != (uint)uncompressedSize)
    {
        PERROR(QString("Data texture uncompressed size diffrent than expected!\n"));
        return ByteBuffer();
    }

    uint blocksCount = (uncompressedChunkSize + maxBlockSize - 1) / maxBlockSize;
    if ((compressedChunkSize + SizeOfChunk + SizeOfChunkBlock * blocksCount) != (uint)compressedSize)
    {
        PERROR(QString("Data texture compressed size diffrent than expected!\n"));
        return ByteBuffer();
    }

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
        if (block.compressedBuffer == nullptr)
            CRASH_MSG((QString("Out of memory! - amount: ") +
                       QString::number(blocks[b].comprSize)).toStdString().c_str());
        stream.ReadToBuffer(block.compressedBuffer, blocks[b].comprSize);
        block.uncompressedBuffer = new quint8[maxBlockSize * 2];
        if (block.uncompressedBuffer == nullptr)
            CRASH_MSG((QString("Out of memory! - amount: ") +
                       QString::number(maxBlockSize * 2)).toStdString().c_str());
        blocks[b] = block;
    }

    bool errorFlag = false;
    if (type == StorageTypes::extLZO || type == StorageTypes::pccLZO)
    {
        for (int b = 0; b < blocks.count(); b++)
        {
            uint dstLen = maxBlockSize * 2;
            Package::ChunkBlock block = blocks[b];
            LzoDecompress(block.compressedBuffer, block.comprSize, block.uncompressedBuffer, &dstLen);
            if (dstLen != block.uncomprSize)
                errorFlag = true;
        }
    }
    else if (type == StorageTypes::extZlib || type == StorageTypes::pccZlib)
    {
        #pragma omp parallel for
        for (int b = 0; b < blocks.count(); b++)
        {
            uint dstLen = maxBlockSize * 2;
            Package::ChunkBlock block = blocks[b];
            if (ZlibDecompress(block.compressedBuffer, block.comprSize, block.uncompressedBuffer, &dstLen) == -100)
                CRASH_MSG("Out of memory!");
            if (dstLen != block.uncomprSize)
                errorFlag = true;
        }
    }
    else
        CRASH_MSG("Compression type not expected!");

    if (errorFlag)
    {
        PERROR(QString("ERROR: Decompressed data size not expected!"));
        return ByteBuffer();
    }

    int dstPos = 0;
    for (int b = 0; b < blocks.count(); b++)
    {
        memcpy(data.ptr() + dstPos, blocks[b].uncompressedBuffer, blocks[b].uncomprSize);
        dstPos += blocks[b].uncomprSize;
        delete[] blocks[b].compressedBuffer;
        delete[] blocks[b].uncompressedBuffer;
    }

    return data;
}

uint Texture::getCrcData(ByteBuffer data)
{
    if (data.ptr() == nullptr)
        return 0;
    if (properties->getProperty("Format").valueName == "PF_NormalMap_HQ") // only ME1 and ME2
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

bool Texture::existMipmap(int width, int height)
{
    for (int l = 0; l < mipMapsList.count(); l++)
    {
        if (mipMapsList[l].width == width && mipMapsList[l].height == height)
            return true;
    }
    return false;
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
            mipMapData = decompressTexture(dynamic_cast<Stream &>(*textureData), mipmap.storageType, mipmap.uncompressedSize, mipmap.compressedSize);
            if (mipMapData.ptr() == nullptr)
            {
                PERROR(QString("StorageType: ") + QString::number(mipmap.storageType) +
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
                QString archive = properties->getProperty("TextureFileCacheName").valueName;
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
                            CRASH_MSG((QString("More instances of TFC file: ") + archive + ".tfc").toStdString().c_str());
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
                PERROR("StorageType: " + QString::number(mipmap.storageType) +
                       "\nExternal file offset: " + QString::number(mipmap.dataOffset) + "\n");
                return ByteBuffer();
            }
            auto fs = FileStream(filename, FileMode::Open, FileAccess::ReadOnly);
            fs.JumpTo(mipmap.dataOffset);
            if (mipmap.storageType == StorageTypes::extLZO || mipmap.storageType == StorageTypes::extZlib)
            {
                mipMapData = decompressTexture(dynamic_cast<Stream &>(fs), mipmap.storageType, mipmap.uncompressedSize, mipmap.compressedSize);
                if (mipMapData.ptr() == nullptr)
                {
                    PERROR(QString("File: ") + filename +
                        "\nStorageType: " + QString::number(mipmap.storageType) +
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
            newData.WriteUInt32(0);
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
