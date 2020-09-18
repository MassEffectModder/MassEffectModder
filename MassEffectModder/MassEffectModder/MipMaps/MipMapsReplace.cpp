/*
 * MassEffectModder
 *
 * Copyright (C) 2018-2020 Pawel Kolodziejski
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

#include <MipMaps/MipMaps.h>
#include <GameData/GameData.h>
#include <GameData/Package.h>
#include <Texture/Texture.h>
#include <Texture/TextureMovie.h>
#include <Misc/Misc.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <Helpers/QSort.h>

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

void MipMaps::RemoveLowerMips(Image *image)
{
    for (int t = 0; t < image->getMipMaps().count(); t++)
    {
        if (image->getMipMaps()[t]->getOrigWidth() < 4 &&
            image->getMipMaps()[t]->getOrigHeight() < 4)
        {
            image->removeMipByIndex(t--);
        }
    }
}

bool MipMaps::VerifyTextures(QList<TextureMapEntry> &textures,
                             ProgressCallback callback, void *callbackHandle)
{
    bool errors = false;
    int lastProgress = -1;

    for (int k = 0; k < textures.count(); k++)
    {
#ifdef GUI
        QApplication::processEvents();
#endif
        int newProgress = (k + 1) * 100 / textures.count();
        if (lastProgress != newProgress)
        {
            lastProgress = newProgress;
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
                ConsoleSync();
            }
            else if (callback)
            {
                callback(callbackHandle, newProgress, "Verifing textures");
            }
        }
        for (int t = 0; t < textures[k].list.count(); t++)
        {
            TextureMapEntry foundTexture = textures[k];
            if (textures[k].list[t].path.length() == 0)
                continue;
            TextureMapPackageEntry matchedTexture = textures[k].list[t];
            if (matchedTexture.crcs.count() != 0)
            {
                if (!g_ipc)
                {
                    PINFO(QString("Texture: ") + QString::number(k + 1) + " of " + QString::number(k) +
                          + " " + foundTexture.name + " in " + matchedTexture.path + "\n");
                }
                Package package{};
                if (package.Open(g_GameData->GamePath() + matchedTexture.path) != 0)
                {
                    auto exportData = package.getExportData(matchedTexture.exportID);
                    if (exportData.ptr() == nullptr)
                    {
                        if (g_ipc)
                        {
                            ConsoleWrite(QString("[IPC]ERROR Texture ") + foundTexture.name +
                                         " has broken export data in package: " +
                                         matchedTexture.path + "Export Id: " +
                                         QString::number(matchedTexture.exportID + 1) + " Skipping...");
                            ConsoleSync();
                        }
                        else
                        {
                            PERROR(QString("Error: Texture ") + foundTexture.name +
                                   " has broken export data in package: " +
                                   matchedTexture.path + "\nExport Id: " +
                                   QString::number(matchedTexture.exportID + 1) + "\nSkipping...\n");
                        }
                        errors = true;
                        continue;
                    }
                    Texture texture = Texture(package, matchedTexture.exportID, exportData);
                    exportData.Free();
                    for (int m = 0; m < matchedTexture.crcs.count(); m++)
                    {
                        if (matchedTexture.crcs[m] != texture.getCrcData(texture.getMipMapDataByIndex(m)))
                        {
                            if (g_ipc)
                            {
                                ConsoleWrite(QString("[IPC]ERROR Texture ") + foundTexture.name +
                                             " CRC does not match, mipmap: " +
                                             QString::number(m) + ", Package: " +
                                             matchedTexture.path + ", Export Id: " +
                                             QString::number(matchedTexture.exportID + 1));
                                ConsoleSync();
                            }
                            else
                            {
                                PERROR(QString("Error: Texture ") + foundTexture.name +
                                       " CRC does not match, mipmap: " +
                                       QString::number(m) + "\nPackage: " +
                                       matchedTexture.path + "\nExport Id: " +
                                       QString::number(matchedTexture.exportID + 1) + "\n");
                            }
                            errors = true;
                        }
                    }
                }
            }
        }
    }
    return errors;
}

QString MipMaps::replaceTextures(QList<MapPackagesToMod> &map, QList<TextureMapEntry> &textures,
                                 QStringList &pkgsToMarker, QStringList &pkgsToRepack,
                                 QList<ModEntry> &modsToReplace, bool repack,
                                 bool appendMarker, bool verify, bool removeMips, int cacheAmount,
                                 ProgressCallback callback, void *callbackHandle)
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

    if (g_ipc)
    {
        ConsoleWrite(QString("[IPC]AMOUNT_MEMORY_GB ") + QString::number(memoryAmount));
        ConsoleWrite(QString("[IPC]CACHE_LIMIT ") + QString::number(cacheLimit));
        ConsoleSync();
    }

    for (int e = 0; e < map.count(); e++)
    {
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + map[e].packagePath);
            ConsoleSync();
        }
        else
        {
            PINFO(QString("Package: ") + QString::number(e + 1) + " of " + QString::number(map.count()) +
                         " " + map[e].packagePath + "\n");
        }

        int newProgress = (e + 1) * 100 / map.count();
        if (lastProgress != newProgress)
        {
            lastProgress = newProgress;
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
                ConsoleSync();
            }
            else if (callback)
            {
                callback(callbackHandle, newProgress, "Installing textures");
            }
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
#ifdef GUI
            QApplication::processEvents();
#endif
            MapPackagesToModEntry entryMap = map[e].textures[p];
            TextureMapPackageEntry matched = textures[entryMap.texturesIndex].list[entryMap.listIndex];
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

            if (matched.movieTexture)
            {
                TextureMovie textureMovie = TextureMovie(package, matched.exportID, exportData);
                exportData.Free();

                ByteBuffer data;
                if (mod.injectedMovieTexture.size() != 0)
                {
                    data = mod.injectedMovieTexture;
                }
                else
                {
                    FileStream fs = FileStream(mod.memPath, FileMode::Open, FileAccess::ReadOnly);
                    fs.JumpTo(mod.memEntryOffset);
                    data = Misc::decompressData(fs, mod.memEntrySize);
                }
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
                int w = *reinterpret_cast<qint32 *>(data.ptr() + 20);
                int h = *reinterpret_cast<qint32 *>(data.ptr() + 24);
                textureMovie.getProperties().setIntValue("SizeX", w);
                textureMovie.getProperties().setIntValue("SizeY", h);
                StorageTypes storageType = textureMovie.getStorageType();
                if ((GameData::gameType == MeType::ME2_TYPE ||
                     GameData::gameType == MeType::ME3_TYPE) &&
                    storageType == StorageTypes::extUnc)
                {
                    QString archive = textureMovie.getProperties().getProperty("TextureFileCacheName").valueName;
                    QString archiveFile = g_GameData->MainData() + "/" + archive + ".tfc";
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
                                fs.WriteFromBuffer(textureMovie.getProperties().getProperty("TFCFileGuid").valueStruct);
                                archiveFile = DLCArchiveFile;
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

                    bool oldSpace = mod.memEntrySize <= (long)textureMovie.getUncompressedSize();
                    if (!oldSpace)
                    {
                        quint32 fileLength = QFile(archiveFile).size();
                        if (fileLength + 0x5000000UL > 0x80000000UL)
                        {
                            archiveFile = "";
                            ByteBuffer guid(tfcNewGuid, 16);
                            for (int indexTfc = 0; indexTfc < 100; indexTfc++)
                            {
                                guid.ptr()[0] = indexTfc;
                                QString tfcNewName = QString().asprintf("TexturesMEM%02d", indexTfc);
                                archiveFile = g_GameData->MainData() + "/" + tfcNewName + ".tfc";
                                if (!QFile(archiveFile).exists())
                                {
                                    textureMovie.getProperties().setNameValue("TextureFileCacheName", tfcNewName);
                                    textureMovie.getProperties().setStructValue("TFCFileGuid", "Guid", guid);
                                    FileStream fs = FileStream(archiveFile, FileMode::Create, FileAccess::WriteOnly);
                                    fs.WriteFromBuffer(guid);
                                    guid.Free();
                                    break;
                                }

                                fileLength = QFile(archiveFile).size();
                                if (fileLength + 0x5000000UL < 0x80000000UL)
                                {
                                    textureMovie.getProperties().setNameValue("TextureFileCacheName", tfcNewName);
                                    textureMovie.getProperties().setStructValue("TFCFileGuid", "Guid", guid);
                                    guid.Free();
                                    break;
                                }
                                archiveFile = "";
                            }
                            if (archiveFile.length() == 0)
                                CRASH_MSG("No more TFC files available!");
                        }
                        FileStream archiveFs = FileStream(archiveFile, FileMode::Open, FileAccess::ReadWrite);
                        archiveFs.SeekEnd();
                        textureMovie.replaceMovieData(data, archiveFs.Position());
                        archiveFs.WriteFromBuffer(data);
                    }
                    else
                    {
                        FileStream archiveFs = FileStream(archiveFile, FileMode::Open, FileAccess::ReadWrite);
                        archiveFs.JumpTo(textureMovie.getDataOffset());
                        archiveFs.WriteFromBuffer(data);
                    }
                }
                else
                {
                    textureMovie.replaceMovieData(data, 0);
                }
                if (mod.injectedMovieTexture.size() == 0)
                    data.Free();

                ByteBuffer bufferProperties = textureMovie.getProperties().toArray();
                {
                    MemoryStream newData;
                    newData.WriteFromBuffer(bufferProperties);
                    ByteBuffer bufferTextureData = textureMovie.toArray();
                    newData.WriteFromBuffer(bufferTextureData);
                    bufferTextureData.Free();
                    ByteBuffer bufferTexture = newData.ToArray();
                    package.setExportData(matched.exportID, bufferTexture);
                    bufferTexture.Free();
                }
                bufferProperties.Free();
            }
            else
            {
                Texture texture = Texture(package, matched.exportID, exportData);
                exportData.Free();
                QString fmt = texture.getProperties().getProperty("Format").valueName;
                PixelFormat pixelFormat = Image::getPixelFormatType(fmt);
                texture.removeEmptyMips();

                if (!texture.getProperties().exists("LODGroup"))
                    texture.getProperties().setByteValue("LODGroup", "TEXTUREGROUP_Character", "TextureGroup", 1025);

                Image *image = nullptr;
                if (mod.cacheCprMipmaps.count() == 0)
                {
                    if (mod.injectedTexture != nullptr)
                    {
                        image = mod.injectedTexture;
                    }
                    else
                    {
                        FileStream fs = FileStream(mod.memPath, FileMode::Open, FileAccess::ReadOnly);
                        fs.JumpTo(mod.memEntryOffset);
                        ByteBuffer data = Misc::decompressData(fs, mod.memEntrySize);
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
                    }

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

                    errors += Misc::CorrectTexture(image, texture, newPixelFormat, mod.textureName);

                    // remove lower mipmaps below 4x4 for DXT compressed textures
                    if (mod.cachedPixelFormat == PixelFormat::DXT1 ||
                        mod.cachedPixelFormat == PixelFormat::DXT3 ||
                        mod.cachedPixelFormat == PixelFormat::DXT5 ||
                        mod.cachedPixelFormat == PixelFormat::ATI2)
                    {
                        RemoveLowerMips(image);
                    }
                    if (image->getMipMaps().count() == 0)
                    {
                        if (g_ipc)
                        {
                            ConsoleWrite(QString("[IPC]ERROR Texture ") + mod.textureName +
                                         " has zero mips after mips filtering.\nSkipping...");
                            ConsoleSync();
                        }
                        else
                        {
                            PERROR(QString("Error: Texture ") + mod.textureName +
                                   " has zero mips after mips filtering.\nSkipping...\n");
                        }
                        continue;
                    }

                    if (verify)
                        matched.crcs.clear();
                    mod.cacheSize = 0;
                    for (int m = 0; m < image->getMipMaps().count(); m++)
                    {
                        if (verify)
                            matched.crcs.push_back(texture.getCrcData(image->getMipMaps()[m]->getRefData()));
                        if (GameData::gameType == MeType::ME1_TYPE)
                            mod.cacheCprMipmapsStorageType = StorageTypes::extLZO;
                        else
                            mod.cacheCprMipmapsStorageType = StorageTypes::extZlib;
                        mod.cacheCprMipmapsDecompressedSize.push_back(image->getMipMaps()[m]->getRefData().size());
                        auto data = Package::compressData(image->getMipMaps()[m]->getRefData(),
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

                int forceInternalMip = false;
                bool hadNeverStream = false;
                if (texture.getProperties().exists("NeverStream")) {
                    hadNeverStream = true;
                    texture.getProperties().removeProperty("NeverStream");
                }
                if (GameData::gameType == MeType::ME2_TYPE ||
                    GameData::gameType == MeType::ME3_TYPE)
                {
                    if (mod.textureName.startsWith("CubemapFace"))
                    {
                        forceInternalMip = true;
                    }
                }
                if (GameData::gameType == MeType::ME3_TYPE)
                {
                    if ((pixelFormat == PixelFormat::G8 &&
                        hadNeverStream &&
                        !texture.HasExternalMips()))
                    {
                        forceInternalMip = true;
                    }
                }

                auto mipmapsPre = QList<Texture::TextureMipMap>();
                for (int m = 0; m < mod.cacheCprMipmaps.count(); m++)
                {
                    Texture::TextureMipMap mipmap;
                    if (texture.mipMapsList.count() != 1 && m < 6) // package mips for streaming
                    {
                        mipmap.storageType = StorageTypes::pccUnc;
                    }
                    else if (forceInternalMip)
                    {
                        mipmap.storageType = StorageTypes::pccUnc;
                        if (!texture.getProperties().exists("NeverStream"))
                            texture.getProperties().setBoolValue("NeverStream", true);
                    }
                    else
                    {
                        if (GameData::gameType == MeType::ME1_TYPE)
                        {
                            if (matched.linkToMaster == -1)
                            {
                                if (!texture.getProperties().exists("NeverStream"))
                                    texture.getProperties().setBoolValue("NeverStream", true);
                            }
                            if (matched.linkToMaster == -1 ||
                                texture.mipMapsList.count() == 1)
                            {
                                mipmap.storageType = StorageTypes::pccLZO;
                            }
                            else
                            {
                                mipmap.storageType = StorageTypes::extLZO;
                            }
                        }
                        else if (GameData::gameType == MeType::ME2_TYPE ||
                                 GameData::gameType == MeType::ME3_TYPE)
                        {
                            if (texture.mipMapsList.count() == 1)
                                mipmap.storageType = StorageTypes::pccUnc;
                            else
                                mipmap.storageType = StorageTypes::extZlib;
                        }
                    }

                    mipmapsPre.push_front(mipmap);
                }

                auto mipmaps = QList<Texture::TextureMipMap>();
                for (int m = 0; m < mipmapsPre.count(); m++)
                {
                    Texture::TextureMipMap mipmap = mipmapsPre[m];
                    if (GameData::gameType == MeType::ME2_TYPE ||
                        GameData::gameType == MeType::ME3_TYPE)
                    {
                        if ((mipmap.storageType == StorageTypes::extZlib ||
                             mipmap.storageType == StorageTypes::extLZO ||
                             mipmap.storageType == StorageTypes::extUnc) &&
                             mod.arcTexture.count() != 0)
                        {
                            if (mod.arcTexture[m].storageType != mipmap.storageType)
                            {
                                mod.arcTexture.clear();
                            }
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
                if (GameData::gameType == MeType::ME2_TYPE ||
                    GameData::gameType == MeType::ME3_TYPE)
                {
                    if (!texture.getProperties().exists("TextureFileCacheName"))
                    {
                        FileStream fs = FileStream(g_GameData->MainData() + "/Textures.tfc", FileMode::Open, FileAccess::ReadOnly);
                        ByteBuffer guid = fs.ReadToBuffer(16);
                        texture.getProperties().setNameValue("TextureFileCacheName", "Textures");
                        texture.getProperties().setStructValue("TFCFileGuid", "Guid", guid);
                    }
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

                            if (testMipmap.storageType == StorageTypes::extZlib ||
                                testMipmap.storageType == StorageTypes::extLZO)
                            {
                                Texture::TextureMipMap oldTestMipmap = texture.getMipmap(testMipmap.width, testMipmap.height);
                                if (mod.cacheCprMipmaps[mip].getRefData().size() > oldTestMipmap.compressedSize)
                                {
                                    oldSpace = false;
                                    break;
                                }
                            }
                            else
                            {
                                oldSpace = false;
                                break;
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
                                QString tfcNewName = QString().asprintf("TexturesMEM%02d", indexTfc);
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

                for (int m = 0; m < mipmaps.count(); m++)
                {
                    Texture::TextureMipMap mipmap = mipmaps[m];
                    mipmap.uncompressedSize = mod.cacheCprMipmapsDecompressedSize[m];
                    if (GameData::gameType == MeType::ME1_TYPE)
                    {
                        if (mipmap.storageType == StorageTypes::pccLZO ||
                            mipmap.storageType == StorageTypes::pccZlib)
                        {
                            mipmap.newData = mod.cacheCprMipmaps[m].getRefData();
                            mipmap.compressedSize = mipmap.newData.size();
                        }
                        else if (mipmap.storageType == StorageTypes::pccUnc)
                        {
                            mipmap.compressedSize = mipmap.uncompressedSize;
                            if (image)
                            {
                                mipmap.newData = image->getMipMaps()[m]->getRefData();
                            }
                            else
                            {
                                MemoryStream stream(mod.cacheCprMipmaps[m].getRefData());
                                auto mip = Package::decompressData(stream, mod.cacheCprMipmapsStorageType,
                                                                     mipmap.uncompressedSize,
                                                                     mod.cacheCprMipmaps[m].getRefData().size());
                                mipmap.newData = mip;
                                mipmap.freeNewData = true;
                            }
                        }
                        else if ((mipmap.storageType == StorageTypes::extLZO ||
                                  mipmap.storageType == StorageTypes::extZlib) && matched.linkToMaster != -1)
                        {
                            auto mip = mod.masterTextures.find(matched.linkToMaster).value()[m];
                            mipmap.compressedSize = mip.compressedSize;
                            mipmap.dataOffset = mip.dataOffset;
                        }
                        else
                        {
                            CRASH();
                        }
                    }
                    else
                    {
                        if (mipmap.storageType == StorageTypes::extLZO ||
                            mipmap.storageType == StorageTypes::extZlib ||
                            mipmap.storageType == StorageTypes::pccLZO ||
                            mipmap.storageType == StorageTypes::pccZlib)
                        {
                            mipmap.newData = mod.cacheCprMipmaps[m].getRefData();
                            mipmap.compressedSize = mipmap.newData.size();
                        }
                        else if (mipmap.storageType == StorageTypes::pccUnc ||
                                 mipmap.storageType == StorageTypes::extUnc)
                        {
                            mipmap.compressedSize = mipmap.uncompressedSize;
                            if (image)
                            {
                                mipmap.newData = image->getMipMaps()[m]->getRefData();
                            }
                            else
                            {
                                MemoryStream stream(mod.cacheCprMipmaps[m].getRefData());
                                auto mip = Package::decompressData(stream, mod.cacheCprMipmapsStorageType,
                                                                     mipmap.uncompressedSize,
                                                                     mod.cacheCprMipmaps[m].getRefData().size());
                                mipmap.newData = mip;
                                mipmap.freeNewData = true;
                            }
                        }
                        if (mipmap.storageType == StorageTypes::extZlib ||
                            mipmap.storageType == StorageTypes::extLZO ||
                            mipmap.storageType == StorageTypes::extUnc)
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

                ByteBuffer bufferProperties = texture.getProperties().toArray();
                {
                    MemoryStream newData;
                    newData.WriteFromBuffer(bufferProperties);
                    ByteBuffer bufferTextureData = texture.toArray(0, false); // filled later
                    newData.WriteFromBuffer(bufferTextureData);
                    bufferTextureData.Free();
                    ByteBuffer bufferTexture = newData.ToArray();
                    package.setExportData(matched.exportID, bufferTexture);
                    bufferTexture.Free();
                }
                {
                    MemoryStream newData;
                    newData.WriteFromBuffer(bufferProperties);
                    uint packageDataOffset = package.exportsTable[matched.exportID].getDataOffset() + (uint)newData.Position();
                    ByteBuffer bufferTextureData = texture.toArray(packageDataOffset);
                    newData.WriteFromBuffer(bufferTextureData);
                    bufferTextureData.Free();
                    ByteBuffer bufferTexture = newData.ToArray();
                    package.setExportData(matched.exportID, bufferTexture);
                    bufferTexture.Free();
                }
                bufferProperties.Free();

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

                if (g_ipc)
                {
                    ConsoleWrite(QString("[IPC]CACHE_USAGE ") + QString::number(cacheUsage));
                    ConsoleSync();
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
                else if (cacheUsage > cacheLimit)
                {
                    foreach(MipMap mip, mod.cacheCprMipmaps)
                    {
                        mip.Free();
                    }
                    mod.cacheCprMipmaps.clear();
                    cacheUsage -= mod.cacheSize;
                }

                if (mod.injectedTexture == nullptr)
                    delete image;

                modsToReplace.replace(entryMap.modIndex, mod);
                textures[entryMap.texturesIndex].list[entryMap.listIndex] = matched;
            }
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

    for (int e = 0; e < modsToReplace.count(); e++)
    {
        if (modsToReplace[e].instance > 0)
        {
            PERROR(QString("Mod index: ") + QString::number(e) + ", instances: " + QString::number(modsToReplace[e].instance));
        }
    }

    return errors;
}

static int comparePaths(const MapTexturesToMod &e1, const MapTexturesToMod &e2)
{
    int compResult = AsciiStringCompareCaseIgnore(e1.packagePath, e2.packagePath);
    if (compResult < 0)
        return -1;
    if (compResult > 0)
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

QString MipMaps::replaceModsFromList(QList<TextureMapEntry> &textures, QStringList &pkgsToMarker,
                                     QStringList &pkgsToRepack, QList<ModEntry> &modsToReplace,
                                     bool repack, bool appendMarker, bool verify, bool removeMips,
                                     int cacheAmount, ProgressCallback callback, void *callbackHandle)
{
    QString errors;
    bool binaryMods = false;

    // Remove duplicates
    for (int i = 0; i < modsToReplace.count(); i++)
    {
        if (modsToReplace[i].binaryModType)
            binaryMods = true;
        ModEntry mod = modsToReplace[i];
        // Filter replacing blank textures
        if (TreeScan::IsBlankTexture(mod.textureCrc))
        {
            modsToReplace.removeAt(i);
            i--;
            continue;
        }

        for (int l = 0; l < i; l++)
        {
            if ((mod.textureCrc != 0 && mod.textureCrc == modsToReplace[l].textureCrc) ||
                (mod.binaryModType && modsToReplace[l].binaryModType &&
                mod.exportId == modsToReplace[l].exportId &&
                AsciiStringMatch(mod.packagePath, modsToReplace[l].packagePath)))
            {
                if (!mod.binaryModType)
                {
                    if (g_ipc)
                    {
                        ConsoleWrite(QString("[IPC]MOD_OVERRIDE ") + mod.textureName +
                                     QString().asprintf("_0x%08X", mod.textureCrc) + ", " + mod.memPath);
                        ConsoleSync();
                    }
                    else
                    {
                        PINFO(QString("Override texture: ") + mod.textureName +
                              QString().asprintf("_0x%08X", mod.textureCrc) + ", " + mod.memPath + "\n");
                    }
                }
                else
                {
                    if (g_ipc)
                    {
                        ConsoleWrite(QString("[IPC]MOD_OVERRIDE ") + mod.memPath);
                        ConsoleSync();
                    }
                    else
                    {
                        PINFO(QString("Override binary mod: ") + mod.memPath + "\n");
                    }
                }
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
        if (AsciiStringMatch(previousPath, path))
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
        if (AsciiStringMatch(previousPath, path))
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
                        if (!mapPackages[e].slave &&
                            AsciiStringMatch(mapPackages[e].packagePath, textures[k].list[t].path))
                        {
                            mapPackages[e].removeMips.exportIDs.push_back(textures[k].list[t].exportID);
                            TextureMapPackageEntry f = textures[k].list[t];
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
                if (pkg.SaveToFile(repack, false, appendMarker))
                {
                    if (repack)
                        pkgsToRepack.removeOne(pkg.packagePath);
                    if (appendMarker)
                        pkgsToMarker.removeOne(pkg.packagePath);
                }

                if (!mod.packagePath.contains("/DLC/") &&
                    QDir(g_GameData->DLCData()).exists())
                {
                    QString file = BaseName(mod.packagePath);
                    QStringList DLCs = QDir(g_GameData->DLCData(), "DLC_*", QDir::NoSort, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryList();
                    foreach (QString DLCDir, DLCs)
                    {
                        QString path = g_GameData->DLCData() + "/" + DLCDir + g_GameData->DLCDataSuffix() + "/" + file;
                        if (!QFile(path).exists())
                        {
                            continue;
                        }

                        Package pkg{};
                        if (pkg.Open(path) != 0)
                        {
                            errors += "Warning: Failed open package: " + path + "\n";
                            continue;
                        }
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
                mod.binaryModData.Free();
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
                                  repack, appendMarker, verify, removeMips, cacheAmount,
                                  callback, callbackHandle);
    }

    modsToReplace.clear();

    return errors;
}
