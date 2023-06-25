/*
 * MassEffectModder
 *
 * Copyright (C) 2018-2022 Pawel Kolodziejski
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

namespace {

const quint8 tfcNewGuid[16] = { 0xB4, 0xD2, 0xD7, 0x16, 0x08, 0x4A, 0x4B, 0x99, 0x9F, 0xC9, 0x07, 0x89, 0x87, 0xE0, 0x38, 0x21 };

} // namespace

PixelFormat MipMaps::changeTextureType(PixelFormat gamePixelFormat, PixelFormat texturePixelFormat, Texture &texture)
{
    if (texturePixelFormat == PixelFormat::Internal ||
        texturePixelFormat == PixelFormat::RGBA)
    {
        texturePixelFormat = PixelFormat::ARGB;
    }

    if (gamePixelFormat == texturePixelFormat)
    {
        return gamePixelFormat;
    }

    if (texturePixelFormat == PixelFormat::ARGB && texture.getProperties().exists("CompressionSettings") &&
        texture.getProperties().getProperty("CompressionSettings").getValueName() == "TC_OneBitAlpha")
    {
        gamePixelFormat = PixelFormat::ARGB;
        texture.getProperties().setByteValue("Format", Image::getEngineFormatType(gamePixelFormat), "EPixelFormat");
        texture.getProperties().removeProperty("CompressionSettings");
    }
    else if ((gamePixelFormat == PixelFormat::DXT1) &&
             (texturePixelFormat == PixelFormat::ATI2 || texturePixelFormat == PixelFormat::BC5) &&
        texture.getProperties().exists("CompressionSettings") &&
        texture.getProperties().getProperty("CompressionSettings").getValueName() == "TC_Normalmap")
    {
        gamePixelFormat = texturePixelFormat;
        texture.getProperties().setByteValue("Format", Image::getEngineFormatType(gamePixelFormat), "EPixelFormat");
        if (texturePixelFormat == PixelFormat::ATI2)
            texture.getProperties().setByteValue("CompressionSettings", "TC_NormalmapHQ", "TextureCompressionSettings");
        else
            texture.getProperties().setByteValue("CompressionSettings", "TC_NormalmapBC5", "TextureCompressionSettings");
    }
    else if ((gamePixelFormat == PixelFormat::DXT1 || gamePixelFormat == PixelFormat::ATI2 ||
              gamePixelFormat == PixelFormat::BC5) &&
             (texturePixelFormat == PixelFormat::BC7) &&
        texture.getProperties().exists("CompressionSettings") &&
        (texture.getProperties().getProperty("CompressionSettings").getValueName() == "TC_Normalmap" ||
         texture.getProperties().getProperty("CompressionSettings").getValueName() == "TC_NormalmapHQ" ||
         texture.getProperties().getProperty("CompressionSettings").getValueName() == "TC_NormalmapBC5"))
    {
        gamePixelFormat = PixelFormat::BC7;
        texture.getProperties().setByteValue("Format", Image::getEngineFormatType(gamePixelFormat), "EPixelFormat");
        texture.getProperties().setByteValue("CompressionSettings", "TC_NormalmapBC7", "TextureCompressionSettings");
    }
    else if ((gamePixelFormat == PixelFormat::DXT1 || gamePixelFormat == PixelFormat::ATI2 ||
              gamePixelFormat == PixelFormat::BC5 || gamePixelFormat == PixelFormat::BC7 ||
              gamePixelFormat == PixelFormat::ARGB || gamePixelFormat == PixelFormat::RGB) &&
             (texturePixelFormat == PixelFormat::V8U8 || texturePixelFormat == PixelFormat::ARGB ||
              texturePixelFormat == PixelFormat::RGB) &&
        texture.getProperties().exists("CompressionSettings") &&
        (texture.getProperties().getProperty("CompressionSettings").getValueName() == "TC_Normalmap" ||
         texture.getProperties().getProperty("CompressionSettings").getValueName() == "TC_NormalmapHQ" ||
         texture.getProperties().getProperty("CompressionSettings").getValueName() == "TC_NormalmapBC5" ||
         texture.getProperties().getProperty("CompressionSettings").getValueName() == "TC_NormalmapBC7" ||
         texture.getProperties().getProperty("CompressionSettings").getValueName() == "TC_NormalmapUncompressed"))
    {
        gamePixelFormat = PixelFormat::V8U8;
        texture.getProperties().setByteValue("Format", Image::getEngineFormatType(gamePixelFormat), "EPixelFormat");
        texture.getProperties().setByteValue("CompressionSettings", "TC_NormalmapUncompressed", "TextureCompressionSettings");
    }
    else if (gamePixelFormat == PixelFormat::DXT5 &&
        (texturePixelFormat == PixelFormat::ARGB || texturePixelFormat == PixelFormat::BC7) &&
        texture.getProperties().exists("CompressionSettings") &&
        texture.getProperties().getProperty("CompressionSettings").getValueName() == "TC_NormalmapAlpha")
    {
        gamePixelFormat = PixelFormat::ARGB;
        texture.getProperties().setByteValue("Format", Image::getEngineFormatType(gamePixelFormat), "EPixelFormat");
    }
    else if ((gamePixelFormat == PixelFormat::DXT1) &&
        (texturePixelFormat == PixelFormat::DXT5) &&
        !texture.getProperties().exists("CompressionSettings"))
    {
        gamePixelFormat = PixelFormat::DXT5;
        texture.getProperties().setByteValue("Format", Image::getEngineFormatType(gamePixelFormat), "EPixelFormat");
    }
    else if ((gamePixelFormat == PixelFormat::DXT1 || gamePixelFormat == PixelFormat::DXT5) &&
        (texturePixelFormat == PixelFormat::BC7) &&
        !texture.getProperties().exists("CompressionSettings"))
    {
        gamePixelFormat = PixelFormat::BC7;
        texture.getProperties().setByteValue("Format", Image::getEngineFormatType(gamePixelFormat), "EPixelFormat");
        texture.getProperties().setByteValue("CompressionSettings", "TC_BC7", "TextureCompressionSettings");
    }
    else if ((gamePixelFormat == PixelFormat::DXT1 || gamePixelFormat == PixelFormat::DXT5 ||
              gamePixelFormat == PixelFormat::BC7) &&
        (texturePixelFormat == PixelFormat::ARGB || texturePixelFormat == PixelFormat::RGB) &&
        (!texture.getProperties().exists("CompressionSettings") ||
        (texture.getProperties().exists("CompressionSettings") &&
         texture.getProperties().getProperty("CompressionSettings").getValueName() == "TC_BC7")))
    {
        gamePixelFormat = PixelFormat::ARGB;
        texture.getProperties().setByteValue("Format", Image::getEngineFormatType(gamePixelFormat), "EPixelFormat");
        if (texture.getProperties().exists("CompressionSettings"))
            texture.getProperties().removeProperty("CompressionSettings");
    }
    else if ((gamePixelFormat == PixelFormat::DXT1 || gamePixelFormat == PixelFormat::DXT5 ||
              gamePixelFormat == PixelFormat::BC7 || gamePixelFormat == PixelFormat::ARGB) &&
        (texturePixelFormat == PixelFormat::R10G10B10A2) &&
        (!texture.getProperties().exists("CompressionSettings") ||
        (texture.getProperties().exists("CompressionSettings") &&
         texture.getProperties().getProperty("CompressionSettings").getValueName() == "TC_BC7")))
    {
        gamePixelFormat = PixelFormat::R10G10B10A2;
        texture.getProperties().setByteValue("Format", Image::getEngineFormatType(gamePixelFormat), "EPixelFormat");
        if (texture.getProperties().exists("CompressionSettings"))
            texture.getProperties().removeProperty("CompressionSettings");
    }
    else if ((gamePixelFormat == PixelFormat::DXT1 || gamePixelFormat == PixelFormat::DXT5 ||
              gamePixelFormat == PixelFormat::BC7 || gamePixelFormat == PixelFormat::ARGB) &&
        (texturePixelFormat == PixelFormat::R16G16B16A16) &&
        (!texture.getProperties().exists("CompressionSettings") ||
        (texture.getProperties().exists("CompressionSettings") &&
         texture.getProperties().getProperty("CompressionSettings").getValueName() == "TC_BC7")))
    {
        gamePixelFormat = PixelFormat::R16G16B16A16;
        texture.getProperties().setByteValue("Format", Image::getEngineFormatType(gamePixelFormat), "EPixelFormat");
        if (texture.getProperties().exists("CompressionSettings"))
            texture.getProperties().removeProperty("CompressionSettings");
    }
    else if (((gamePixelFormat == PixelFormat::DXT1 || gamePixelFormat == PixelFormat::ARGB) &&
          texturePixelFormat == PixelFormat::RGBE) ||
         (texturePixelFormat == PixelFormat::RGBE && texture.getProperties().exists("CompressionSettings") &&
         texture.getProperties().getProperty("CompressionSettings").getValueName() == "TC_HighDynamicRange"))
    {
        gamePixelFormat = PixelFormat::RGBE;
        texture.getProperties().setByteValue("Format", Image::getEngineFormatType(PixelFormat::ARGB), "EPixelFormat");
        texture.getProperties().setByteValue("CompressionSettings", "TC_HighDynamicRange", "TextureCompressionSettings");
    }
    else
    {
        PINFO(QString("This texture will not be converted to desired pixel format.\n"));
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
                                         matchedTexture.path + "Export UIndex: " +
                                         QString::number(matchedTexture.exportID + 1) + " Skipping...");
                            ConsoleSync();
                        }
                        else
                        {
                            PERROR(QString("Error: Texture ") + foundTexture.name +
                                   " has broken export data in package: " +
                                   matchedTexture.path + "\nExport UIndex: " +
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
                                             matchedTexture.path + ", Export UIndex: " +
                                             QString::number(matchedTexture.exportID + 1));
                                ConsoleSync();
                            }
                            else
                            {
                                PERROR(QString("Error: Texture ") + foundTexture.name +
                                       " CRC does not match, mipmap: " +
                                       QString::number(m) + "\nPackage: " +
                                       matchedTexture.path + "\nExport UIndex: " +
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
                                 QStringList &pkgsToMarker,
                                 QList<ModEntry> &modsToReplace,
                                 bool appendMarker, bool verify, int cacheAmount,
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
                                 matched.path + "\nExport UIndex: " + QString::number(matched.exportID + 1) + "\nSkipping...");
                    ConsoleSync();
                }
                else
                {
                    PERROR(QString("Error: Texture ") + mod.textureName +
                           " has broken export data in package: " +
                           matched.path + "\nExport UIndex: " + QString::number(matched.exportID + 1) + "\nSkipping...\n");
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
                if (storageType == StorageTypes::extUnc)
                {
                    QString archive = textureMovie.getProperties().getProperty("TextureFileCacheName").getValueName();
                    QString archiveFile = g_GameData->MainData() + "/" + archive + ".tfc";
                    if (matched.path.contains("/DLC", Qt::CaseInsensitive))
                    {
                        mod.arcTfcDLC = true;
                        QString DLCArchiveFile = g_GameData->GamePath() + DirName(matched.path) + "/" + archive + ".tfc";
                        if (QFile(DLCArchiveFile).exists())
                            archiveFile = DLCArchiveFile;
                        else if (!QFile(archiveFile).exists())
                        {
                            QStringList files = g_GameData->tfcFiles.filter(QRegExp(QString("*/") + archive + ".tfc",
                                                                                    Qt::CaseInsensitive, QRegExp::Wildcard));
                            if (files.count() == 1)
                                archiveFile = g_GameData->GamePath() + files.first();
                            else if (files.count() == 0)
                            {
                                FileStream fs = FileStream(DLCArchiveFile, FileMode::Create, FileAccess::WriteOnly);
                                fs.WriteFromBuffer(textureMovie.getProperties().getProperty("TFCFileGuid").getValueStruct());
                                archiveFile = DLCArchiveFile;
                            }
                            else
                            {
                                QString list;
                                foreach(QString file, files)
                                    list += file + "\n";
                                CRASH_MSG((QString("More instances of TFC file: ") + archive + ".tfc\n" +
                                           list + "package: " + matched.path + "\n" +
                                           "Export UIndex: " + QString::number(matched.exportID + 1)).toStdString().c_str());
                            }
                        }
                    }

                    if (!archiveFile.contains("TexturesMEM"))
                    {
                        quint32 fileLength = QFile(archiveFile).size();
                        if (fileLength + 0x5000000UL > 0x80000000UL || !archiveFile.contains("TexturesMEM"))
                        {
                            archiveFile = "";
                            ByteBuffer guid(tfcNewGuid, 16);
                            for (qint32 indexTfc = 0; indexTfc < 9999; indexTfc++)
                            {
                                *(qint32 *)guid.ptr() = indexTfc;
                                QString tfcNewName = QString::asprintf("TexturesMEM%04d", indexTfc);
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

                mod.instance--;
                if (mod.instance < 0)
                    CRASH();
            }
            else
            {
                Texture texture = Texture(package, matched.exportID, exportData);
                exportData.Free();
                QString fmt = texture.getProperties().getProperty("Format").getValueName();
                PixelFormat pixelFormat = Image::getPixelFormatType(fmt);
                texture.removeEmptyMips();

                texture.getProperties().setIntValue("InternalFormatLODBias", -10);

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
                            PERROR(QString("Failed to decompress MEM data: ") + mod.textureName +
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

                    errors += Misc::CorrectTexture(image, texture, newPixelFormat, mod.textureName, 0.2f);

                    // remove lower mipmaps below 4x4 for DXT compressed textures
                    if (mod.cachedPixelFormat == PixelFormat::DXT1 ||
                        mod.cachedPixelFormat == PixelFormat::DXT3 ||
                        mod.cachedPixelFormat == PixelFormat::DXT5 ||
                        mod.cachedPixelFormat == PixelFormat::BC5 ||
                        mod.cachedPixelFormat == PixelFormat::BC7 ||
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
                        mod.cacheCprMipmapsStorageType = StorageTypes::extOodle;
                        mod.cacheCprMipmapsDecompressedSize.push_back(image->getMipMaps()[m]->getRefData().size());
                        auto data = Package::compressData(image->getMipMaps()[m]->getRefData(),
                                                            mod.cacheCprMipmapsStorageType);
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
                if (texture.getProperties().exists("NeverStream")) {
                    texture.getProperties().removeProperty("NeverStream");
                }

                if (pixelFormat == PixelFormat::G8)
                {
                    forceInternalMip = true;
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
                        if (texture.mipMapsList.count() == 1)
                            mipmap.storageType = StorageTypes::pccUnc;
                        else
                            mipmap.storageType = StorageTypes::extOodle;
                    }

                    mipmapsPre.push_front(mipmap);
                }

                auto mipmaps = QList<Texture::TextureMipMap>();
                for (int m = 0; m < mipmapsPre.count(); m++)
                {
                    Texture::TextureMipMap mipmap = mipmapsPre[m];
                    if ((mipmap.storageType == StorageTypes::extZlib ||
                         mipmap.storageType == StorageTypes::extOodle ||
                         mipmap.storageType == StorageTypes::extUnc ||
                         mipmap.storageType == StorageTypes::extUnc2) &&
                         mod.arcTexture.count() != 0)
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
                QString archiveFile;
                if (!texture.getProperties().exists("TextureFileCacheName"))
                {
                    FileStream fs = FileStream(g_GameData->MainData() + "/Textures.tfc", FileMode::Open, FileAccess::ReadOnly);
                    ByteBuffer guid = fs.ReadToBuffer(16);
                    texture.getProperties().setNameValue("TextureFileCacheName", "Textures");
                    texture.getProperties().setStructValue("TFCFileGuid", "Guid", guid);
                }
                QString archive = texture.getProperties().getProperty("TextureFileCacheName").getValueName();
                if (mod.arcTfcDLC && mod.arcTfcName != archive)
                {
                    mod.arcTexture.clear();
                }

                if (mod.arcTexture.count() == 0)
                {
                    archiveFile = g_GameData->MainData() + "/" + archive + ".tfc";
                    if (g_GameData->gameType == MeType::ME1_TYPE || matched.path.contains("/DLC", Qt::CaseInsensitive))
                    {
                        mod.arcTfcDLC = true;
                        QString DLCArchiveFile = g_GameData->GamePath() + DirName(matched.path) + "/" + archive + ".tfc";
                        if (QFile(DLCArchiveFile).exists())
                            archiveFile = DLCArchiveFile;
                        else if (!QFile(archiveFile).exists())
                        {
                            QStringList files = g_GameData->tfcFiles.filter(QRegExp(QString("*/") + archive + ".tfc",
                                                                                    Qt::CaseInsensitive, QRegExp::Wildcard));
                            if (files.count() == 1)
                                archiveFile = g_GameData->GamePath() + files.first();
                            else if (files.count() == 0)
                            {
                                FileStream fs = FileStream(DLCArchiveFile, FileMode::Create, FileAccess::WriteOnly);
                                fs.WriteFromBuffer(texture.getProperties().getProperty("TFCFileGuid").getValueStruct());
                                archiveFile = DLCArchiveFile;
                            }
                            else
                            {
                                QString list;
                                foreach(QString file, files)
                                    list += file + "\n";
                                CRASH_MSG((QString("More instances of TFC file: ") + archive + ".tfc\n" +
                                           list + "package: " + matched.path + "\n" +
                                           "Export UIndex: " + QString::number(matched.exportID + 1)).toStdString().c_str());
                            }
                        }
                    }
                    else
                    {
                        mod.arcTfcDLC = false;
                    }

                    quint32 fileLength = QFile(archiveFile).size();
                    if ((fileLength + 0x5000000UL > 0x80000000UL) || !archiveFile.contains("TexturesMEM"))
                    {
                        archiveFile = "";
                        ByteBuffer guid(tfcNewGuid, 16);
                        for (qint32 indexTfc = 0; indexTfc < 9999; indexTfc++)
                        {
                            *(qint32 *)guid.ptr() = indexTfc;
                            QString tfcNewName = QString::asprintf("TexturesMEM%04d", indexTfc);
                            archiveFile = g_GameData->MainData() + "/" + tfcNewName + ".tfc";
                            if (!QFile(archiveFile).exists())
                            {
                                texture.getProperties().setNameValue("TextureFileCacheName", tfcNewName);
                                texture.getProperties().setStructValue("TFCFileGuid", "Guid", guid);
                                FileStream fs = FileStream(archiveFile, FileMode::Create, FileAccess::WriteOnly);
                                fs.WriteFromBuffer(guid);
                                guid.Free();
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

                for (int m = 0; m < mipmaps.count(); m++)
                {
                    Texture::TextureMipMap mipmap = mipmaps[m];
                    mipmap.uncompressedSize = mod.cacheCprMipmapsDecompressedSize[m];
                    if (mipmap.storageType == StorageTypes::extZlib ||
                        mipmap.storageType == StorageTypes::extOodle ||
                        mipmap.storageType == StorageTypes::pccZlib ||
                        mipmap.storageType == StorageTypes::pccOodle)
                    {
                        mipmap.newData = mod.cacheCprMipmaps[m].getRefData();
                        mipmap.compressedSize = mipmap.newData.size();
                    }
                    else if (mipmap.storageType == StorageTypes::pccUnc ||
                             mipmap.storageType == StorageTypes::extUnc ||
                             mipmap.storageType == StorageTypes::extUnc2)
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
                        mipmap.storageType == StorageTypes::extOodle ||
                        mipmap.storageType == StorageTypes::extUnc ||
                        mipmap.storageType == StorageTypes::extUnc2)
                    {
                        if (mod.arcTexture.count() == 0)
                        {
                            triggerCacheArc = true;
                            FileStream fs = FileStream(archiveFile, FileMode::Open, FileAccess::ReadWrite);
                            fs.SeekEnd();
                            mipmap.dataOffset = (uint)fs.Position();
                            fs.WriteFromBuffer(mipmap.newData);
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
                    if (mipmaps[m].freeNewData)
                        mipmaps[m].newData.Free();
                    mipmaps.replace(m, mipmap);
                    if (texture.mipMapsList.count() == 1)
                        break;
                }

                texture.replaceMipMaps(mipmaps);

                texture.getProperties().setIntValue("SizeX", texture.mipMapsList.first().width);
                texture.getProperties().setIntValue("SizeY", texture.mipMapsList.first().height);
                texture.getProperties().setIntValue("OriginalSizeX", texture.mipMapsList.first().width);
                texture.getProperties().setIntValue("OriginalSizeY", texture.mipMapsList.first().height);
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

                if (triggerCacheArc)
                {
                    mod.CopyMipMapsList(mod.arcTexture, texture.mipMapsList);
                    memcpy(mod.arcTfcGuid, texture.getProperties().getProperty("TFCFileGuid").getValueStruct().ptr(), 16);
                    mod.arcTfcName = texture.getProperties().getProperty("TextureFileCacheName").getValueName();
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

        if (package.SaveToFile(false, false, appendMarker))
        {
            if (appendMarker)
                pkgsToMarker.removeOne(package.packagePath);
        }
    }

    for (int e = 0; e < modsToReplace.count(); e++)
    {
        if (modsToReplace[e].instance > 0)
        {
            PINFO(QString("Mod index: ") + QString::number(e) + ", instances: " + QString::number(modsToReplace[e].instance));
        }
    }

    return errors;
}

namespace {

int comparePaths(const MapTexturesToMod &e1, const MapTexturesToMod &e2)
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

} // namespace

QString MipMaps::replaceModsFromList(QList<TextureMapEntry> &textures, QStringList &pkgsToMarker,
                                     QList<ModEntry> &modsToReplace,
                                     bool appendMarker, bool verify,
                                     int cacheAmount, ProgressCallback callback, void *callbackHandle)
{
    QString errors;

    // Remove duplicates
    for (int i = 0; i < modsToReplace.count(); i++)
    {
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
            if ((mod.textureCrc != 0 && mod.textureCrc == modsToReplace[l].textureCrc))
            {
                if (g_ipc)
                {
                    ConsoleWrite(QString("[IPC]MOD_OVERRIDE ") + mod.textureName +
                                 QString::asprintf("_0x%08X", mod.textureCrc) + ", " + mod.memPath);
                    ConsoleSync();
                }
                else
                {
                    PINFO(QString("Override texture: ") + mod.textureName +
                          QString::asprintf("_0x%08X", mod.textureCrc) + ", " + mod.memPath + "\n");
                }
                modsToReplace.removeAt(l);
                i--;
                break;
            }
        }
    }

    QList<MapTexturesToMod> map = QList<MapTexturesToMod>();

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
            previousPath = map[i].packagePath.toLower();
            mapPackages.push_back(mapEntry);
            packagesIndex++;
        }
    }
    map.clear();

    if (mapPackages.count() != 0)
    {
        if (!g_ipc)
        {
            PINFO("\nInstalling texture mods...\n");
        }

        errors += replaceTextures(mapPackages, textures, pkgsToMarker, modsToReplace,
                                  appendMarker, verify, cacheAmount,
                                  callback, callbackHandle);
    }

    modsToReplace.clear();

    return errors;
}
