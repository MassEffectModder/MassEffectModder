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

#include "MipMaps.h"
#include "GameData.h"
#include "Texture.h"
#include "Package.h"
#include "Misc.h"
#include "Helpers/MiscHelpers.h"
#include "Helpers/Logs.h"
#include "Helpers/QSort.h"

static const quint8 tfcNewGuid[16] = { 0xB4, 0xD2, 0xD7, 0x16, 0x08, 0x4A, 0x4B, 0x99, 0x9F, 0xC9, 0x07, 0x89, 0x87, 0xE0, 0x38, 0x21 };

PixelFormat MipMaps::changeTextureType(PixelFormat gamePixelFormat, PixelFormat texturePixelFormat, Texture &texture)
{
    if ((gamePixelFormat == PixelFormat::DXT5 || gamePixelFormat == PixelFormat::DXT1 || gamePixelFormat == PixelFormat::ATI2) &&
        (texturePixelFormat == PixelFormat::RGB || texturePixelFormat == PixelFormat::ARGB ||
         texturePixelFormat == PixelFormat::ATI2 || texturePixelFormat == PixelFormat::V8U8))
    {
        if (texturePixelFormat == PixelFormat::ARGB && texture.getProperties().exists("CompressionSettings") &&
            texture.getProperties().getProperty("CompressionSettings").valueName == "TC_OneBitAlpha")
        {
            gamePixelFormat = PixelFormat::ARGB;
            texture.getProperties().setByteValue("Format", Image::getEngineFormatType(gamePixelFormat), "EPixelFormat");
            texture.getProperties().removeProperty("CompressionSettings");
        }
        else if (texturePixelFormat == PixelFormat::ATI2 &&
            texture.getProperties().exists("CompressionSettings") &&
            texture.getProperties().getProperty("CompressionSettings").valueName == "TC_Normalmap")
        {
            gamePixelFormat = PixelFormat::ATI2;
            texture.getProperties().setByteValue("Format", Image::getEngineFormatType(gamePixelFormat), "EPixelFormat");
            texture.getProperties().setByteValue("CompressionSettings", "TC_NormalmapHQ", "TextureCompressionSettings");
        }
        else if (GameData::gameType != MeType::ME3_TYPE && texturePixelFormat == PixelFormat::ARGB &&
            texture.getProperties().exists("CompressionSettings") &&
            (texture.getProperties().getProperty("CompressionSettings").valueName == "TC_Normalmap" ||
            texture.getProperties().getProperty("CompressionSettings").valueName == "TC_NormalmapHQ"))
        {
            gamePixelFormat = PixelFormat::ARGB;
            texture.getProperties().setByteValue("Format", Image::getEngineFormatType(gamePixelFormat), "EPixelFormat");
            texture.getProperties().setByteValue("CompressionSettings", "TC_Normalmap", "TextureCompressionSettings");
        }
        else if ((gamePixelFormat == PixelFormat::DXT5 || gamePixelFormat == PixelFormat::DXT1) &&
            (texturePixelFormat == PixelFormat::ARGB || texturePixelFormat == PixelFormat::RGB) &&
            !texture.getProperties().exists("CompressionSettings"))
        {
            gamePixelFormat = PixelFormat::ARGB;
            texture.getProperties().setByteValue("Format", Image::getEngineFormatType(gamePixelFormat), "EPixelFormat");
        }
        else if (GameData::gameType == MeType::ME3_TYPE && gamePixelFormat == PixelFormat::DXT5 &&
            texturePixelFormat == PixelFormat::ARGB &&
            texture.getProperties().exists("CompressionSettings") &&
            texture.getProperties().getProperty("CompressionSettings").valueName == "TC_NormalmapAlpha")
        {
            gamePixelFormat = PixelFormat::ARGB;
            texture.getProperties().setByteValue("Format", Image::getEngineFormatType(gamePixelFormat), "EPixelFormat");
        }
        else if (GameData::gameType == MeType::ME3_TYPE && gamePixelFormat == PixelFormat::DXT1 &&
            (texturePixelFormat == PixelFormat::ARGB || texturePixelFormat == PixelFormat::V8U8) &&
            texture.getProperties().exists("CompressionSettings") &&
            (texture.getProperties().getProperty("CompressionSettings").valueName == "TC_Normalmap" ||
            texture.getProperties().getProperty("CompressionSettings").valueName == "TC_NormalmapHQ"))
        {
            gamePixelFormat = PixelFormat::V8U8;
            texture.getProperties().setByteValue("Format", Image::getEngineFormatType(gamePixelFormat), "EPixelFormat");
            texture.getProperties().setByteValue("CompressionSettings", "TC_NormalmapUncompressed", "TextureCompressionSettings");
        }
    }

    return gamePixelFormat;
}

void MipMaps::RemoveLowerMips(Image *image, Texture *texture)
{
    for (int t = 0; t < image->getMipMaps().count(); t++)
    {
        if (image->getMipMaps()[t]->getOrigWidth() <= texture->mipMapsList.first().width &&
            image->getMipMaps()[t]->getOrigHeight() <= texture->mipMapsList.first().height &&
            texture->mipMapsList.count() > 1)
        {
            bool found = false;
            for (int m = 0; m < texture->mipMapsList.count(); m++)
            {
                if (texture->mipMapsList[m].width == image->getMipMaps()[t]->getOrigWidth() &&
                    texture->mipMapsList[m].height == image->getMipMaps()[t]->getOrigHeight())
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                image->removeMipByIndex(t--);
            }
        }
    }
}

void MipMaps::RemoveLowerMips(Image *image)
{
    for (int t = 0; t < image->getMipMaps().count(); t++)
    {
        if (image->getMipMaps()[t]->getOrigWidth() < 4 ||
            image->getMipMaps()[t]->getOrigHeight() < 4)
        {
            image->removeMipByIndex(t--);
        }
    }
}

void MipMaps::AddMissingLowerMips(Image *image, Texture *texture)
{
    for (int t = 0; t < texture->mipMapsList.count(); t++)
    {
        if (texture->mipMapsList[t].width <= image->getMipMaps().first()->getOrigWidth() &&
            texture->mipMapsList[t].height <= image->getMipMaps().first()->getOrigHeight())
        {
            bool found = false;
            for (int m = 0; m < image->getMipMaps().count(); m++)
            {
                if (image->getMipMaps()[m]->getOrigWidth() == texture->mipMapsList[t].width &&
                    image->getMipMaps()[m]->getOrigHeight() == texture->mipMapsList[t].height)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                auto mipmap = new MipMap(texture->mipMapsList[t].width,
                                         texture->mipMapsList[t].height,
                                         image->getPixelFormat());
                image->getMipMaps().push_back(mipmap);
            }
        }
    }
}

QString MipMaps::replaceTextures(QList<MapPackagesToMod> &map, QList<FoundTexture> &textures,
                                 QStringList &pkgsToMarker, QStringList &pkgsToRepack,
                                 QList<ModEntry> &modsToReplace, bool repack,
                                 bool appendMarker, bool verify, bool removeMips, int cacheAmount)
{
    QString errors = "";
    int lastProgress = -1;
    int memoryAmount = DetectAmountMemoryGB();
    if (memoryAmount == 0)
        memoryAmount = 16;
    quint64 cacheUsage = 0;
    quint64 cacheLimit = (memoryAmount - 2) * 1024ULL * 1024 * 1024;
    if (cacheAmount >= 0 && cacheAmount <= 100)
        cacheLimit = (quint64)((memoryAmount * 1024ULL * 1024 * 1024) * (cacheAmount / 100.0));

    for (int e = 0; e < map.count(); e++)
    {
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + map[e].packagePath);
            int newProgress = (e + 1) * 100 / map.count();
            if (lastProgress != newProgress)
            {
                ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
                lastProgress = newProgress;
            }
            ConsoleSync();
        }
        else
        {
            PINFO(QString("Package: ") + QString::number(e + 1) + " of " + QString::number(map.count()) +
                         " started: " + map[e].packagePath + "\n");
        }

        Package package{};
        if (package.Open(g_GameData->GamePath() + map[e].packagePath) != 0)
        {
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR Issue opening package file: ") + map[e].packagePath);
                ConsoleSync();
            }
            else
            {
                QString err;
                err += "---- Start --------------------------------------------\n";
                err += "Issue opening package file: " + map[e].packagePath + "\n";
                err += "---- End ----------------------------------------------\n\n";
                PERROR(err);
            }
            continue;
        }

        for (int p = 0; p < map[e].textures.count(); p++)
        {
            MapPackagesToModEntry entryMap = map[e].textures[p];
            MatchedTexture matched = textures[entryMap.texturesIndex].list[entryMap.listIndex];
            ModEntry mod = modsToReplace[entryMap.modIndex];
            auto exportData = package.getExportData(matched.exportID);
            if (exportData.ptr() == nullptr)
            {
                if (g_ipc)
                {
                    ConsoleWrite(QString("[IPC]ERROR Texture ") + mod.textureName +
                                 " has broken export data in package: " +
                                 matched.path + "\nExport Id: " + QString::number(matched.exportID + 1) + "\nSkipping...");
                    ConsoleSync();
                }
                else
                {
                    PERROR(QString("Error: Texture ") + mod.textureName +
                           " has broken export data in package: " +
                           matched.path + "\nExport Id: " + QString::number(matched.exportID + 1) + "\nSkipping...\n");
                }
                continue;
            }
            Texture texture = Texture(package, matched.exportID, exportData);
            exportData.Free();
            QString fmt = texture.getProperties().getProperty("Format").valueName;
            PixelFormat pixelFormat = Image::getPixelFormatType(fmt);
            texture.removeEmptyMips();

            if (GameData::gameType == MeType::ME1_TYPE && texture.mipMapsList.count() < 6)
            {
                for (int i = texture.mipMapsList.count() - 1; i != 0; i--)
                    texture.mipMapsList.removeAt(i);
            }

            if (!texture.getProperties().exists("LODGroup"))
                texture.getProperties().setByteValue("LODGroup", "TEXTUREGROUP_Character", "TextureGroup", 1025);

            Image *image = nullptr;
            if (mod.cacheCprMipmaps.count() == 0)
            {
                FileStream fs = FileStream(mod.memPath, FileMode::Open, FileAccess::ReadOnly);
                fs.JumpTo(mod.memEntryOffset);
                ByteBuffer data = decompressData(fs, mod.memEntrySize);
                if (data.size() == 0)
                {
                    if (g_ipc)
                    {
                        ConsoleWrite(QString("[IPC]ERROR ") + mod.textureName + " MEM file: " + mod.memPath);
                        ConsoleSync();
                    }
                    PERROR(QString("Failed decompress data: ") + mod.textureName +
                           " MEM file: " + mod.memPath + "\n");
                    continue;
                }
                image = new Image(data, ImageFormat::DDS);
                data.Free();

                if (!Misc::CheckImage(*image, texture, mod.textureName))
                {
                    errors += "Error in texture: " + mod.textureName + " This texture has wrong aspect ratio, skipping texture...\n";
                    delete image;
                    continue;
                }

                PixelFormat newPixelFormat = pixelFormat;
                if (mod.markConvert)
                    newPixelFormat = changeTextureType(pixelFormat, image->getPixelFormat(), texture);
                mod.cachedPixelFormat = newPixelFormat;

                errors += Misc::CorrectTexture(image, texture, pixelFormat, newPixelFormat,
                                               mod.markConvert, mod.textureName);

                // remove lower mipmaps below 4x4
                RemoveLowerMips(image);

                if (verify)
                    matched.crcs.clear();
                mod.cacheSize = 0;
                for (int m = 0; m < image->getMipMaps().count(); m++)
                {
                    if (verify)
                        matched.crcs.push_back(texture.getCrcData(image->getMipMaps()[m]->getRefData()));
                    if (GameData::gameType == MeType::ME1_TYPE)
                        mod.cacheCprMipmapsStorageType = Texture::StorageTypes::extLZO;
                    else
                        mod.cacheCprMipmapsStorageType = Texture::StorageTypes::extZlib;
                    mod.cacheCprMipmapsDecompressedSize.push_back(image->getMipMaps()[m]->getRefData().size());
                    auto data = texture.compressTexture(image->getMipMaps()[m]->getRefData(),
                                                        mod.cacheCprMipmapsStorageType, repack);
                    mod.cacheCprMipmaps.push_back(MipMap(data, image->getMipMaps()[m]->getOrigWidth(),
                                                  image->getMipMaps()[m]->getOrigHeight(), mod.cachedPixelFormat, true));
                    mod.cacheSize += data.size();
                    data.Free();
                }
                cacheUsage += mod.cacheSize;
            }
            else
            {
                if (mod.markConvert)
                    changeTextureType(pixelFormat, mod.cachedPixelFormat, texture);
            }

            int indexOfPackageMipmap = mod.cacheCprMipmaps.count() - 6;
            auto mipmaps = QList<Texture::TextureMipMap>();
            for (int m = 0; m < mod.cacheCprMipmaps.count(); m++)
            {
                Texture::TextureMipMap mipmap;
                mipmap.width = mod.cacheCprMipmaps[m].getOrigWidth();
                mipmap.height = mod.cacheCprMipmaps[m].getOrigHeight();
                mipmap.storageType = texture.getTopMipmap().storageType;
                if (texture.mipMapsList.count() > 1)
                {
                    if (m >= indexOfPackageMipmap)
                    {
                        mipmap.storageType = texture.getBottomMipmap().storageType;
                    }
                    else
                    {
                        if (texture.existMipmap(mipmap.width, mipmap.height))
                            mipmap.storageType = texture.getMipmap(mipmap.width, mipmap.height).storageType;
                        else
                        {
                            if (GameData::gameType == MeType::ME1_TYPE && matched.linkToMaster == -1)
                            {
                                if (mipmap.storageType == Texture::StorageTypes::pccUnc)
                                {
                                    mipmap.storageType = Texture::StorageTypes::pccLZO;
                                }
                            }
                            else if (GameData::gameType == MeType::ME1_TYPE && matched.linkToMaster != -1)
                            {
                                if (mipmap.storageType == Texture::StorageTypes::pccUnc ||
                                    mipmap.storageType == Texture::StorageTypes::pccLZO ||
                                    mipmap.storageType == Texture::StorageTypes::pccZlib)
                                {
                                    mipmap.storageType = Texture::StorageTypes::extLZO;
                                }
                            }
                            else if (GameData::gameType == MeType::ME2_TYPE ||
                                     GameData::gameType == MeType::ME3_TYPE)
                            {
                                if (texture.getProperties().exists("TextureFileCacheName"))
                                {
                                    if (GameData::gameType == MeType::ME2_TYPE)
                                        mipmap.storageType = Texture::StorageTypes::extLZO;
                                    else
                                        mipmap.storageType = Texture::StorageTypes::extZlib;
                                }
                            }
                        }
                    }
                }

                if (GameData::gameType == MeType::ME2_TYPE)
                {
                    if (mipmap.storageType == Texture::StorageTypes::extLZO)
                        mipmap.storageType = Texture::StorageTypes::extZlib;
                    if (mipmap.storageType == Texture::StorageTypes::pccLZO)
                        mipmap.storageType = Texture::StorageTypes::pccZlib;
                }

                if (mod.arcTexture.count() != 0)
                {
                    if (mod.arcTexture[m].storageType != mipmap.storageType)
                    {
                        mod.arcTexture.clear();
                    }
                }

                mipmap.width = mod.cacheCprMipmaps[m].getWidth();
                mipmap.height = mod.cacheCprMipmaps[m].getHeight();
                mipmaps.push_back(mipmap);
                if (texture.mipMapsList.count() == 1)
                    break;
            }

            bool triggerCacheArc = false;
            bool newTfcFile = false;
            bool oldSpace = true;
            QString archiveFile;
            if (texture.getProperties().exists("TextureFileCacheName"))
            {
                QString archive = texture.getProperties().getProperty("TextureFileCacheName").valueName;
                if (mod.arcTfcDLC && mod.arcTfcName != archive)
                {
                    mod.arcTexture.clear();
                }

                if (mod.arcTexture.count() == 0)
                {
                    archiveFile = g_GameData->MainData() + "/" + archive + ".tfc";
                    if (matched.path.contains("/DLC", Qt::CaseInsensitive))
                    {
                        mod.arcTfcDLC = true;
                        QString DLCArchiveFile = g_GameData->GamePath() + DirName(matched.path) + "/" + archive + ".tfc";
                        if (QFile(DLCArchiveFile).exists())
                            archiveFile = DLCArchiveFile;
                        else if (!QFile(archiveFile).exists())
                        {
                            QStringList files = g_GameData->tfcFiles.filter(QRegExp("*" + archive + ".tfc",
                                                                                    Qt::CaseInsensitive, QRegExp::Wildcard));
                            if (files.count() == 1)
                                archiveFile = g_GameData->GamePath() + files.first();
                            else if (files.count() == 0)
                            {
                                FileStream fs = FileStream(DLCArchiveFile, FileMode::Create, FileAccess::WriteOnly);
                                fs.WriteFromBuffer(texture.getProperties().getProperty("TFCFileGuid").valueStruct);
                                archiveFile = DLCArchiveFile;
                                newTfcFile = true;
                            }
                            else
                            {
                                QString list;
                                foreach(QString file, files)
                                    list += file + "\n";
                                CRASH_MSG((QString("More instances of TFC file: ") + archive + ".tfc\n" +
                                           list + "package: " + matched.path + "\n" +
                                           "export id: " + QString::number(matched.exportID + 1)).toStdString().c_str());
                            }
                        }
                    }
                    else
                    {
                        mod.arcTfcDLC = false;
                    }

                    // check if texture fit in old space
                    for (int mip = 0; mip < mod.cacheCprMipmaps.count(); mip++)
                    {
                        Texture::TextureMipMap testMipmap{};
                        testMipmap.width = mod.cacheCprMipmaps[mip].getOrigWidth();
                        testMipmap.height = mod.cacheCprMipmaps[mip].getOrigHeight();
                        if (texture.existMipmap(testMipmap.width, testMipmap.height))
                            testMipmap.storageType = texture.getMipmap(testMipmap.width, testMipmap.height).storageType;
                        else
                        {
                            oldSpace = false;
                            break;
                        }

                        if (testMipmap.storageType == Texture::StorageTypes::extZlib ||
                            testMipmap.storageType == Texture::StorageTypes::extLZO)
                        {
                            Texture::TextureMipMap oldTestMipmap = texture.getMipmap(testMipmap.width, testMipmap.height);
                            if (mod.cacheCprMipmaps[mip].getRefData().size() > oldTestMipmap.compressedSize)
                            {
                                oldSpace = false;
                                break;
                            }
                        }
                        if (texture.mipMapsList.count() == 1)
                            break;
                    }

                    quint32 fileLength = QFile(archiveFile).size();
                    if (!oldSpace && fileLength + 0x5000000UL > 0x80000000UL)
                    {
                        archiveFile = "";
                        ByteBuffer guid(tfcNewGuid, 16);
                        for (int indexTfc = 0; indexTfc < 100; indexTfc++)
                        {
                            guid.ptr()[0] = indexTfc;
                            QString tfcNewName = QString().sprintf("TexturesMEM%02d", indexTfc);
                            archiveFile = g_GameData->MainData() + "/" + tfcNewName + ".tfc";
                            if (!QFile(archiveFile).exists())
                            {
                                texture.getProperties().setNameValue("TextureFileCacheName", tfcNewName);
                                texture.getProperties().setStructValue("TFCFileGuid", "Guid", guid);
                                FileStream fs = FileStream(archiveFile, FileMode::Create, FileAccess::WriteOnly);
                                fs.WriteFromBuffer(guid);
                                guid.Free();
                                newTfcFile = true;
                                break;
                            }

                            fileLength = QFile(archiveFile).size();
                            if (fileLength + 0x5000000UL < 0x80000000UL)
                            {
                                texture.getProperties().setNameValue("TextureFileCacheName", tfcNewName);
                                texture.getProperties().setStructValue("TFCFileGuid", "Guid", guid);
                                guid.Free();
                                break;
                            }
                            archiveFile = "";
                        }
                        if (archiveFile.length() == 0)
                            CRASH_MSG("No more TFC files available!");
                    }
                }
                else
                {
                    ByteBuffer guid(const_cast<quint8 *>(mod.arcTfcGuid), 16);
                    texture.getProperties().setNameValue("TextureFileCacheName", mod.arcTfcName);
                    texture.getProperties().setStructValue("TFCFileGuid", "Guid", guid);
                    guid.Free();
                }
            }

            for (int m = 0; m < mod.cacheCprMipmaps.count(); m++)
            {
                Texture::TextureMipMap mipmap = mipmaps[m];
                mipmap.uncompressedSize = mod.cacheCprMipmapsDecompressedSize[m];
                if (GameData::gameType == MeType::ME1_TYPE)
                {
                    if (mipmap.storageType == Texture::StorageTypes::pccLZO ||
                        mipmap.storageType == Texture::StorageTypes::pccZlib)
                    {
                        mipmap.newData = mod.cacheCprMipmaps[m].getRefData();
                        mipmap.compressedSize = mipmap.newData.size();
                    }
                    if (mipmap.storageType == Texture::StorageTypes::pccUnc)
                    {
                        mipmap.compressedSize = mipmap.uncompressedSize;
                        if (image)
                        {
                            mipmap.newData = image->getMipMaps()[m]->getRefData();
                        }
                        else
                        {
                            MemoryStream stream(mod.cacheCprMipmaps[m].getRefData());
                            auto mip = texture.decompressTexture(stream, mod.cacheCprMipmapsStorageType,
                                                                 mipmap.uncompressedSize,
                                                                 mod.cacheCprMipmaps[m].getRefData().size());
                            mipmap.newData = mip;
                            mipmap.freeNewData = true;
                        }
                    }
                    if ((mipmap.storageType == Texture::StorageTypes::extLZO ||
                         mipmap.storageType == Texture::StorageTypes::extZlib) && matched.linkToMaster != -1)
                    {
                        auto mip = mod.masterTextures.find(matched.linkToMaster).value()[m];
                        mipmap.compressedSize = mip.compressedSize;
                        mipmap.dataOffset = mip.dataOffset;
                    }
                }
                else
                {
                    if (mipmap.storageType == Texture::StorageTypes::extZlib ||
                        mipmap.storageType == Texture::StorageTypes::extLZO)
                    {
                        mipmap.newData = mod.cacheCprMipmaps[m].getRefData();
                        mipmap.compressedSize = mipmap.newData.size();
                    }

                    if (mipmap.storageType == Texture::StorageTypes::pccUnc ||
                        mipmap.storageType == Texture::StorageTypes::extUnc)
                    {
                        mipmap.compressedSize = mipmap.uncompressedSize;
                        if (image)
                        {
                            mipmap.newData = image->getMipMaps()[m]->getRefData();
                        }
                        else
                        {
                            MemoryStream stream(mod.cacheCprMipmaps[m].getRefData());
                            auto mip = texture.decompressTexture(stream, mod.cacheCprMipmapsStorageType,
                                                                 mipmap.uncompressedSize,
                                                                 mod.cacheCprMipmaps[m].getRefData().size());
                            mipmap.newData = mip;
                            mipmap.freeNewData = true;
                        }
                    }
                    if (mipmap.storageType == Texture::StorageTypes::extZlib ||
                        mipmap.storageType == Texture::StorageTypes::extLZO ||
                        mipmap.storageType == Texture::StorageTypes::extUnc)
                    {
                        if (mod.arcTexture.count() == 0)
                        {
                            triggerCacheArc = true;

                            if (!newTfcFile && oldSpace)
                            {
                                FileStream fs = FileStream(archiveFile, FileMode::Open, FileAccess::ReadWrite);
                                Texture::TextureMipMap oldMipmap = texture.getMipmap(mipmap.width, mipmap.height);
                                fs.JumpTo(oldMipmap.dataOffset);
                                mipmap.dataOffset = oldMipmap.dataOffset;
                                fs.WriteFromBuffer(mipmap.newData);
                            }
                            else
                            {
                                FileStream fs = FileStream(archiveFile, FileMode::Open, FileAccess::ReadWrite);
                                fs.SeekEnd();
                                mipmap.dataOffset = (uint)fs.Position();
                                fs.WriteFromBuffer(mipmap.newData);
                            }
                        }
                        else
                        {
                            if ((mipmap.width >= 4 && mod.arcTexture[m].width != mipmap.width) ||
                                (mipmap.height >= 4 && mod.arcTexture[m].height != mipmap.height))
                            {
                                CRASH();
                            }
                            mipmap.dataOffset = mod.arcTexture[m].dataOffset;
                        }
                    }
                }
                if (mipmaps[m].freeNewData)
                    mipmaps[m].newData.Free();
                mipmaps.replace(m, mipmap);
                if (texture.mipMapsList.count() == 1)
                    break;
            }

            texture.replaceMipMaps(mipmaps);
            texture.getProperties().setIntValue("SizeX", texture.mipMapsList.first().width);
            texture.getProperties().setIntValue("SizeY", texture.mipMapsList.first().height);
            if (texture.getProperties().exists("MipTailBaseIdx"))
                texture.getProperties().setIntValue("MipTailBaseIdx", texture.mipMapsList.count() - 1);
            if (GameData::gameType != MeType::ME1_TYPE &&
                texture.mipMapsList.count() > 6 &&
                !texture.HasExternalMips() &&
                !texture.getProperties().exists("NeverStream"))
            {
                texture.getProperties().setBoolValue("NeverStream", true);
            }

            {
                MemoryStream newData;
                ByteBuffer buffer = texture.getProperties().toArray();
                newData.WriteFromBuffer(buffer);
                buffer.Free();
                buffer = texture.toArray(0, false); // filled later
                newData.WriteFromBuffer(buffer);
                buffer.Free();
                buffer = newData.ToArray();
                package.setExportData(matched.exportID, buffer);
                buffer.Free();
            }

            uint packageDataOffset;
            {
                MemoryStream newData;
                ByteBuffer buffer = texture.getProperties().toArray();
                newData.WriteFromBuffer(buffer);
                buffer.Free();
                packageDataOffset = package.exportsTable[matched.exportID].getDataOffset() + (uint)newData.Position();
                buffer = texture.toArray(packageDataOffset);
                newData.WriteFromBuffer(buffer);
                buffer.Free();
                buffer = newData.ToArray();
                package.setExportData(matched.exportID, buffer);
                buffer.Free();
            }

            if (GameData::gameType == MeType::ME1_TYPE)
            {
                if (matched.linkToMaster == -1)
                {
                    QList<Texture::TextureMipMap> mastersList;
                    mod.CopyMipMapsList(mastersList, texture.mipMapsList);
                    mod.masterTextures.insert(entryMap.listIndex, mastersList);
                }
            }
            else
            {
                if (triggerCacheArc)
                {
                    mod.CopyMipMapsList(mod.arcTexture, texture.mipMapsList);
                    memcpy(mod.arcTfcGuid, texture.getProperties().getProperty("TFCFileGuid").valueStruct.ptr(), 16);
                    mod.arcTfcName = texture.getProperties().getProperty("TextureFileCacheName").valueName;
                }
            }

            matched.removeEmptyMips = false;
            if (!map[e].slave)
            {
                for (int r = 0; r < map[e].removeMips.exportIDs.count(); r++)
                {
                    if (map[e].removeMips.exportIDs[r] == matched.exportID)
                    {
                        map[e].removeMips.exportIDs.removeAt(r);
                        break;
                    }
                }
            }

            mod.instance--;
            if (mod.instance < 0)
                CRASH();
            if (mod.instance == 0)
            {
                foreach(MipMap mip, mod.cacheCprMipmaps)
                {
                    mip.Free();
                }
                mod.cacheCprMipmaps.clear();
                cacheUsage -= mod.cacheSize;

                mod.arcTexture.clear();
                mod.masterTextures.clear();
            }

            delete image;

            if (cacheUsage > cacheLimit)
            {
                foreach(MipMap mip, mod.cacheCprMipmaps)
                {
                    mip.Free();
                }
                mod.cacheCprMipmaps.clear();
                cacheUsage -= mod.cacheSize;
            }

            modsToReplace.replace(entryMap.modIndex, mod);
            textures[entryMap.texturesIndex].list[entryMap.listIndex] = matched;
        }

        if (removeMips && !map[e].slave)
        {
            removeMipMapsPerPackage(1, textures, package, map[e].removeMips,
                                    pkgsToMarker, pkgsToRepack, repack, appendMarker);
        }
        else
        {
            if (package.SaveToFile(repack, false, appendMarker))
            {
                if (repack)
                    pkgsToRepack.removeOne(package.packagePath);
                if (appendMarker)
                    pkgsToMarker.removeOne(package.packagePath);
            }
        }
    }

    return errors;
}

static int comparePaths(const MapTexturesToMod &e1, const MapTexturesToMod &e2)
{
    if (e1.packagePath.compare(e2.packagePath, Qt::CaseInsensitive) < 0)
        return -1;
    if (e1.packagePath.compare(e2.packagePath, Qt::CaseInsensitive) > 0)
        return 1;
    if (e1.texturesIndex < e2.texturesIndex)
        return -1;
    if (e1.texturesIndex > e2.texturesIndex)
        return 1;
    if (e1.listIndex < e2.listIndex)
        return -1;
    if (e1.listIndex > e2.listIndex)
        return 1;
    return 0;
}

QString MipMaps::replaceModsFromList(QList<FoundTexture> &textures, QStringList &pkgsToMarker,
                                     QStringList &pkgsToRepack, QList<ModEntry> &modsToReplace,
                                     bool repack, bool appendMarker, bool verify, bool removeMips,
                                     int cacheAmount)
{
    QString errors;
    bool binaryMods = false;

    // Remove duplicates
    for (int i = 0; i < modsToReplace.count(); i++)
    {
        ModEntry mod = modsToReplace[i];
        for (int l = 0; l < i; l++)
        {
            if (mod.binaryModType)
                binaryMods = true;
            if ((mod.textureCrc != 0 && mod.textureCrc == modsToReplace[l].textureCrc) ||
                (mod.binaryModType && modsToReplace[l].binaryModType &&
                mod.exportId == modsToReplace[l].exportId &&
                mod.packagePath.compare(modsToReplace[l].packagePath) == 0))
            {
                modsToReplace.removeAt(l);
                i--;
                break;
            }
        }
    }

    QList<MapTexturesToMod> map = QList<MapTexturesToMod>();
    QList<MapTexturesToMod> mapSlaves = QList<MapTexturesToMod>();

    for (int k = 0; k < textures.count(); k++)
    {
        int index = -1;
        for (int t = 0; t < modsToReplace.count(); t++)
        {
            if (textures[k].crc == modsToReplace[t].textureCrc)
            {
                index = t;
                break;
            }
        }
        if (index == -1)
            continue;

        for (int t = 0; t < textures[k].list.count(); t++)
        {
            if (textures[k].list[t].path.length() == 0)
                continue;

            MapTexturesToMod entry{};
            entry.packagePath = textures[k].list[t].path;
            entry.modIndex = index;
            entry.listIndex = t;
            entry.texturesIndex = k;
            if (GameData::gameType == MeType::ME1_TYPE && textures[k].list[t].linkToMaster != -1)
                mapSlaves.push_back(entry);
            else
                map.push_back(entry);

            ModEntry mod = modsToReplace[index];
            mod.instance++;
            modsToReplace.replace(index, mod);
        }
    }

    QSort(map, 0, map.count() - 1, comparePaths);
    auto mapPackages = QList<MapPackagesToMod>();
    QString previousPath;
    int packagesIndex = -1;
    for (int i = 0; i < map.count(); i++)
    {
        MapPackagesToModEntry entry{};
        entry.modIndex = map[i].modIndex;
        entry.texturesIndex = map[i].texturesIndex;
        entry.listIndex = map[i].listIndex;
        QString path = map[i].packagePath.toLower();
        if (previousPath == path)
        {
            MapPackagesToMod mapEntry = mapPackages[packagesIndex];
            mapEntry.usage += modsToReplace[map[i].modIndex].memEntrySize;
            mapEntry.instances += modsToReplace[map[i].modIndex].instance;
            mapEntry.textures.push_back(entry);
            mapPackages.replace(packagesIndex, mapEntry);
        }
        else
        {
            MapPackagesToMod mapEntry{};
            mapEntry.textures.push_back(entry);
            mapEntry.packagePath = map[i].packagePath;
            mapEntry.usage = modsToReplace[map[i].modIndex].memEntrySize;
            mapEntry.instances = modsToReplace[map[i].modIndex].instance;
            mapEntry.removeMips.pkgPath = map[i].packagePath;
            previousPath = map[i].packagePath.toLower();
            mapPackages.push_back(mapEntry);
            packagesIndex++;
        }
    }
    map.clear();

    QSort(mapSlaves, 0, mapSlaves.count() - 1, comparePaths);
    previousPath = "";
    for (int i = 0; i < mapSlaves.count(); i++)
    {
        MapPackagesToModEntry entry{};
        entry.modIndex = mapSlaves[i].modIndex;
        entry.texturesIndex = mapSlaves[i].texturesIndex;
        entry.listIndex = mapSlaves[i].listIndex;
        QString path = mapSlaves[i].packagePath.toLower();
        if (previousPath == path)
        {
            MapPackagesToMod mapEntry = mapPackages[packagesIndex];
            mapEntry.usage += modsToReplace[mapSlaves[i].modIndex].memEntrySize;
            mapEntry.instances = modsToReplace[mapSlaves[i].modIndex].instance;
            mapEntry.textures.push_back(entry);
            mapPackages.replace(packagesIndex, mapEntry);
        }
        else
        {
            MapPackagesToMod mapEntry{};
            mapEntry.textures.push_back(entry);
            mapEntry.packagePath = mapSlaves[i].packagePath;
            mapEntry.usage = modsToReplace[mapSlaves[i].modIndex].memEntrySize;
            mapEntry.instances = modsToReplace[mapSlaves[i].modIndex].instance;
            mapEntry.slave = true;
            previousPath = mapSlaves[i].packagePath.toLower();
            mapPackages.push_back(mapEntry);
            packagesIndex++;
        }
    }
    mapSlaves.clear();

    if (removeMips)
    {
        for (int k = 0; k < textures.count(); k++)
        {
            for (int t = 0; t < textures[k].list.count(); t++)
            {
                if (textures[k].list[t].path.length() == 0)
                    continue;
                if (!textures[k].list[t].slave && textures[k].list[t].removeEmptyMips)
                {
                    for (int e = 0; e < mapPackages.count(); e++)
                    {
                        if (!mapPackages[e].slave && mapPackages[e].packagePath == textures[k].list[t].path)
                        {
                            mapPackages[e].removeMips.exportIDs.push_back(textures[k].list[t].exportID);
                            MatchedTexture f = textures[k].list[t];
                            f.removeEmptyMips = false;
                            textures[k].list[t] = f;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (binaryMods)
    {
        if (!g_ipc)
        {
            PINFO("\nInstalling binary mods...\n");
        }

        for (int i = 0; i < modsToReplace.count(); i++)
        {
            ModEntry mod = modsToReplace[i];
            if (mod.binaryModType)
            {
                QString path = g_GameData->GamePath() + mod.packagePath;
                if (!QFile(path).exists())
                {
                    errors += "Warning: File " + path + " not exists in your game setup.\n";
                    mod.binaryModData.Free();
                    continue;
                }
                Package pkg{};
                if (pkg.Open(path) != 0)
                {
                    errors += "Warning: Failed open package: " + path + "\n";
                    mod.binaryModData.Free();
                    continue;
                }
                pkg.setExportData(mod.exportId, mod.binaryModData);
                mod.binaryModData.Free();
                if (pkg.SaveToFile(repack, false, appendMarker))
                {
                    if (repack)
                        pkgsToRepack.removeOne(pkg.packagePath);
                    if (appendMarker)
                        pkgsToMarker.removeOne(pkg.packagePath);
                }
            }
        }
    }

    if (mapPackages.count() != 0)
    {
        if (!g_ipc)
        {
            PINFO("\nInstalling texture mods...\n");
        }

        errors += replaceTextures(mapPackages, textures, pkgsToMarker, pkgsToRepack, modsToReplace,
                                  repack, appendMarker, verify, removeMips, cacheAmount);
    }

    modsToReplace.clear();

    return errors;
}
