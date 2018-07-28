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

#include "MipMaps.h"
#include "GameData.h"
#include "Texture.h"
#include "Package.h"
#include "Helpers/MiscHelpers.h"

static const TFCTexture guids[] =
{
    {
        { 0x11, 0xD3, 0xC3, 0x39, 0xB3, 0x40, 0x44, 0x61, 0xBB, 0x0E, 0x76, 0x75, 0x2D, 0xF7, 0xC3, 0xB1 },
        "Texture2D"
    },
    {
        { 0x2B, 0x7D, 0x2F, 0x16, 0x63, 0x52, 0x4F, 0x3E, 0x97, 0x5B, 0x0E, 0xF2, 0xC1, 0xEB, 0xC6, 0x5D },
        "Format"
    },
    {
        { 0x81, 0xCD, 0x12, 0x5C, 0xBB, 0x72, 0x40, 0x2D, 0x99, 0xB1, 0x63, 0x8D, 0xC0, 0xA7, 0x6E, 0x03 },
        "IntProperty"
    },
    {
        { 0xA5, 0xBE, 0xFF, 0x48, 0xB4, 0x7A, 0x47, 0xB0, 0xB2, 0x07, 0x2B, 0x35, 0x96, 0x39, 0x55, 0xFB },
        "ByteProperty"
    },
    {
        { 0x59, 0xF2, 0x1B, 0x17, 0xD0, 0xFE, 0x42, 0x3E, 0x94, 0x8A, 0x26, 0xBE, 0x26, 0x3C, 0x46, 0x2E },
        "SizeX"
    },
    {
        { 0x0C, 0x70, 0x7A, 0x01, 0xA0, 0xC1, 0x49, 0xB4, 0x97, 0x8D, 0x3B, 0xA4, 0x94, 0x71, 0xBE, 0x43 },
        "SizeY"
    },
};


PixelFormat MipMaps::changeTextureType(PixelFormat gamePixelFormat, PixelFormat texturePixelFormat, Texture &texture)
{
    if ((gamePixelFormat == PixelFormat::DXT5 || gamePixelFormat == PixelFormat::DXT1 || gamePixelFormat == PixelFormat::ATI2) &&
        (texturePixelFormat == PixelFormat::RGB || texturePixelFormat == PixelFormat::ARGB ||
         texturePixelFormat == PixelFormat::ATI2 || texturePixelFormat == PixelFormat::V8U8))
    {
        if (texturePixelFormat == PixelFormat::ARGB && texture.getProperties()->exists("CompressionSettings") &&
            texture.getProperties()->getProperty("CompressionSettings").valueName == "TC_OneBitAlpha")
        {
            gamePixelFormat = PixelFormat::ARGB;
            texture.getProperties()->setByteValue("Format", Image::getEngineFormatType(gamePixelFormat), "EPixelFormat");
            texture.getProperties()->removeProperty("CompressionSettings");
        }
        else if (texturePixelFormat == PixelFormat::ATI2 &&
            texture.getProperties()->exists("CompressionSettings") &&
            texture.getProperties()->getProperty("CompressionSettings").valueName == "TC_Normalmap")
        {
            gamePixelFormat = PixelFormat::ATI2;
            texture.getProperties()->setByteValue("Format", Image::getEngineFormatType(gamePixelFormat), "EPixelFormat");
            texture.getProperties()->setByteValue("CompressionSettings", "TC_NormalmapHQ", "TextureCompressionSettings");
        }
        else if (GameData::gameType != MeType::ME3_TYPE && texturePixelFormat == PixelFormat::ARGB &&
            texture.getProperties()->exists("CompressionSettings") &&
            (texture.getProperties()->getProperty("CompressionSettings").valueName == "TC_Normalmap" ||
            texture.getProperties()->getProperty("CompressionSettings").valueName == "TC_NormalmapHQ"))
        {
            gamePixelFormat = PixelFormat::ARGB;
            texture.getProperties()->setByteValue("Format", Image::getEngineFormatType(gamePixelFormat), "EPixelFormat");
            texture.getProperties()->setByteValue("CompressionSettings", "TC_Normalmap", "TextureCompressionSettings");
        }
        else if ((gamePixelFormat == PixelFormat::DXT5 || gamePixelFormat == PixelFormat::DXT1) &&
            (texturePixelFormat == PixelFormat::ARGB || texturePixelFormat == PixelFormat::RGB) &&
            !texture.getProperties()->exists("CompressionSettings"))
        {
            gamePixelFormat = PixelFormat::ARGB;
            texture.getProperties()->setByteValue("Format", Image::getEngineFormatType(gamePixelFormat), "EPixelFormat");
        }
        else if (GameData::gameType == MeType::ME3_TYPE && gamePixelFormat == PixelFormat::DXT5 &&
            texturePixelFormat == PixelFormat::ARGB &&
            texture.getProperties()->exists("CompressionSettings") &&
            texture.getProperties()->getProperty("CompressionSettings").valueName == "TC_NormalmapAlpha")
        {
            gamePixelFormat = PixelFormat::ARGB;
            texture.getProperties()->setByteValue("Format", Image::getEngineFormatType(gamePixelFormat), "EPixelFormat");
        }
        else if (GameData::gameType == MeType::ME3_TYPE && gamePixelFormat == PixelFormat::DXT1 &&
            texturePixelFormat == PixelFormat::V8U8 &&
            texture.getProperties()->exists("CompressionSettings") &&
            (texture.getProperties()->getProperty("CompressionSettings").valueName == "TC_Normalmap" ||
            texture.getProperties()->getProperty("CompressionSettings").valueName == "TC_NormalmapHQ"))
        {
            gamePixelFormat = PixelFormat::V8U8;
            texture.getProperties()->setByteValue("Format", Image::getEngineFormatType(gamePixelFormat), "EPixelFormat");
            texture.getProperties()->setByteValue("CompressionSettings", "TC_NormalmapUncompressed", "TextureCompressionSettings");
        }
    }

    return gamePixelFormat;
}

QString MipMaps::replaceTextures(QList<MapPackagesToMod> &map, QList<FoundTexture> &textures,
                                 QStringList &pkgsToMarker, QStringList &pkgsToRepack,
                                 QList<ModEntry> &modsToReplace, bool repack,
                                 bool appendMarker, bool verify, bool removeMips, bool ipc)
{
    QString errors = "";
    int lastProgress = -1;
    ulong memorySize = DetectAmountMemoryGB();

    for (int e = 0; e < map.count(); e++)
    {
        if (ipc)
        {
            ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + map.at(e).packagePath);
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
            ConsoleWrite(QString("Package: ") + QString::number(e + 1) + " of " + QString::number(map.count()) +
                         " started: " + map.at(e).packagePath);
        }

        Package package{};
        if (package.Open(g_GameData->GamePath() + map.at(e).packagePath) != 0)
        {
            if (ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR Issue opening package file: ") + map.at(e).packagePath);
                ConsoleSync();
            }
            else
            {
                QString err;
                err += "---- Start --------------------------------------------\n";
                err += "Issue opening package file: " + map.at(e).packagePath + "\n";
                err += "---- End ----------------------------------------------\n\n";
                ConsoleWrite(err);
            }
            continue;
        }

        for (int p = 0; p < map.at(e).textures.count(); p++)
        {
            MapPackagesToModEntry entryMap = map.at(e).textures[p];
            MatchedTexture matched = textures[entryMap.texturesIndex].list.at(entryMap.listIndex);
            ModEntry mod = modsToReplace.at(entryMap.modIndex);
            Texture texture = Texture(package, matched.exportID, package.getExportData(matched.exportID));
            QString fmt = texture.getProperties()->getProperty("Format").valueName;
            PixelFormat pixelFormat = Image::getPixelFormatType(fmt);
            texture.removeEmptyMips();

            Image *image = mod.cacheImage;
            if (image == nullptr)
            {
                FileStream fs = FileStream(mod.memPath, FileMode::Open, FileAccess::ReadOnly);
                fs.JumpTo(mod.memEntryOffset);
                ByteBuffer data = decompressData(fs, mod.memEntrySize);
                image = new Image(data, ImageFormat::DDS);
                if (memorySize > 8 || modsToReplace.count() == 1)
                    mod.cacheImage = image;
            }

            if (image->getMipMaps().first().getOrigWidth() / image->getMipMaps().first().getOrigHeight() !=
                texture.mipMapsList.first().width / texture.mipMapsList.first().height)
            {
                errors += "Error in texture: " + mod.textureName + " This texture has wrong aspect ratio, skipping texture...\n";
                continue;
            }

            if (GameData::gameType == MeType::ME1_TYPE && texture.mipMapsList.count() < 6)
            {
                for (int i = texture.mipMapsList.count() - 1; i != 0; i--)
                    texture.mipMapsList.removeAt(i);
            }

            PixelFormat newPixelFormat = pixelFormat;
            if (mod.markConvert)
                newPixelFormat = changeTextureType(pixelFormat, image->getPixelFormat(), texture);

            if (!image->checkDDSHaveAllMipmaps() ||
                (texture.mipMapsList.count() > 1 && image->getMipMaps().count() <= 1) ||
                (mod.markConvert && image->getPixelFormat() != newPixelFormat) ||
                (!mod.markConvert && image->getPixelFormat() != pixelFormat))
            {
                bool dxt1HasAlpha = false;
                quint8 dxt1Threshold = 128;
                if (pixelFormat == PixelFormat::DXT1 && texture.getProperties()->exists("CompressionSettings") &&
                    texture.getProperties()->getProperty("CompressionSettings").valueName == "TC_OneBitAlpha")
                {
                    dxt1HasAlpha = true;
                    if (image->getPixelFormat() == PixelFormat::ARGB ||
                        image->getPixelFormat() == PixelFormat::DXT3 ||
                        image->getPixelFormat() == PixelFormat::DXT5)
                    {
                        errors += "Warning for texture: " + mod.textureName + ". This texture converted from full alpha to binary alpha.\n";
                    }
                }
                image->correctMips(newPixelFormat, dxt1HasAlpha, dxt1Threshold);
                if (memorySize > 8 || modsToReplace.count() == 1)
                    mod.cacheImage = image;
            }

            // remove lower mipmaps from source image which not exist in game data
            for (int t = 0; t < image->getMipMaps().count(); t++)
            {
                if (image->getMipMaps()[t].getOrigWidth() <= texture.mipMapsList.first().width &&
                    image->getMipMaps()[t].getOrigHeight() <= texture.mipMapsList.first().height &&
                    texture.mipMapsList.count() > 1)
                {
                    bool found = false;
                    for (int m = t; m < image->getMipMaps().count(); m++)
                    {
                        if (!(texture.mipMapsList.at(m).width == image->getMipMaps()[t].getOrigWidth() &&
                              texture.mipMapsList.at(m).height == image->getMipMaps()[t].getOrigHeight()))
                        {
                            found = true;;
                        }
                    }
                    if (!found)
                    {
                        image->getMipMaps().removeAt(t--);
                    }
                }
            }

            bool skip = false;
            // reuse lower mipmaps from game data which not exist in source image
            for (int t = 0; t < texture.mipMapsList.count(); t++)
            {
                if (texture.mipMapsList[t].width <= image->getMipMaps().first().getOrigWidth() &&
                    texture.mipMapsList[t].height <= image->getMipMaps().first().getOrigHeight())
                {
                    bool found = false;
                    for (int m = t; m < image->getMipMaps().count(); m++)
                    {
                        if (!(texture.mipMapsList.at(m).width == image->getMipMaps()[t].getOrigWidth() &&
                              texture.mipMapsList.at(m).height == image->getMipMaps()[t].getOrigHeight()))
                        {
                            found = true;;
                        }
                    }
                    if (!found)
                    {
                        ByteBuffer data = texture.getMipMapData(texture.mipMapsList[t]);
                        if (data.ptr() == nullptr)
                        {
                            errors += QString("Error in game data: ") + matched.path + ", skipping texture...\n";
                            skip = true;
                            break;
                        }
                        MipMap mipmap = MipMap(data, texture.mipMapsList[t].width, texture.mipMapsList[t].height, pixelFormat);
                        image->getMipMaps().push_back(mipmap);
                    }
                }
            }
            if (skip)
                continue;

            if (!texture.getProperties()->exists("LODGroup"))
                texture.getProperties()->setByteValue("LODGroup", "TEXTUREGROUP_Character", "TextureGroup", 1025);

            if (mod.cacheCprMipmaps.count() == 0)
            {
                for (int m = 0; m < image->getMipMaps().count(); m++)
                {
                    if (GameData::gameType == MeType::ME1_TYPE)
                        mod.cacheCprMipmaps.push_back(texture.compressTexture(image->getMipMaps()[m].getData(), Texture::StorageTypes::extLZO));
                    else
                        mod.cacheCprMipmaps.push_back(texture.compressTexture(image->getMipMaps()[m].getData(), Texture::StorageTypes::extZlib));
                }
            }

            if (verify)
                matched.crcs.clear();
            auto mipmaps = QList<Texture::TextureMipMap>();
            for (int m = 0; m < image->getMipMaps().count(); m++)
            {
                if (verify)
                    matched.crcs.push_back(texture.getCrcData(image->getMipMaps()[m].getData()));
                Texture::TextureMipMap mipmap;
                mipmap.width = image->getMipMaps()[m].getOrigWidth();
                mipmap.height = image->getMipMaps()[m].getOrigHeight();
                if (texture.existMipmap(mipmap.width, mipmap.height))
                    mipmap.storageType = texture.getMipmap(mipmap.width, mipmap.height).storageType;
                else
                {
                    mipmap.storageType = texture.getTopMipmap().storageType;
                    if (texture.mipMapsList.count() > 1)
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
                        else if (GameData::gameType == MeType::ME2_TYPE || GameData::gameType == MeType::ME3_TYPE)
                        {
                            if (texture.getProperties()->exists("TextureFileCacheName"))
                            {
                                if (texture.mipMapsList.count() < 6)
                                {
                                    mipmap.storageType = Texture::StorageTypes::pccUnc;
                                    texture.getProperties()->setBoolValue("NeverStream", true);
                                }
                                else
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

                if (GameData::gameType != MeType::ME1_TYPE)
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

                mipmap.width = image->getMipMaps()[m].getWidth();
                mipmap.height = image->getMipMaps()[m].getHeight();
                mipmaps.push_back(mipmap);
                if (texture.mipMapsList.count() == 1)
                    break;
            }

            bool triggerCacheArc = false;
            bool newTfcFile = false;
            bool oldSpace = true;
            QString archiveFile;
            if (texture.getProperties()->exists("TextureFileCacheName"))
            {
                QString archive = texture.getProperties()->getProperty("TextureFileCacheName").valueName;
                if (mod.arcTfcDLC && mod.arcTfcName != archive)
                    mod.arcTexture.clear();

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
                            QStringList files = QDir(g_GameData->bioGamePath(), archive + ".tfc", QDir::NoSort, QDir::Files | QDir::NoSymLinks).entryList();
                            if (files.count() == 1)
                                archiveFile = files.first();
                            else if (files.count() == 0)
                            {
                                DLCArchiveFile = DirName(DLCArchiveFile) + "/Textures_" +
                                        BaseName(DirName(DirName(matched.path))) + ".tfc";
                                if (QFile(DLCArchiveFile).exists())
                                    archiveFile = DLCArchiveFile;
                                else
                                    archiveFile = g_GameData->MainData() + "/Textures.tfc";
                            }
                            else
                                CRASH();
                        }
                    }
                    else
                    {
                        mod.arcTfcDLC = false;
                    }

                    // check if texture fit in old space
                    for (int mip = 0; mip < image->getMipMaps().count(); mip++)
                    {
                        Texture::TextureMipMap testMipmap{};
                        testMipmap.width = image->getMipMaps()[mip].getOrigWidth();
                        testMipmap.height = image->getMipMaps()[mip].getOrigHeight();
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
                            if (mod.cacheCprMipmaps[mip].size() > oldTestMipmap.compressedSize)
                            {
                                oldSpace = false;
                                break;
                            }
                        }
                        if (texture.mipMapsList.count() == 1)
                            break;
                    }

                    long fileLength = QFile(archiveFile).size();
                    if (!oldSpace && fileLength + 0x5000000 > 0x80000000)
                    {
                        archiveFile = "";
                        for (const auto & guid : guids)
                        {
                            const TFCTexture& newGuid = guid;
                            archiveFile = g_GameData->MainData() + "/" + newGuid.name + ".tfc";
                            if (!QFile(archiveFile).exists())
                            {
                                texture.getProperties()->setNameValue("TextureFileCacheName", newGuid.name);
                                texture.getProperties()->setStructValue("TFCFileGuid", "Guid",
                                                                    ByteBuffer(const_cast<quint8 *>(newGuid.guid), 16));
                                FileStream fs = FileStream(archiveFile, FileMode::Create, FileAccess::WriteOnly);
                                fs.WriteFromBuffer(const_cast<quint8 *>(guid.guid), 16);
                                newTfcFile = true;
                                break;
                            }

                            fileLength = QFile(archiveFile).exists();
                            if (fileLength + 0x5000000 < 0x80000000)
                            {
                                texture.getProperties()->setNameValue("TextureFileCacheName", newGuid.name);
                                texture.getProperties()->setStructValue("TFCFileGuid", "Guid",
                                                                    ByteBuffer(const_cast<quint8 *>(newGuid.guid), 16));
                                break;
                            }
                            archiveFile = "";
                        }
                        if (archiveFile == "")
                            CRASH_MSG("No free TFC texture file!");
                    }
                }
                else
                {
                    texture.getProperties()->setNameValue("TextureFileCacheName", mod.arcTfcName);
                    texture.getProperties()->setStructValue("TFCFileGuid", "Guid", ByteBuffer(mod.arcTfcGuid, 16));
                }
            }

            for (int m = 0; m < image->getMipMaps().count(); m++)
            {
                Texture::TextureMipMap mipmap = mipmaps[m];
                mipmap.uncompressedSize = image->getMipMaps()[m].getData().size();
                if (GameData::gameType == MeType::ME1_TYPE)
                {
                    if (mipmap.storageType == Texture::StorageTypes::pccLZO ||
                        mipmap.storageType == Texture::StorageTypes::pccZlib)
                    {
                        if (matched.linkToMaster == -1)
                            mipmap.newData = mod.cacheCprMipmaps[m];
                        else
                        {
                            mipmap.newData = mod.masterTextures.find(matched.linkToMaster).value()[m].newData;
                        }
                        mipmap.compressedSize = mipmap.newData.size();
                    }
                    if (mipmap.storageType == Texture::StorageTypes::pccUnc)
                    {
                        mipmap.compressedSize = mipmap.uncompressedSize;
                        mipmap.newData = image->getMipMaps()[m].getData();
                    }
                    if ((mipmap.storageType == Texture::StorageTypes::extLZO ||
                        mipmap.storageType == Texture::StorageTypes::extZlib) && matched.linkToMaster != -1)
                    {
                        mipmap.compressedSize = mod.masterTextures.find(matched.linkToMaster).value()[m].compressedSize;
                        mipmap.dataOffset = mod.masterTextures.find(matched.linkToMaster).value()[m].dataOffset;
                    }
                }
                else
                {
                    if (mipmap.storageType == Texture::StorageTypes::extZlib ||
                        mipmap.storageType == Texture::StorageTypes::extLZO)
                    {
                        if (mod.cacheCprMipmaps.count() != image->getMipMaps().count())
                            CRASH();
                        mipmap.newData = mod.cacheCprMipmaps[m];
                        mipmap.compressedSize = mipmap.newData.size();
                    }

                    if (mipmap.storageType == Texture::StorageTypes::pccUnc ||
                        mipmap.storageType == Texture::StorageTypes::extUnc)
                    {
                        mipmap.compressedSize = mipmap.uncompressedSize;
                        mipmap.newData = image->getMipMaps()[m].getData();
                    }
                    if (mipmap.storageType == Texture::StorageTypes::extZlib ||
                        mipmap.storageType == Texture::StorageTypes::extLZO ||
                        mipmap.storageType == Texture::StorageTypes::extUnc)
                    {
                        if (mod.arcTexture.count())
                        {
                            triggerCacheArc = true;

                            if (!newTfcFile && oldSpace)
                            {
                                FileStream fs = FileStream(archiveFile, FileMode::Open, FileAccess::WriteOnly);
                                Texture::TextureMipMap oldMipmap = texture.getMipmap(mipmap.width, mipmap.height);
                                fs.JumpTo(oldMipmap.dataOffset);
                                mipmap.dataOffset = oldMipmap.dataOffset;
                                fs.WriteFromBuffer(mipmap.newData);
                            }
                            else
                            {
                                FileStream fs = FileStream(archiveFile, FileMode::Open, FileAccess::WriteOnly);
                                fs.SeekEnd();
                                mipmap.dataOffset = (uint)fs.Position();
                                fs.WriteFromBuffer(mipmap.newData);
                            }
                        }
                        else
                        {
                            if (mod.arcTexture[m].width != mipmap.width ||
                                mod.arcTexture[m].height != mipmap.height)
                                CRASH();
                            mipmap.dataOffset = mod.arcTexture[m].dataOffset;
                        }
                    }
                }
                mipmaps.replace(m, mipmap);
                if (texture.mipMapsList.count() == 1)
                    break;
            }

            texture.replaceMipMaps(mipmaps);
            texture.getProperties()->setIntValue("SizeX", texture.mipMapsList.first().width);
            texture.getProperties()->setIntValue("SizeY", texture.mipMapsList.first().height);
            if (texture.getProperties()->exists("MipTailBaseIdx"))
                texture.getProperties()->setIntValue("MipTailBaseIdx", texture.mipMapsList.count() - 1);

            {
                MemoryStream newData{};
                newData.WriteFromBuffer(texture.getProperties()->toArray());
                newData.WriteFromBuffer(texture.toArray(0, false)); // filled later
                package.setExportData(matched.exportID, newData.ToArray());
            }

            {
                MemoryStream newData{};
                newData.WriteFromBuffer(texture.getProperties()->toArray());
                newData.WriteFromBuffer(texture.toArray(package.exportsTable[matched.exportID].getDataOffset() + (uint)newData.Position()));
                package.setExportData(matched.exportID, newData.ToArray());
            }

            if (GameData::gameType == MeType::ME1_TYPE)
            {
                if (matched.linkToMaster == -1)
                    mod.masterTextures.insert(entryMap.listIndex, texture.mipMapsList);
            }
            else
            {
                if (triggerCacheArc)
                {
                    mod.arcTexture = texture.mipMapsList;
                    memcpy(mod.arcTfcGuid, texture.getProperties()->getProperty("TFCFileGuid").valueStruct.ptr(), 0);
                    mod.arcTfcName = texture.getProperties()->getProperty("TextureFileCacheName").valueName;
                }
            }

            matched.removeEmptyMips = false;
            if (!map.at(e).slave)
            {
                for (int r = 0; r < map.at(e).removeMips->exportIDs.count(); r++)
                {
                    if (map.at(e).removeMips->exportIDs[r] == matched.exportID)
                    {
                        map.at(e).removeMips->exportIDs.removeAt(r);
                        break;
                    }
                }
            }

            mod.instance--;
            if (mod.instance < 0)
                CRASH();
            if (mod.instance == 0)
            {
                if (mod.arcTexture.count() != 0)
                {
                    mod.arcTexture.clear();
                }
                if (mod.cacheCprMipmaps.count() != 0)
                {
                    mod.cacheCprMipmaps.clear();
                }
                mod.cacheImage = nullptr;
                if (mod.masterTextures.count() != 0)
                {
                    mod.masterTextures.clear();
                }
            }

            if (memorySize <= 6 && mod.cacheCprMipmaps.count() != 0 && modsToReplace.count() != 1)
            {
                mod.cacheCprMipmaps.clear();
            }

            modsToReplace.replace(entryMap.modIndex, mod);
            textures[entryMap.texturesIndex].list[entryMap.listIndex] = matched;
        }

        if (removeMips && !map.at(e).slave)
        {
            if (GameData::gameType == MeType::ME1_TYPE)
                removeMipMapsME1(1, textures, pkgsToMarker, ipc);
            else
                removeMipMapsME2ME3(textures, pkgsToMarker, pkgsToRepack, ipc, repack);
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

static bool comparePaths(MapTexturesToMod &e1, MapTexturesToMod &e2)
{
    return e1.packagePath.compare(e2.packagePath, Qt::CaseInsensitive);
}

QString MipMaps::replaceModsFromList(QList<FoundTexture> &textures, QStringList &pkgsToMarker,
                                     QStringList &pkgsToRepack, QList<ModEntry> &modsToReplace,
                                     bool repack, bool appendMarker, bool verify, bool removeMips, bool ipc)
{
    QString errors;
    bool binaryMods = false;

    if (!ipc)
    {
        ConsoleWrite("Preparing...");
    }

    // Remove duplicates
    for (int i = 0; i < modsToReplace.count(); i++)
    {
        ModEntry mod = modsToReplace.at(i);
        for (int l = 0; l < i; l++)
        {
            if (mod.binaryModType)
                binaryMods = true;
            if ((mod.textureCrc != 0 && mod.textureCrc == modsToReplace.at(l).textureCrc) ||
                (mod.binaryModType && modsToReplace.at(l).binaryModType &&
                mod.exportId == modsToReplace.at(l).exportId &&
                mod.packagePath.compare(modsToReplace.at(l).packagePath) == 0))
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
            if (textures.at(k).crc == modsToReplace.at(t).textureCrc)
            {
                index = t;
                break;
            }
        }
        if (index == -1)
            continue;

        for (int t = 0; t < textures[k].list.count(); t++)
        {
            if (textures[k].list[t].path == "")
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

            ModEntry mod = modsToReplace.at(index);
            mod.instance++;
            modsToReplace.replace(index, mod);
        }
    }

    std::sort(map.begin(), map.end(), comparePaths);

    auto mapPackages = QList<MapPackagesToMod>();
    QString previousPath;
    int packagesIndex = -1;
    for (int i = 0; i < map.count(); i++)
    {
        MapPackagesToModEntry entry{};
        entry.modIndex = map.at(i).modIndex;
        entry.texturesIndex = map.at(i).texturesIndex;
        entry.listIndex = map.at(i).listIndex;
        QString path = map.at(i).packagePath.toLower();
        if (previousPath == path)
        {
            MapPackagesToMod mapEntry = mapPackages.at(packagesIndex);
            mapEntry.usage += modsToReplace.at(map.at(i).modIndex).memEntrySize;
            mapEntry.instances += modsToReplace.at(map.at(i).modIndex).instance;
            mapEntry.textures.push_back(entry);
            mapPackages.replace(packagesIndex, mapEntry);
        }
        else
        {
            MapPackagesToMod mapEntry{};
            mapEntry.textures.push_back(entry);
            mapEntry.packagePath = map.at(i).packagePath;
            mapEntry.usage = modsToReplace.at(map.at(i).modIndex).memEntrySize;
            mapEntry.instances = modsToReplace.at(map.at(i).modIndex).instance;
            mapEntry.removeMips->pkgPath = map.at(i).packagePath;
            previousPath = map.at(i).packagePath.toLower();
            mapPackages.push_back(mapEntry);
            packagesIndex++;
        }
    }
    map.clear();

    std::sort(mapSlaves.begin(), mapSlaves.end(), comparePaths);
    previousPath = "";
    for (int i = 0; i < mapSlaves.count(); i++)
    {
        MapPackagesToModEntry entry{};
        entry.modIndex = mapSlaves.at(i).modIndex;
        entry.texturesIndex = mapSlaves.at(i).texturesIndex;
        entry.listIndex = mapSlaves.at(i).listIndex;
        QString path = mapSlaves.at(i).packagePath.toLower();
        if (previousPath == path)
        {
            MapPackagesToMod mapEntry = mapPackages.at(packagesIndex);
            mapEntry.usage += modsToReplace.at(mapSlaves.at(i).modIndex).memEntrySize;
            mapEntry.instances = modsToReplace.at(mapSlaves.at(i).modIndex).instance;
            mapEntry.textures.push_back(entry);
            mapPackages.replace(packagesIndex, mapEntry);
        }
        else
        {
            MapPackagesToMod mapEntry{};
            mapEntry.textures.push_back(entry);
            mapEntry.packagePath = mapSlaves.at(i).packagePath;
            mapEntry.usage = modsToReplace.at(mapSlaves.at(i).modIndex).memEntrySize;
            mapEntry.instances = modsToReplace.at(mapSlaves.at(i).modIndex).instance;
            mapEntry.slave = true;
            previousPath = mapSlaves.at(i).packagePath.toLower();
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
                if (textures[k].list[t].path == "")
                    continue;
                if (!textures[k].list[t].slave && textures[k].list[t].removeEmptyMips)
                {
                    for (int e = 0; e < mapPackages.count(); e++)
                    {
                        if (!mapPackages[e].slave && mapPackages.at(e).packagePath == textures[k].list[t].path)
                        {
                            mapPackages[e].removeMips->exportIDs.push_back(textures[k].list[t].exportID);
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
        if (!ipc)
        {
            ConsoleWrite("Installing binary mods...");
        }

        for (int i = 0; i < modsToReplace.count(); i++)
        {
            ModEntry mod = modsToReplace.at(i);
            if (mod.binaryModType)
            {
                QString path = g_GameData->GamePath() + mod.packagePath;
                if (!QFile(path).exists())
                {
                    errors += "Warning: File " + path + " not exists in your game setup.\n";
                    continue;
                }
                Package pkg{};
                pkg.Open(path);
                pkg.setExportData(mod.exportId, mod.binaryModData);
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
        if (!ipc)
        {
            ConsoleWrite("Installing texture mods...");
        }

        errors += replaceTextures(mapPackages, textures, pkgsToMarker, pkgsToRepack, modsToReplace,
                                  repack, appendMarker, verify, removeMips, ipc);
    }

    modsToReplace.clear();

    return errors;
}