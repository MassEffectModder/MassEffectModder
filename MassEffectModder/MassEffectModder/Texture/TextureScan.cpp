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

#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <Wrappers.h>
#include <Texture/TextureScan.h>
#include <Texture/Texture.h>
#include <Texture/TextureMovie.h>
#include <Texture/TextureCube.h>
#include <GameData/Package.h>
#include <GameData/GameData.h>
#include <GameData/TOCFile.h>
#include <Program/ConfigIni.h>
#include <Image/Image.h>
#include <Resources/Resources.h>
#include <Types/MemTypes.h>
#include <MipMaps/MipMaps.h>
#include <Misc/Misc.h>

namespace {

bool generateBuiltinMapFiles = false; // change to true to enable map files generation

} // namespace

bool TreeScan::IsBlankTexture(uint crc)
{
    const uint crcTable[] = {
        0x0A5F0BEA,
        0x0E174561,
        0x104A50D1,
        0x1DF115DD,
        0x22BCD339,
        0x270B666B,
        0x28687114,
        0x35013CC0,
        0x38E3FFEE,
        0x3D570562,
        0x450B9A51,
        0x4994D034,
        0x4D14CF12,
        0x4D558A87,
        0x53D3A11E,
        0x54AB2D79,
        0x58C715E3,
        0x5B835EB5,
        0x64CD1504,
        0x72767881,
        0x7400F70D,
        0x75DA7513,
        0x77452EB8,
        0x7BAC27D6,
        0x81173232,
        0x868DB039,
        0x8A729CC9,
        0x8A99F553,
        0x91412D11,
        0x93E31E81,
        0x9819367B,
        0xA6BAFBBA,
        0xB42D608E,
        0xC7C0EFB3,
        0xDB8EC88D,
        0xE52D43BA,
        0xE6F5AA52,
        0xE92ECBF8,
        0xEEB8BF95,
        0xF2697AA7,
        0xFE04D332,
        0xFEE00359,
    };

    for (unsigned int crcValue : crcTable)
    {
        if (crc == crcValue)
            return true;
    }
    return false;
}

void TreeScan::loadTexturesMap(MeType gameId, Resources &resources, QList<TextureMapEntry> &textures)
{
    QStringList pkgs;
    if (gameId == MeType::ME1_TYPE)
        pkgs = resources.tablePkgsME1;
    else if (gameId == MeType::ME2_TYPE)
        pkgs = resources.tablePkgsME2;
    else
        pkgs = resources.tablePkgsME3;

    FileStream tmp = FileStream(QString(":/mele%1map.bin").arg((int)gameId), FileMode::Open, FileAccess::ReadOnly);
    if (tmp.ReadUInt32() != 0x504D5443)
        CRASH();
    ByteBuffer decompressed(tmp.ReadInt32());
    ByteBuffer compressed = tmp.ReadToBuffer(tmp.ReadUInt32());
    uint dstLen = decompressed.size();
    LzmaDecompress(compressed.ptr(), compressed.size(), decompressed.ptr(), &dstLen);
    compressed.Free();
    if (decompressed.size() != dstLen)
        CRASH();

    auto fs = MemoryStream(decompressed);
    decompressed.Free();
    fs.Skip(8);
    uint countTexture = fs.ReadUInt32();
    for (uint i = 0; i < countTexture; i++)
    {
        TextureMapEntry texture{};
        int len = fs.ReadByte();
        fs.ReadStringASCII(texture.name, len);
        texture.crc = fs.ReadUInt32();
        texture.width = fs.ReadInt16();
        texture.height = fs.ReadInt16();
        texture.pixfmt = (PixelFormat)fs.ReadByte();
        texture.type = (TextureType)fs.ReadByte();
        int countPackages = fs.ReadInt16();
        texture.list = QList<TextureMapPackageEntry>();
        for (int k = 0; k < countPackages; k++)
        {
            TextureMapPackageEntry matched{};
            matched.exportID = fs.ReadInt32();
            quint8 flags = fs.ReadByte();
            matched.hasAlphaData = (flags & 1) == 1;
            matched.numMips = fs.ReadByte();
            matched.movieTexture = (texture.type == TextureType::Movie);
            matched.path = pkgs[fs.ReadInt16()];
            matched.path.replace(QChar('\\'), QChar('/'));
            texture.list.push_back(matched);
        }
        textures.push_back(texture);
    }
}

bool TreeScan::loadTexturesMapFile(QString &path, QList<TextureMapEntry> &textures, bool ignoreCheck)
{
    if (!QFile(path).exists())
    {
        if (g_ipc)
        {
            ConsoleWrite("[IPC]ERROR_TEXTURE_MAP_MISSING");
            ConsoleSync();
        }
        else
        {
            PERROR("No existing texture map scan file was found!\n");
        }
        return false;
    }

    bool foundRemoved = false;
    bool foundAdded = false;

    FileStream fs = FileStream(path, FileMode::Open, FileAccess::ReadOnly);
    uint tag = fs.ReadUInt32();
    uint version = fs.ReadUInt32();
    if (tag != textureMapBinTag || version > textureMapBinVersion)
    {
        if (g_ipc)
        {
            ConsoleWrite("[IPC]ERROR_TEXTURE_MAP_WRONG");
            ConsoleSync();
        }
        else
        {
            PERROR("Detected corrupt or old version of texture map scan file!\n");
        }
        return false;
    }

    QStringList packages = QStringList();

    if (version == 1)
        loadTexturesMapFileV1(fs, textures, packages);
    else
        CRASH();

    if (!ignoreCheck)
    {
        for (int i = 0; i < packages.count(); i++)
        {
            bool found = false;
            for (int s = 0; s < g_GameData->packageFiles.count(); s++)
            {
                if (AsciiStringMatchCaseIgnore(g_GameData->packageFiles[s], packages[i]))
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                if (g_ipc)
                {
                    ConsoleWrite(QString("[IPC]ERROR_REMOVED_FILE ") + packages[i]);
                    ConsoleSync();
                }
                else
                {
                    PERROR(QString("Files were removed since the texture scan was conducted: ") + packages[i] + "\n");
                }
                foundRemoved = true;
            }
        }
        if (!g_ipc && foundRemoved)
            PERROR("The above files were removed since last game data scan.\n");

        for (int i = 0; i < g_GameData->packageFiles.count(); i++)
        {
            bool found = false;
            for (int s = 0; s < packages.count(); s++)
            {
                if (AsciiStringMatchCaseIgnore(packages[s], g_GameData->packageFiles[i]))
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                if (g_ipc)
                {
                    ConsoleWrite(QString("[IPC]ERROR_ADDED_FILE ") + g_GameData->packageFiles[i]);
                    ConsoleSync();
                }
                else
                {
                    PERROR(QString("File: ") + g_GameData->packageFiles[i] + "\n");
                }
                foundAdded = true;
            }
        }
        if (!g_ipc && foundAdded)
            PERROR("The above files added since last game data scan.\n");
    }

    return !foundRemoved && !foundAdded;
}

void TreeScan::loadTexturesMapFileV1(Stream &streeam, QList<TextureMapEntry> &textures, QStringList &packages)
{
    uint countTexture = streeam.ReadUInt32();
    for (uint i = 0; i < countTexture; i++)
    {
        TextureMapEntry texture{};
        int len = streeam.ReadInt32();
        streeam.ReadStringASCII(texture.name, len);
        texture.crc = streeam.ReadUInt32();
        uint countPackages = streeam.ReadUInt32();
        texture.list = QList<TextureMapPackageEntry>();
        for (uint k = 0; k < countPackages; k++)
        {
            TextureMapPackageEntry matched{};
            matched.exportID = streeam.ReadInt32();
            quint32 flags = streeam.ReadUInt32();
            matched.movieTexture = (flags & 1) == 1;
            matched.hasAlphaData = (flags & 2) == 2;
            len = streeam.ReadInt32();
            streeam.ReadStringASCII(matched.path, len);
            matched.path.replace(QChar('\\'), QChar('/'));
            texture.list.push_back(matched);
        }
        textures.push_back(texture);
    }

    int numPackages = streeam.ReadInt32();
    for (int i = 0; i < numPackages; i++)
    {
        int len = streeam.ReadInt32();
        QString pkgPath;
        streeam.ReadStringASCII(pkgPath, len);
        pkgPath.replace(QChar('\\'), QChar('/'));
        packages.push_back(pkgPath);
    }
}

bool TreeScan::PrepareListOfTextures(MeType gameId, Resources &resources,
                                    QList<TextureMapEntry> &textures,
                                    bool saveMapFile,
                                    ProgressCallback callback, void *callbackHandle)
{
    QStringList pkgs;
    QList<MD5FileEntry> md5Entries;
    if (gameId == MeType::ME1_TYPE)
    {
        pkgs = resources.tablePkgsME1;
        md5Entries = resources.entriesME1;
    }
    else if (gameId == MeType::ME2_TYPE)
    {
        pkgs = resources.tablePkgsME2;
        md5Entries = resources.entriesME2;
    }
    else
    {
        pkgs = resources.tablePkgsME3;
        md5Entries = resources.entriesME3;
    }

    QString path = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation).first() +
            "/MassEffectModder";
    if (!QDir(path).exists())
        QDir(path).mkpath(path);
    QString filename = path + QString("/mele%1map.bin").arg((int)gameId);

#ifdef GUI
    QElapsedTimer timer;
    timer.start();
#endif
    Misc::restartStageTimer();
    if (!generateBuiltinMapFiles)
    {
        if (g_ipc)
        {
            ConsoleWrite("[IPC]STAGE_CONTEXT STAGE_PRESCAN");
            ConsoleSync();
        }

        loadTexturesMap(gameId, resources, textures);

        for (int k = 0; k < textures.count(); k++)
        {
#ifdef GUI
            if (timer.elapsed() > 100)
            {
                QApplication::processEvents();
                timer.restart();
            }
#endif
            bool found = false;
            for (int t = 0; t < textures[k].list.count(); t++)
            {
                if (std::binary_search(g_GameData->packageFiles.begin(),
                                       g_GameData->packageFiles.end(),
                                       textures[k].list[t].path, comparePath))
                {
                    found = true;
                    continue;
                }
                TextureMapPackageEntry f = textures[k].list[t];
                f.path = "";
                textures[k].list[t] = f;
            }
            if (!found)
            {
                textures[k].list.clear();
                textures.removeAt(k);
                k--;
            }
        }
    }

    int count = g_GameData->packageFiles.count();
    for (int i = 0; i < count; i++)
    {
#ifdef GUI
        if (timer.elapsed() > 100)
        {
            QApplication::processEvents();
            timer.restart();
        }
#endif
        QString str = g_GameData->packageFiles[i];
        if (str.contains("_IT.") ||
            str.contains("_FR.") ||
            str.contains("_ES.") ||
            str.contains("_DE.") ||
            str.contains("_RA.") ||
            str.contains("_RU.") ||
            str.contains("_PLPC.") ||
            str.contains("_BRA.") ||
            str.contains("_DEU.") ||
            str.contains("_FRA.") ||
            str.contains("_ITA.") ||
            str.contains("_POL."))
        {
            g_GameData->packageFiles.push_back(str);
            g_GameData->packageFiles.removeAt(i--);
            count--;
        }
    }
    long elapsed = Misc::elapsedStageTime();
    if (g_ipc)
    {
        ConsoleWrite(QString("[IPC]STAGE_TIMING %1").arg(elapsed));
        ConsoleSync();
    }

    Misc::restartStageTimer();
    if (g_ipc)
    {
        ConsoleWrite("[IPC]STAGE_CONTEXT STAGE_SCAN");
        ConsoleSync();
    }

    if (!generateBuiltinMapFiles)
    {
        QStringList addedFiles;
        QStringList modifiedFiles;

        for (int i = 0; i < g_GameData->packageFiles.count(); i++)
        {
#ifdef GUI
            if (timer.elapsed() > 100)
            {
                QApplication::processEvents();
                timer.restart();
            }
#endif
            bool modified = true;
            bool foundPkg = false;
            QString package = g_GameData->packageFiles[i].toLower();
            long packageSize = QFile(g_GameData->GamePath() + g_GameData->packageFiles[i]).size();
            auto range = std::equal_range(md5Entries.begin(), md5Entries.end(),
                                          package, Resources::ComparePath());
            for (auto it = range.first; it != range.second; it++)
            {
                if (it->path.compare(package, Qt::CaseSensitive) != 0)
                    break;
                foundPkg = true;
                if (packageSize == it->size)
                {
                    modified = false;
                    break;
                }
            }
            if (foundPkg && modified)
                modifiedFiles.push_back(g_GameData->packageFiles[i]);
            else if (!foundPkg)
                addedFiles.push_back(g_GameData->packageFiles[i]);
        }

        int lastProgress = -1;
        int totalPackages = modifiedFiles.count() + addedFiles.count();
        int currentPackage = 0;
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]STAGE_WEIGHT STAGE_SCAN ") +
                QString::number(((float)totalPackages / g_GameData->packageFiles.count())));
            ConsoleSync();
        }
        for (int i = 0; i < modifiedFiles.count(); i++, currentPackage++)
        {
#ifdef GUI
            if (timer.elapsed() > 100)
            {
                QApplication::processEvents();
                timer.restart();
            }
#endif
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + modifiedFiles[i]);
                ConsoleSync();
            }
            else
            {
                PINFO(QString("Package ") + QString::number(currentPackage + 1) + "/" +
                                     QString::number(totalPackages) + " : " +
                                     modifiedFiles[i] + "\n");
            }

            int newProgress = currentPackage * 100 / totalPackages;
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
                    callback(callbackHandle, newProgress, "Scanning textures");
                }
            }
			if (!FindTextures(textures, modifiedFiles[i], true))
				return false;
        }

        for (int i = 0; i < addedFiles.count(); i++, currentPackage++)
        {
#ifdef GUI
            if (timer.elapsed() > 100)
            {
                QApplication::processEvents();
                timer.restart();
            }
#endif
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + addedFiles[i]);
                ConsoleSync();
            }
            else
            {
                PINFO(QString("Package ") + QString::number(currentPackage + 1) + "/" +
                                     QString::number(totalPackages) + " : " +
                                     addedFiles[i] + "\n");
            }

            int newProgress = currentPackage * 100 / totalPackages;
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
                    callback(callbackHandle, newProgress, "Scanning textures");
                }
            }
			if (!FindTextures(textures, addedFiles[i], false))
				return false;
		}
    }
    else
    {
        int lastProgress = -1;
        for (int i = 0; i < g_GameData->packageFiles.count(); i++)
        {
#ifdef GUI
            if (timer.elapsed() > 100)
            {
                QApplication::processEvents();
                timer.restart();
            }
#endif
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + g_GameData->packageFiles[i]);
                ConsoleSync();
            }
            else
            {
                PINFO(QString("Package ") + QString::number(i + 1) + "/" +
                                     QString::number(g_GameData->packageFiles.count()) + " : " +
                                     g_GameData->packageFiles[i] + "\n");
            }

            int newProgress = i * 100 / g_GameData->packageFiles.count();
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
                    callback(callbackHandle, newProgress, "Scanning textures");
                }
            }
			if (!FindTextures(textures, g_GameData->packageFiles[i], false))
				return false;
		}
    }

    if (callback)
        callback(callbackHandle, 100, "Scanning textures");

    if (saveMapFile)
    {
        if (QFile(filename).exists())
            QFile(filename).remove();

        auto fs = FileStream(filename, FileMode::Create, FileAccess::WriteOnly);
        MemoryStream mem;
        mem.WriteUInt32(textureMapBinTag);
        mem.WriteUInt32(textureMapBinVersion);
        mem.WriteInt32(textures.count());

        for (int i = 0; i < textures.count(); i++)
        {
            const TextureMapEntry& texture = textures[i];
            if (generateBuiltinMapFiles)
                mem.WriteByte(texture.name.length());
            else
                mem.WriteInt32(texture.name.length());
            mem.WriteStringASCII(texture.name);
            mem.WriteUInt32(texture.crc);
            if (generateBuiltinMapFiles)
            {
                mem.WriteInt16(texture.width);
                mem.WriteInt16(texture.height);
                mem.WriteByte(texture.pixfmt);
                mem.WriteByte(texture.type);

                mem.WriteInt16(texture.list.count());
            }
            else
            {
                mem.WriteInt32(texture.list.count());
            }
            for (int k = 0; k < texture.list.count(); k++)
            {
                const TextureMapPackageEntry& m = texture.list[k];
                mem.WriteInt32(m.exportID);
                if (generateBuiltinMapFiles)
                {
                    quint32 flags = m.hasAlphaData ? 1 : 0;
                    mem.WriteByte(flags);
                    mem.WriteByte(m.numMips);
                    mem.WriteInt16(pkgs.indexOf(m.path));
                }
                else
                {
                    quint32 flags = m.movieTexture ? 1 : 0;
                    flags |= m.hasAlphaData ? 2 : 0;
                    mem.WriteUInt32(flags);
                    mem.WriteInt32(m.path.length());
                    QString path = m.path;
                    mem.WriteStringASCII(path.replace(QChar('/'), QChar('\\'), Qt::CaseInsensitive));
                }
            }
        }
        if (!generateBuiltinMapFiles)
        {
            mem.WriteInt32(g_GameData->packageFiles.count());
            for (int i = 0; i < g_GameData->packageFiles.count(); i++)
            {
                mem.WriteInt32(g_GameData->packageFiles[i].length());
                mem.WriteStringASCII(g_GameData->packageFiles[i].replace(QChar('/'), QChar('\\')));
            }
        }
        mem.SeekBegin();

        if (generateBuiltinMapFiles)
        {
            fs.WriteUInt32(0x504D5443);
            fs.WriteUInt32(mem.Length());
            quint8 *compressed = nullptr;
            uint compressedSize = 0;
            ByteBuffer decompressed = mem.ToArray();
            LzmaCompress(decompressed.ptr(), decompressed.size(), &compressed, &compressedSize, 9);
            decompressed.Free();
            fs.WriteUInt32(compressedSize);
            fs.WriteFromBuffer(compressed, compressedSize);
            delete[] compressed;
        }
        else
        {
            fs.CopyFrom(mem, mem.Length());
        }
    }

    elapsed = Misc::elapsedStageTime();
    if (g_ipc)
    {
        ConsoleWrite(QString("[IPC]STAGE_TIMING %1").arg(elapsed));
        ConsoleSync();
    }

    return true;
}

bool TreeScan::FindTextures(QList<TextureMapEntry> &textures, const QString &packagePath, bool modified)
{
    Package package;
    int status = package.Open(g_GameData->GamePath() + packagePath);
    if (status != 0)
    {
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]ERROR Issue opening package file: ") + packagePath);
            ConsoleSync();
        }
        else
        {
            PERROR(QString("ERROR: Issue opening package file: ") + packagePath + "\n");
        }
		return false;
    }

    for (int i = 0; i < package.exportsTable.count(); i++)
    {
        Package::ExportEntry& exp = package.exportsTable[i];
        int id = package.getClassNameId(exp.getClassId());
        if (id == package.nameIdTexture2D ||
            id == package.nameIdLightMapTexture2D ||
            id == package.nameIdShadowMapTexture2D ||
            id == package.nameIdTextureFlipBook ||
            id == package.nameIdTextureMovie ||
            id == package.nameIdTextureCube)
        {
            ByteBuffer exportData = package.getExportData(i);
            if (exportData.ptr() == nullptr)
            {
                if (g_ipc)
                {
                    ConsoleWrite(QString("[IPC]ERROR Texture ") + exp.objectName +
                                 " has broken export data in package: " +
                                 packagePath + "\nExport UIndex: " + QString::number(i + 1) + "\nSkipping...");
                    ConsoleSync();
                }
                else
                {
                    PERROR(QString("Error: Texture ") + exp.objectName +
                                 " has broken export data in package: " +
                                 packagePath +"\nExport UIndex: " + QString::number(i + 1) + "\nSkipping...\n");
                }
				return false;
			}

            TextureMovie *textureMovie = nullptr;
            TextureCube *textureCube = nullptr;
            Texture *texture = nullptr;
            uint crc;

            TextureMapPackageEntry matchTexture{};
            matchTexture.exportID = i;
            matchTexture.path = packagePath;
            matchTexture.hasAlphaData = false;

            if (id == package.nameIdTextureMovie)
            {
                textureMovie = new TextureMovie(package, i, exportData);
                exportData.Free();
                if (!textureMovie->hasTextureData())
                {
                    delete textureMovie;
                    continue;
                }
                matchTexture.movieTexture = true;
                crc = textureMovie->getCrcData();
            }
            else if (id == package.nameIdTextureCube)
            {
                textureCube = new TextureCube(package, i, exportData);
                exportData.Free();
                delete textureCube;
                continue;
            }
            else
            {
                texture = new Texture(package, i, exportData);
                exportData.Free();
                if (!texture->hasImageData())
                {
                    delete texture;
                    continue;
                }

                matchTexture.numMips = texture->numNotEmptyMips();
                crc = texture->getCrcTopMipmap();
            }

            if (crc == 0)
            {
                if (g_ipc)
                {
                    ConsoleWrite(QString("[IPC]ERROR Texture ") + exp.objectName + " is broken in package: " +
                                 packagePath + "\nExport UIndex: " + QString::number(i + 1) + "\nSkipping...");
                    ConsoleSync();
                }
                else
                {
                    PERROR(QString("Error: Texture ") + exp.objectName + " is broken in package: " +
                                 packagePath +"\nExport UIndex: " + QString::number(i + 1) + "\nSkipping...\n");
                }
                delete textureMovie;
                delete texture;
				return false;
			}

            int foundTextureIndex = -1;
            for (int k = 0; k < textures.count(); k++)
            {
                if (textures[k].crc == crc)
                {
                    foundTextureIndex = k;
                    break;
                }
            }
            QString packagePathLower = packagePath.toLower();
            if (foundTextureIndex != -1)
            {
                const TextureMapEntry& foundTexName = textures[foundTextureIndex];
                if (modified)
                {
                    bool found = false;
                    for (int s = 0; s < foundTexName.list.count(); s++)
                    {
                        if (foundTexName.list[s].exportID == i &&
                            AsciiStringMatchCaseIgnore(foundTexName.list[s].path, packagePathLower))
                        {
                            found = true;
                            break;
                        }
                    }
                    if (found)
                    {
                        delete textureMovie;
                        delete texture;
                        continue;
                    }
                }
                textures[foundTextureIndex].list.push_back(matchTexture);
            }
            else
            {
                if (modified)
                {
                    for (int k = 0; k < textures.count(); k++)
                    {
                        bool found = false;
                        for (int t = 0; t < textures[k].list.count(); t++)
                        {
                            if (textures[k].list[t].exportID == i &&
                                AsciiStringMatchCaseIgnore(textures[k].list[t].path, packagePathLower))
                            {
                                TextureMapPackageEntry f = textures[k].list[t];
                                f.path = "";
                                textures[k].list[t] = f;
                                found = true;
                                break;
                            }
                        }
                        if (found)
                            break;
                    }
                }
                TextureMapEntry foundTex;
                foundTex.name = exp.objectName;
                foundTex.crc = crc;

                if (id == package.nameIdTextureMovie)
                {
                    if (generateBuiltinMapFiles)
                    {
                        foundTex.width = textureMovie->getProperties().getProperty("SizeX").getValueInt();
                        foundTex.height = textureMovie->getProperties().getProperty("SizeY").getValueInt();
                        foundTex.pixfmt = Image::getPixelFormatType(textureMovie->getProperties().getProperty("Format").getValueName());
                        foundTex.type = TextureType::Movie;
                    }
                }
                else
                {
                    foundTex.width = texture->getTopMipmap().width;
                    foundTex.height = texture->getTopMipmap().height;
                    foundTex.pixfmt = Image::getPixelFormatType(texture->getProperties().getProperty("Format").getValueName());
                    if (texture->getProperties().exists("CompressionSettings"))
                    {
                        QString cmp = texture->getProperties().getProperty("CompressionSettings").getValueName();
                        if (cmp == "TC_Default")
                        {
                            //Some textures may have this set, treat same as if no compression setting
                            foundTex.type = TextureType::Diffuse;
                        }
                        else if (cmp == "TC_OneBitAlpha")
                        {
                            foundTex.type = TextureType::OneBitAlpha;
                            matchTexture.hasAlphaData = true;
                        }
                        else if (cmp == "TC_Displacementmap")
                            foundTex.type = TextureType::Displacementmap;
                        else if (cmp == "TC_Grayscale")
                            foundTex.type = TextureType::GreyScale;
                        else if (cmp == "TC_Normalmap" ||
                            cmp == "TC_NormalmapHQ" ||
                            cmp == "TC_NormalmapAlpha" ||
                            cmp == "TC_NormalmapBC5" ||
                            cmp == "TC_NormalmapBC7" ||
                            cmp == "TC_NormalmapUncompressed")
                        {
                            foundTex.type = TextureType::Normalmap;
                            if (cmp == "TC_NormalmapAlpha")
                                matchTexture.hasAlphaData = true;
                        }
                        else if (cmp == "TC_BC7" ||
                                 cmp == "TC_HighDynamicRange")
                        {
                            foundTex.type = TextureType::Diffuse;
                        }
                        else
                        {
                            CRASH_MSG(QString("Unknown texture compression type on %1: %2").arg(foundTex.name, cmp).toStdString().c_str());
                        }
                    }
                    else
                    {
                        foundTex.type = TextureType::Diffuse;
                    }

                    if (foundTex.type == TextureType::Diffuse)
                    {
                        if (foundTex.pixfmt == PixelFormat::DXT5 ||
                            foundTex.pixfmt == PixelFormat::BC7 ||
                            foundTex.pixfmt == PixelFormat::ARGB ||
                            foundTex.pixfmt == PixelFormat::R10G10B10A2 ||
                            foundTex.pixfmt == PixelFormat::R16G16B16A16)
                        {
                            ByteBuffer data = texture->getTopImageData();
                            auto pixels = Image::convertRawToInternal(data, foundTex.width, foundTex.height, foundTex.pixfmt);
                            matchTexture.hasAlphaData = Image::InternalDetectAlphaData(pixels, foundTex.width, foundTex.height);
                            data.Free();
                            pixels.Free();
                        }
                    }
                }
                foundTex.list.push_back(matchTexture);
                textures.push_back(foundTex);
            }
            delete textureMovie;
            delete texture;
        }
    }

	return true;
}
