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
#include <Wrappers.h>
#include <Texture/TextureScan.h>
#include <Texture/Texture.h>
#include <Texture/TextureMovie.h>
#include <GameData/Package.h>
#include <GameData/GameData.h>
#include <GameData/TOCFile.h>
#include <Program/ConfigIni.h>
#include <Image/Image.h>
#include <Resources/Resources.h>
#include <Types/MemTypes.h>
#include <MipMaps/MipMaps.h>
#include <Misc/Misc.h>

static bool generateBuiltinMapFiles = false; // change to true to enable map files generation

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

void TreeScan::WarnNorm4k(MeType gameId, Texture *texture, const QString &textureName, const QString &packagePath,
                          int exportId, QList<Texture4kNormEntry> &texture4kNorms)
{
    if (!g_ipc || !texture || gameId != MeType::ME3_TYPE)
        return;
    if (texture->getTopMipmap().width < 4096 && texture->getTopMipmap().height < 4096)
        return;
    if (!texture->getProperties().exists("CompressionSettings"))
        return;

    QString cmp = texture->getProperties().getProperty("CompressionSettings").valueName;
    if (cmp == "TC_Normalmap" ||
        cmp == "TC_NormalmapHQ" ||
        cmp == "TC_NormalmapAlpha" ||
        cmp == "TC_NormalmapUncompressed")
    {
        ConsoleWrite(QString("[IPC]WARN_4K_NORM_FOUND ") + QString::number(exportId + 1) + " " +
                     textureName + " " + packagePath);
        ConsoleSync();
        Texture4kNormEntry entry;
        entry.path = packagePath;
        entry.exportId = exportId;
        entry.textureName = textureName;
        texture4kNorms.push_back(entry);
    }
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

    FileStream tmp = FileStream(QString(":/me%1map.bin").arg((int)gameId), FileMode::Open, FileAccess::ReadOnly);
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
        texture.flags = (TextureProperty::TextureTypes)fs.ReadByte();
        int countPackages = fs.ReadInt16();
        texture.list = QList<TextureMapPackageEntry>();
        for (int k = 0; k < countPackages; k++)
        {
            TextureMapPackageEntry matched{};
            matched.exportID = fs.ReadInt32();
            if (gameId == MeType::ME1_TYPE)
            {
                matched.linkToMaster = fs.ReadInt16();
                if (matched.linkToMaster != -1)
                {
                    matched.slave = true;
                    fs.ReadStringASCIINull(matched.basePackageName);
                }
                matched.linkToMaster = -1;
                matched.mipmapOffset = fs.ReadUInt32();
            }
            matched.removeEmptyMips = fs.ReadByte() != 0;
            matched.numMips = fs.ReadByte();
            matched.movieTexture = (texture.flags == TextureProperty::TextureTypes::Movie);
            matched.path = pkgs[fs.ReadInt16()];
            matched.path.replace(QChar('\\'), QChar('/'));
            matched.packageName = BaseNameWithoutExt(matched.path).toLower();
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
            PERROR("Missing textures scan file!\n");
        }
        return false;
    }

    bool foundRemoved = false;
    bool foundAdded = false;

    FileStream fs = FileStream(path, FileMode::Open, FileAccess::ReadOnly);
    uint tag = fs.ReadUInt32();
    uint version = fs.ReadUInt32();
    if (tag != textureMapBinTag || version != textureMapBinVersion)
    {
        if (g_ipc)
        {
            ConsoleWrite("[IPC]ERROR_TEXTURE_MAP_WRONG");
            ConsoleSync();
        }
        else
        {
            PERROR("Detected wrong or old version of textures scan file!\n");
        }
        return false;
    }

    uint countTexture = fs.ReadUInt32();
    for (uint i = 0; i < countTexture; i++)
    {
        TextureMapEntry texture{};
        int len = fs.ReadInt32();
        fs.ReadStringASCII(texture.name, len);
        texture.crc = fs.ReadUInt32();
        uint countPackages = fs.ReadUInt32();
        texture.list = QList<TextureMapPackageEntry>();
        for (uint k = 0; k < countPackages; k++)
        {
            TextureMapPackageEntry matched{};
            matched.exportID = fs.ReadInt32();
            matched.linkToMaster = fs.ReadInt32();
            if (matched.linkToMaster == -2)
            {
                matched.linkToMaster = -1;
                matched.movieTexture = true;
            }
            len = fs.ReadInt32();
            fs.ReadStringASCII(matched.path, len);
            matched.path.replace(QChar('\\'), QChar('/'));
            texture.list.push_back(matched);
        }
        // removing blank textures from the textures map
        if (texture.crc == 0xFEE00359)
        {
            continue;
        }
        textures.push_back(texture);
    }

    QStringList packages = QStringList();
    int numPackages = fs.ReadInt32();
    for (int i = 0; i < numPackages; i++)
    {
        int len = fs.ReadInt32();
        QString pkgPath;
        fs.ReadStringASCII(pkgPath, len);
        pkgPath.replace(QChar('\\'), QChar('/'));
        packages.push_back(pkgPath);
    }

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
                    PERROR(QString("Removed file since last game data scan: ") + packages[i] + "\n");
                }
                foundRemoved = true;
            }
        }
        if (!g_ipc && foundRemoved)
            PERROR("Above files removed since last game data scan.\n");

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
            PERROR("Above files added since last game data scan.\n");
    }

    return !foundRemoved && !foundAdded;
}

bool TreeScan::PrepareListOfTextures(MeType gameId, Resources &resources,
                                    QList<TextureMapEntry> &textures, QList<Texture4kNormEntry> &texture4kNorms,
                                    bool removeEmptyMips, bool saveMapFile,
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
    QString filename = path + QString("/me%1map.bin").arg((int)gameId);

#ifdef GUI
    QElapsedTimer timer;
    timer.start();
#endif
    Misc::restartStageTimer();
    if (!generateBuiltinMapFiles && !g_GameData->FullScanGame)
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

    if (!g_GameData->FullScanGame)
    {
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
        if (GameData::gameType == MeType::ME1_TYPE)
        {
            g_GameData->mapME1PackageUpperNames.clear();
            for (int i = 0; i < g_GameData->packageFiles.count(); i++)
            {
                g_GameData->mapME1PackageUpperNames.insert(BaseNameWithoutExt(g_GameData->packageFiles[i]).toLower(), i);
            }
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

    if (!generateBuiltinMapFiles && !g_GameData->FullScanGame)
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
            FindTextures(gameId, textures, modifiedFiles[i], texture4kNorms, true);
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
            FindTextures(gameId, textures, addedFiles[i], texture4kNorms, false);
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
            FindTextures(gameId, textures, g_GameData->packageFiles[i], texture4kNorms, false);
        }
    }

    if (callback)
        callback(callbackHandle, 100, "Scanning textures");

    if (gameId == MeType::ME1_TYPE)
    {
        for (int k = 0; k < textures.count(); k++)
        {
            bool foundWeakSlave = false;
            for (int w = 0; w < textures[k].list.count(); w++)
            {
                if (textures[k].list[w].weakSlave && !textures[k].list[w].movieTexture)
                {
                    foundWeakSlave = true;
                    break;
                }
            }
            if (foundWeakSlave)
            {
                // try make weak slave as master by matching slave with mipmap offsets
                for (int t = 0; t < textures[k].list.count(); t++)
                {
                    if (textures[k].list[t].weakSlave)
                    {
                        TextureMapPackageEntry weakSlaveTexture = textures[k].list[t];
                        uint mipmapOffset = weakSlaveTexture.mipmapOffset;
                        QString pkgName = weakSlaveTexture.packageName;
                        if (pkgName == weakSlaveTexture.basePackageName)
                            CRASH();

                        for (int j = 0; j < textures[k].list.count(); j++)
                        {
                            if (textures[k].list[j].slave &&
                                textures[k].list[j].mipmapOffset == mipmapOffset &&
                                textures[k].list[j].basePackageName == pkgName)
                            {
                                weakSlaveTexture.weakSlave = false;
                                textures[k].list[t] = weakSlaveTexture;
                                break;
                            }
                        }
                    }
                }

                // for the rest try make weak slaves as slave
                for (int t = 0; t < textures[k].list.count(); t++)
                {
                    if (textures[k].list[t].weakSlave)
                    {
                        TextureMapPackageEntry weakSlaveTexture = textures[k].list[t];
                        QString basePkgName = weakSlaveTexture.basePackageName;
                        if (basePkgName == weakSlaveTexture.packageName)
                            CRASH();

                        for (int j = 0; j < textures[k].list.count(); j++)
                        {
                            if (!textures[k].list[j].slave &&
                                !textures[k].list[j].weakSlave &&
                                 textures[k].list[j].packageName == basePkgName)
                            {
                                weakSlaveTexture.weakSlave = false;
                                weakSlaveTexture.slave = true;
                                textures[k].list[t] = weakSlaveTexture;
                                break;
                            }
                        }
                    }
                }
            }

            // sort masters on top
            QList<TextureMapPackageEntry> texList;
            for (int t = 0; t < textures[k].list.count(); t++)
            {
                if (textures[k].list[t].slave)
                    texList.push_back(textures[k].list[t]);
                else
                    texList.push_front(textures[k].list[t]);
            }
            TextureMapEntry f = textures[k];
            f.list = texList;
            textures[k] = f;

            // match slave with master by external offsets first
            for (int t = 0; t < textures[k].list.count(); t++)
            {
                if (textures[k].list[t].slave && textures[k].list[t].linkToMaster == -1)
                {
                    TextureMapPackageEntry slaveTexture = textures[k].list[t];
                    uint mipmapOffset = slaveTexture.mipmapOffset;
                    QString basePkgName = slaveTexture.basePackageName;
                    if (basePkgName == BaseNameWithoutExt(slaveTexture.path).toLower())
                        CRASH();

                    for (int j = 0; j < textures[k].list.count(); j++)
                    {
                        if (!textures[k].list[j].slave &&
                            !textures[k].list[j].weakSlave &&
                             textures[k].list[j].mipmapOffset == mipmapOffset &&
                             textures[k].list[j].packageName == basePkgName)
                        {
                            slaveTexture.linkToMaster = j;
                            textures[k].list[t] = slaveTexture;
                            break;
                        }
                    }
                }
            }

            // match rest slaves with master
            for (int t = 0; t < textures[k].list.count(); t++)
            {
                if (textures[k].list[t].slave && textures[k].list[t].linkToMaster == -1)
                {
                    TextureMapPackageEntry slaveTexture = textures[k].list[t];
                    QString basePkgName = slaveTexture.basePackageName;
                    if (basePkgName == BaseNameWithoutExt(slaveTexture.path).toLower())
                        CRASH();

                    for (int j = 0; j < textures[k].list.count(); j++)
                    {
                        if (!textures[k].list[j].slave &&
                            !textures[k].list[j].weakSlave &&
                             textures[k].list[j].packageName == basePkgName)
                        {
                            slaveTexture.linkToMaster = j;
                            textures[k].list[t] = slaveTexture;
                            break;
                        }
                    }
                }
            }

        }
    }

    for (int k = 0; k < textures.count(); k++)
    {
        for (int t = 0; t < textures[k].list.count(); t++)
        {
            if (textures[k].list[t].weakSlave) {
                QString info = QString("Orphaned weak slave texture: ") + textures[k].name +
                        ", Export Id: " + QString::number(textures[k].list[t].exportID + 1) +
                        ", Base package: " + textures[k].list[t].basePackageName +
                        ", Package path: " + textures[k].list[t].path + "\n";
                PINFO(info);
            }
            if (textures[k].list[t].linkToMaster == -1 &&
                textures[k].list[t].slave &&
                textures[k].list[t].basePackageName.length() != 0) {
                QString info = QString("Not connected slave texture: ") + textures[k].name +
                        ", Export Id: " + QString::number(textures[k].list[t].exportID + 1) +
                        ", Base package: " + textures[k].list[t].basePackageName +
                        ", Package path: " + textures[k].list[t].path + "\n";
                PINFO(info);
            }
        }
    }

    if (!g_GameData->FullScanGame)
    {
        std::sort(g_GameData->packageFiles.begin(), g_GameData->packageFiles.end(), comparePath);
        if (gameId == MeType::ME1_TYPE)
        {
            g_GameData->mapME1PackageUpperNames.clear();
            for (int i = 0; i < g_GameData->packageFiles.count(); i++)
            {
                g_GameData->mapME1PackageUpperNames.insert(BaseNameWithoutExt(g_GameData->packageFiles[i]).toLower(), i);
            }
        }
    }

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
                mem.WriteByte(texture.flags);

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
                    if (GameData::gameType == MeType::ME1_TYPE)
                    {
                        mem.WriteInt16(m.linkToMaster);
                        if (m.linkToMaster != -1)
                            mem.WriteStringASCIINull(m.basePackageName);
                        mem.WriteUInt32(m.mipmapOffset);
                    }
                    mem.WriteByte(m.removeEmptyMips ? 1 : 0);
                    mem.WriteByte(m.numMips);
                    mem.WriteInt16(pkgs.indexOf(m.path));
                }
                else
                {
                    mem.WriteInt32(m.movieTexture ? -2 : m.linkToMaster);
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

    if (removeEmptyMips)
    {
        PINFO("\nRemove empty mips started...\n");
        MipMaps mipMaps;
        QStringList pkgsToMarkers;
        QStringList pkgsToRepack;
        mipMaps.removeMipMaps(1, textures, pkgsToMarkers, pkgsToRepack, false, false, false,
                              callback, callbackHandle);
        if (GameData::gameType == MeType::ME1_TYPE)
            mipMaps.removeMipMaps(2, textures, pkgsToMarkers, pkgsToRepack, false, false, false,
                                  callback, callbackHandle);
        if (GameData::gameType == MeType::ME3_TYPE)
            TOCBinFile::UpdateAllTOCBinFiles();
        PINFO("\nRemove empty mips finished.\n\n");
    }

    return true;
}

void TreeScan::FindTextures(MeType gameId, QList<TextureMapEntry> &textures, const QString &packagePath,
                            QList<Texture4kNormEntry> &texture4kNorms, bool modified)
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
        return;
    }

    for (int i = 0; i < package.exportsTable.count(); i++)
    {
        Package::ExportEntry& exp = package.exportsTable[i];
        int id = package.getClassNameId(exp.getClassId());
        if (id == package.nameIdTexture2D ||
            id == package.nameIdLightMapTexture2D ||
            id == package.nameIdShadowMapTexture2D ||
            id == package.nameIdTextureFlipBook ||
            id == package.nameIdTextureMovie)
        {
            ByteBuffer exportData = package.getExportData(i);
            if (exportData.ptr() == nullptr)
            {
                if (g_ipc)
                {
                    ConsoleWrite(QString("[IPC]ERROR Texture ") + exp.objectName +
                                 " has broken export data in package: " +
                                 packagePath + "\nExport Id: " + QString::number(i + 1) + "\nSkipping...");
                    ConsoleSync();
                }
                else
                {
                    PERROR(QString("Error: Texture ") + exp.objectName +
                                 " has broken export data in package: " +
                                 packagePath +"\nExport Id: " + QString::number(i + 1) + "\nSkipping...\n");
                }
                continue;
            }

            TextureMovie *textureMovie = nullptr;
            Texture *texture = nullptr;
            uint crc;

            TextureMapPackageEntry matchTexture{};
            matchTexture.exportID = i;
            matchTexture.path = packagePath;

            if (id == package.nameIdTextureMovie)
            {
                textureMovie = new TextureMovie(package, i, exportData);
                exportData.Free();
                if (!textureMovie->hasTextureData())
                {
                    delete textureMovie;
                    continue;
                }
                if (gameId == MeType::ME1_TYPE)
                    matchTexture.linkToMaster = -1;
                matchTexture.movieTexture = true;
                crc = textureMovie->getCrcData();
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

                const Texture::TextureMipMap& mipmap = texture->getTopMipmap();
                matchTexture.packageName = texture->packageName;
                matchTexture.removeEmptyMips = texture->hasEmptyMips();
                matchTexture.numMips = texture->numNotEmptyMips();
                if (gameId == MeType::ME1_TYPE)
                {
                    matchTexture.basePackageName = texture->basePackageName;
                    matchTexture.slave = texture->slave;
                    matchTexture.weakSlave = texture->weakSlave;
                    matchTexture.linkToMaster = -1;
                    matchTexture.mipmapOffset = mipmap.dataOffset;
                }
                crc = texture->getCrcTopMipmap();
            }

            if (crc == 0)
            {
                if (g_ipc)
                {
                    ConsoleWrite(QString("[IPC]ERROR Texture ") + exp.objectName + " is broken in package: " +
                                 packagePath + "\nExport Id: " + QString::number(i + 1) + "\nSkipping...");
                    ConsoleSync();
                }
                else
                {
                    PERROR(QString("Error: Texture ") + exp.objectName + " is broken in package: " +
                                 packagePath +"\nExport Id: " + QString::number(i + 1) + "\nSkipping...\n");
                }
                delete textureMovie;
                delete texture;
                continue;
            }

            WarnNorm4k(gameId, texture, exp.objectName, packagePath, i, texture4kNorms);

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
                foundTex.list.push_back(matchTexture);
                foundTex.name = exp.objectName;
                foundTex.crc = crc;

                if (id == package.nameIdTextureMovie)
                {
                    if (generateBuiltinMapFiles)
                    {
                        foundTex.width = textureMovie->getProperties().getProperty("SizeX").valueInt;
                        foundTex.height = textureMovie->getProperties().getProperty("SizeY").valueInt;
                        foundTex.pixfmt = Image::getPixelFormatType(textureMovie->getProperties().getProperty("Format").valueName);
                        foundTex.flags = TextureProperty::TextureTypes::Movie;
                    }
                    delete textureMovie;
                }
                else
                {
                    if (generateBuiltinMapFiles)
                    {
                        foundTex.width = texture->getTopMipmap().width;
                        foundTex.height = texture->getTopMipmap().height;
                        foundTex.pixfmt = Image::getPixelFormatType(texture->getProperties().getProperty("Format").valueName);
                        if (texture->getProperties().exists("CompressionSettings"))
                        {
                            QString cmp = texture->getProperties().getProperty("CompressionSettings").valueName;
                            if (cmp == "TC_OneBitAlpha")
                                foundTex.flags = TextureProperty::TextureTypes::OneBitAlpha;
                            else if (cmp == "TC_Displacementmap")
                                foundTex.flags = TextureProperty::TextureTypes::Displacementmap;
                            else if (cmp == "TC_Grayscale")
                                foundTex.flags = TextureProperty::TextureTypes::GreyScale;
                            else if (cmp == "TC_Normalmap" ||
                                cmp == "TC_NormalmapHQ" ||
                                cmp == "TC_NormalmapAlpha" ||
                                cmp == "TC_NormalmapUncompressed")
                            {
                                foundTex.flags = TextureProperty::TextureTypes::Normalmap;
                            }
                            else
                            {
                                CRASH();
                            }
                        }
                        else
                        {
                            foundTex.flags = TextureProperty::TextureTypes::Normal;
                        }
                    }
                    delete texture;
                }
                textures.push_back(foundTex);
            }
        }
    }
}
