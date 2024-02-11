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

#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <Helpers/Crc32.h>
#include <GameData/Package.h>
#include <GameData/GameData.h>
#include <Texture/TextureMovie.h>
#include <Program/ConfigIni.h>
#include <Types/MemTypes.h>

TextureMovie::TextureMovie(Package &package, int exportId, const ByteBuffer &data)
{
    dataExportId = exportId;
    packagePath = package.packagePath;
    properties = new Properties(package, data, package.getPropertiesOffset(exportId));
    if (data.size() == properties->propertyEndOffset)
        return;

    textureData = new MemoryStream(data, properties->propertyEndOffset, data.size() - properties->propertyEndOffset);

    if (GameData::gameType != MeType::ME3_TYPE)
    {
        textureData->Skip(12); // 12 zeros
        textureData->SkipInt32(); // position in the package
    }
    storageType = (StorageTypes)textureData->ReadInt32();
    uncompressedSize = textureData->ReadInt32();
    compressedSize = textureData->ReadInt32();
    dataOffset = textureData->ReadInt32();
    if (storageType == StorageTypes::pccUnc)
    {
        dataOffset = textureData->Position();
        quint32 tag = textureData->ReadUInt32();
        if (tag != BIK1_TAG && tag != BIK2_TAG && tag != BIK2_202205_TAG)
            CRASH_MSG("Unsuppported version of bink movie for texture.");
    }
    else if (storageType != StorageTypes::extUnc && storageType == StorageTypes::extUnc2)
    {
        CRASH_MSG("Texture Movies cannot be stored as compressed data! This is not supported.");
    }
}

void TextureMovie::replaceMovieData(ByteBuffer newData, uint offset)
{
    compressedSize = newData.size();
    uncompressedSize = newData.size();

    delete textureData;
    textureData = new MemoryStream();
    if (GameData::gameType != MeType::ME3_TYPE)
    {
        textureData->WriteZeros(16);
    }
    textureData->WriteUInt32(storageType);
    textureData->WriteInt32(uncompressedSize);
    textureData->WriteInt32(compressedSize);
    if (storageType == StorageTypes::pccUnc)
    {
        textureData->WriteUInt32(0);
        dataOffset = textureData->Position();
        textureData->WriteFromBuffer(newData);
    }
    else if (storageType == StorageTypes::extUnc)
    {
        dataOffset = offset;
        textureData->WriteUInt32(dataOffset);
    }
    else
        CRASH();
}

const ByteBuffer TextureMovie::getData()
{
    ByteBuffer data;

    switch (storageType)
    {
    case StorageTypes::pccUnc:
        {
            textureData->JumpTo(dataOffset);
            data = textureData->ReadToBuffer(uncompressedSize);
            break;
        }
    case StorageTypes::pccZlib:
    case StorageTypes::pccOodle:
    case StorageTypes::extZlib:
    case StorageTypes::extOodle:
        {
            CRASH_MSG("Texture Movies cannot be stored as compressed data! This is not supported.");
        }
        break;
    case StorageTypes::extUnc:
    case StorageTypes::extUnc2:
        {
            QString filename;
            QString archive = properties->getProperty("TextureFileCacheName").getValueName();
            filename = g_GameData->MainData() + "/" + archive + ".tfc";
            if (packagePath.contains("/DLC", Qt::CaseInsensitive))
            {
                QString DLCArchiveFile = g_GameData->GamePath() + DirName(packagePath) + "/" + archive + ".tfc";
                if (QFile(DLCArchiveFile).exists())
                    filename = DLCArchiveFile;
                else if (!QFile(filename).exists())
                {
                    QStringList files = FilterByFilename(g_GameData->tfcFiles, archive + ".tfc");
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
                            PERROR(QString("Referenced TFC file not found, do you have a patch for a mod that is not installed?: ") + archive + ".tfc" + "\n");
                        }
                        return ByteBuffer();
                    }
                    else
                    {
                        QString list;
                        foreach(QString file, files)
                            list += file + "\n";
                        PERROR((QString("Multiple instances of TFC file found, this is not supported: ") + archive + ".tfc\n" +
                                   list).toStdString().c_str());
                        return ByteBuffer();
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
                    PERROR(QString("Referenced TFC file not found, do you have a patch for a mod that is not installed?: " + filename + "\n"));
                }
                PERROR(QString("\nPackage: ") + packagePath +
                       "\nStorageType: " + QString::number(storageType) +
                       "\nExport UIndex: " + QString::number(dataExportId + 1) +
                       "\nExternal file offset: " + QString::number(dataOffset) + "\n");
                return ByteBuffer();
            }
            auto fs = FileStream(filename, FileMode::Open, FileAccess::ReadOnly);
            fs.JumpTo(dataOffset);
            quint32 tag = fs.ReadUInt32();
            if (tag != BIK1_TAG && tag != BIK2_TAG && tag != BIK2_202205_TAG)
            {
                if (g_ipc)
                {
                    ConsoleWrite("[IPC]ERROR Unsupported bink movie texture version version");
                    ConsoleSync();
                }
                else
                {
                    PERROR(QString("Unsupported texture movie texture version\n"));
                }
                PERROR(QString("\nPackage: ") + packagePath +
                       "\nStorageType: " + QString::number(storageType) +
                       "\nExport UIndex: " + QString::number(dataExportId + 1) + "\n");
                return ByteBuffer();
            }
            fs.JumpTo(dataOffset);
            data = fs.ReadToBuffer(uncompressedSize);
            break;
        }
    case StorageTypes::empty:
        CRASH();
    }

    return data;
}

uint TextureMovie::getCrcData()
{
    ByteBuffer data = getData();
    uint crc = ~crc32_16bytes_prefetch(data.ptr(), data.size());
    data.Free();
    return crc;
}

const ByteBuffer TextureMovie::toArray()
{
    MemoryStream newData;
    if (GameData::gameType != MeType::ME3_TYPE)
    {
        newData.WriteZeros(16);
    }
    newData.WriteInt32(storageType);
    newData.WriteInt32(uncompressedSize);
    newData.WriteInt32(compressedSize);
    if (storageType == StorageTypes::pccUnc)
    {
        newData.WriteUInt32(0);
        textureData->JumpTo(dataOffset);
        newData.CopyFrom(*textureData, uncompressedSize);
    }
    else if (storageType == StorageTypes::pccZlib ||
             storageType == StorageTypes::pccOodle)
    {
        CRASH_MSG("Texture Movies cannot be stored as compressed data! This is not supported.");
    }
    else
    {
        newData.WriteUInt32(dataOffset);
    }

    return newData.ToArray();
}

TextureMovie::~TextureMovie()
{
    delete textureData;
    delete properties;
}
