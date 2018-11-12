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

#include "Exceptions/SignalHandler.h"
#include "Helpers/MiscHelpers.h"
#include "Wrappers.h"

#include "TreeScan.h"
#include "Texture.h"
#include "Package.h"
#include "ConfigIni.h"
#include "GameData.h"
#include "Image.h"
#include "Resources.h"
#include "MemTypes.h"
#include "TOCFile.h"
#include "MipMaps.h"

static bool generateBuiltinMapFiles = false; // change to true to enable map files generation

void TreeScan::loadTexturesMap(MeType gameId, Resources &resources, QList<FoundTexture> &textures)
{
    QStringList pkgs;
    if (gameId == MeType::ME1_TYPE)
        pkgs = resources.tablePkgsME1;
    else if (gameId == MeType::ME2_TYPE)
        pkgs = resources.tablePkgsME2;
    else
        pkgs = resources.tablePkgsME3;

    FileStream tmp = FileStream(QString(":/Resources/me%1map.bin").arg((int)gameId), FileMode::Open, FileAccess::ReadOnly);
    if (tmp.ReadUInt32() != 0x504D5443)
        CRASH();
    ByteBuffer decompressed = ByteBuffer(tmp.ReadInt32());
    ByteBuffer compressed = tmp.ReadToBuffer(tmp.ReadUInt32());
    uint dstLen = decompressed.size();
    ZlibDecompress(compressed.ptr(), compressed.size(), decompressed.ptr(), &dstLen);
    if (decompressed.size() != dstLen)
        CRASH();

    auto fs = MemoryStream(decompressed);
    fs.Skip(8);
    uint countTexture = fs.ReadUInt32();
    for (uint i = 0; i < countTexture; i++)
    {
        FoundTexture texture{};
        int len = fs.ReadByte();
        fs.ReadStringASCII(texture.name, len);
        texture.crc = fs.ReadUInt32();
        texture.width = fs.ReadInt16();
        texture.height = fs.ReadInt16();
        texture.pixfmt = (PixelFormat)fs.ReadByte();
        texture.flags = (TexProperty::TextureTypes)fs.ReadByte();
        int countPackages = fs.ReadInt16();
        texture.list = QList<MatchedTexture>();
        for (int k = 0; k < countPackages; k++)
        {
            MatchedTexture matched{};
            matched.exportID = fs.ReadInt32();
            if (gameId == MeType::ME1_TYPE)
            {
                matched.linkToMaster = fs.ReadInt16();
                if (matched.linkToMaster != -1)
                {
                    matched.slave = true;
                    fs.ReadStringASCIINull(matched.basePackageName);
                }
            }
            matched.removeEmptyMips = fs.ReadByte() != 0;
            matched.numMips = fs.ReadByte();
            matched.path = pkgs[fs.ReadInt16()];
            matched.path.replace(QChar('\\'), QChar('/'));
            matched.packageName = BaseNameWithoutExt(matched.path).toUpper();
            texture.list.push_back(matched);
        }
        textures.push_back(texture);
    }
}

bool TreeScan::loadTexturesMapFile(QString &path, QList<FoundTexture> &textures, bool ipc)
{
    if (!QFile(path).exists())
    {
        if (ipc)
        {
            ConsoleWrite("[IPC]ERROR_TEXTURE_MAP_MISSING");
            ConsoleSync();
        }
        else
        {
            ConsoleWrite("Missing textures scan file!");
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
        if (ipc)
        {
            ConsoleWrite("[IPC]ERROR_TEXTURE_MAP_WRONG");
            ConsoleSync();
        }
        else
        {
            ConsoleWrite("Detected wrong or old version of textures scan file!");
        }
        return false;
    }

    uint countTexture = fs.ReadUInt32();
    for (uint i = 0; i < countTexture; i++)
    {
        FoundTexture texture{};
        int len = fs.ReadInt32();
        fs.ReadStringASCII(texture.name, len);
        texture.crc = fs.ReadUInt32();
        uint countPackages = fs.ReadUInt32();
        texture.list = QList<MatchedTexture>();
        for (uint k = 0; k < countPackages; k++)
        {
            MatchedTexture matched{};
            matched.exportID = fs.ReadInt32();
            matched.linkToMaster = fs.ReadInt32();
            len = fs.ReadInt32();
            fs.ReadStringASCII(matched.path, len);
            matched.path.replace(QChar('\\'), QChar('/'));
            texture.list.push_back(matched);
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

    for (int i = 0; i < packages.count(); i++)
    {
        bool found = false;
        for (int s = 0; s < g_GameData->packageFiles.count(); s++)
        {
            if (g_GameData->packageFiles[s].compare(packages[i], Qt::CaseInsensitive) == 0)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            if (ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR_REMOVED_FILE ") + packages[i]);
                ConsoleSync();
            }
            else
            {
                ConsoleWrite(QString("Removed file since last game data scan: ") + packages[i]);
            }
            foundRemoved = true;
        }
    }
    if (!ipc && foundRemoved)
        ConsoleWrite("Above files removed since last game data scan.");

    for (int i = 0; i < g_GameData->packageFiles.count(); i++)
    {
        bool found = false;
        for (int s = 0; s < packages.count(); s++)
        {
            if (packages[s].compare(g_GameData->packageFiles[i], Qt::CaseInsensitive) == 0)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            if (ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR_ADDED_FILE ") + g_GameData->packageFiles[i]);
                ConsoleSync();
            }
            else
            {
                ConsoleWrite(QString("File: ") + g_GameData->packageFiles[i]);
            }
            foundAdded = true;
        }
    }
    if (!ipc && foundAdded)
        ConsoleWrite("Above files added since last game data scan.");

    return !foundRemoved && !foundAdded;
}

int TreeScan::PrepareListOfTextures(MeType gameId, Resources &resources, QList<FoundTexture> &textures,
                                    QStringList &pkgsToMarker, QStringList &pkgsToRepack, MipMaps &mipMaps,
                                    bool ipc, bool repack)
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

    if (ipc)
    {
        ConsoleWrite("[IPC]STAGE_CONTEXT STAGE_PRESCAN");
        ConsoleSync();
    }
g_GameData->FullScanME1Game = true;
    if (!g_GameData->FullScanME1Game)
    {
        int count = g_GameData->packageFiles.count();
        for (int i = 0; i < count; i++)
        {
            QString str = g_GameData->packageFiles[i];
            if (str.contains("_IT.") ||
                str.contains("_FR.") ||
                str.contains("_ES.") ||
                str.contains("_DE.") ||
                str.contains("_RA.") ||
                str.contains("_RU.") ||
                str.contains("_PLPC.") ||
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
    }

    if (!generateBuiltinMapFiles && !g_GameData->FullScanME1Game)
    {
        QStringList addedFiles;
        QStringList modifiedFiles;

        loadTexturesMap(gameId, resources, textures);

        QStringList sortedFiles;
        for (int i = 0; i < g_GameData->packageFiles.count(); i++)
        {
            sortedFiles.push_back(g_GameData->RelativeGameData(g_GameData->packageFiles[i]).toLower());
        }
        std::sort(sortedFiles.begin(), sortedFiles.end(), compareByAscii);

        for (int k = 0; k < textures.count(); k++)
        {
            for (int t = 0; t < textures[k].list.count(); t++)
            {
                QString pkgPath = textures[k].list[t].path;
                if (std::binary_search(sortedFiles.begin(), sortedFiles.end(), pkgPath, compareByAscii))
                    continue;
                MatchedTexture f = textures[k].list[t];
                f.path = "";
                textures[k].list[t] = f;
            }
        }

        if (ipc)
        {
            ConsoleWrite("[IPC]STAGE_CONTEXT STAGE_SCAN");
            ConsoleSync();
        }
        for (int i = 0; i < g_GameData->packageFiles.count(); i++)
        {
            int index = -1;
            bool modified = true;
            bool foundPkg = false;
            QString package = g_GameData->RelativeGameData(g_GameData->packageFiles[i]).toLower();
            long packageSize = QFile(g_GameData->packageFiles[i]).size();
            for (int p = 0; p < md5Entries.count(); p++)
            {
                if (package == md5Entries[p].path.toLower())
                {
                    foundPkg = true;
                    if (packageSize == md5Entries[p].size)
                    {
                        modified = false;
                        break;
                    }
                    index = p;
                }
            }
            if (foundPkg && modified)
                modifiedFiles.push_back(md5Entries[index].path);
            else if (!foundPkg)
                addedFiles.push_back(g_GameData->RelativeGameData(g_GameData->packageFiles[i]));
        }

        int lastProgress = -1;
        int totalPackages = modifiedFiles.count() + addedFiles.count();
        int currentPackage = 0;
        if (ipc)
        {
            ConsoleWrite(QString("[IPC]STAGE_WEIGHT STAGE_SCAN ") +
                QString::number(((float)totalPackages / g_GameData->packageFiles.count())));
            ConsoleSync();
        }
        for (int i = 0; i < modifiedFiles.count(); i++, currentPackage++)
        {
            if (ipc)
            {
                ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + modifiedFiles[i]);
                int newProgress = currentPackage * 100 / totalPackages;
                if (lastProgress != newProgress)
                {
                    ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
                    lastProgress = newProgress;
                }
                ConsoleSync();
            }
            FindTextures(gameId, textures, modifiedFiles[i], true, ipc);
        }

        for (int i = 0; i < addedFiles.count(); i++, currentPackage++)
        {
            if (ipc)
            {
                ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + addedFiles[i]);
                int newProgress = currentPackage * 100 / totalPackages;
                if (lastProgress != newProgress)
                {
                    ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
                    lastProgress = newProgress;
                }
                ConsoleSync();
            }
            FindTextures(gameId, textures, addedFiles[i], false, ipc);
        }

        for (int k = 0; k < textures.count(); k++)
        {
            bool found = false;
            for (int t = 0; t < textures[k].list.count(); t++)
            {
                if (textures[k].list[t].path != "")
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                textures[k].list.clear();
                textures.removeAt(k);
                k--;
            }
        }
    }
    else
    {
        int lastProgress = -1;
        for (int i = 0; i < g_GameData->packageFiles.count(); i++)
        {
            if (ipc)
            {
                ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + g_GameData->packageFiles[i]);
                int newProgress = i * 100 / g_GameData->packageFiles.count();
                if (lastProgress != newProgress)
                {
                    ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
                    lastProgress = newProgress;
                }
                ConsoleSync();
            }
            else
            {
                ConsoleWrite(QString("Package ") + QString::number(i + 1) + "/" +
                                     QString::number(g_GameData->packageFiles.count()) + " : " +
                                     g_GameData->packageFiles[i]);
            }
            FindTextures(gameId, textures, g_GameData->packageFiles[i], false, ipc);
        }
    }

    if (gameId == MeType::ME1_TYPE)
    {
        for (int k = 0; k < textures.count(); k++)
        {
            for (int t = 0; t < textures[k].list.count(); t++)
            {
                uint mipmapOffset = textures[k].list[t].mipmapOffset;
                if (textures[k].list[t].slave)
                {
                    MatchedTexture slaveTexture = textures[k].list[t];
                    QString basePkgName = slaveTexture.basePackageName;
                    if (basePkgName == BaseNameWithoutExt(slaveTexture.path).toUpper())
                        CRASH();
                    for (int j = 0; j < textures[k].list.count(); j++)
                    {
                        if (!textures[k].list[j].slave &&
                           textures[k].list[j].mipmapOffset == mipmapOffset &&
                           textures[k].list[j].packageName == basePkgName)
                        {
                            slaveTexture.linkToMaster = j;
                            slaveTexture.slave = true;
                            textures[k].list[t] = slaveTexture;
                            break;
                        }
                    }
                }
            }

            bool foundWeakSlave = false;
            for (int w = 0; w < textures[k].list.count(); w++)
            {
                if (!textures[k].list[w].slave &&
                     textures[k].list[w].weakSlave)
                {
                    foundWeakSlave = true;
                    break;
                }
            }
            if (foundWeakSlave)
            {
                auto texList = QList<MatchedTexture>();
                for (int t = 0; t < textures[k].list.count(); t++)
                {
                    MatchedTexture tex = textures[k].list[t];
                    if (tex.weakSlave)
                        texList.push_back(tex);
                    else
                        texList.push_front(tex);
                }
                FoundTexture f = textures[k];
                f.list = texList;
                textures[k] = f;
                if (textures[k].list.first().weakSlave)
                    continue;

                for (int t = 0; t < textures[k].list.count(); t++)
                {
                    if (textures[k].list[t].weakSlave)
                    {
                        MatchedTexture slaveTexture = textures[k].list[t];
                        QString basePkgName = slaveTexture.basePackageName;
                        if (basePkgName == BaseNameWithoutExt(slaveTexture.path).toUpper())
                            CRASH();
                        for (int j = 0; j < textures[k].list.count(); j++)
                        {
                            if (!textures[k].list[j].weakSlave &&
                               textures[k].list[j].packageName == basePkgName)
                            {
                                slaveTexture.linkToMaster = j;
                                slaveTexture.slave = true;
                                textures[k].list[t] = slaveTexture;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    if (QFile(filename).exists())
        QFile(filename).remove();

    auto fs = FileStream(filename, FileMode::Create, FileAccess::WriteOnly);
    MemoryStream mem;
    mem.WriteUInt32(textureMapBinTag);
    mem.WriteUInt32(textureMapBinVersion);
    mem.WriteInt32(textures.count());

    for (int i = 0; i < textures.count(); i++)
    {
        const FoundTexture& texture = textures.at(i);
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
            const MatchedTexture& m = texture.list[k];
            mem.WriteInt32(m.exportID);
            if (generateBuiltinMapFiles)
            {
                if (GameData::gameType == MeType::ME1_TYPE)
                {
                    mem.WriteInt16(m.linkToMaster);
                    if (m.linkToMaster != -1)
                        mem.WriteStringASCIINull(m.basePackageName);
                }
                mem.WriteByte(m.removeEmptyMips ? 1 : 0);
                mem.WriteByte(m.numMips);
                mem.WriteInt16(pkgs.indexOf(m.path));
            }
            else
            {
                mem.WriteInt32(m.linkToMaster);
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
        ZlibCompress(mem.ToArray().ptr(), mem.Length(), &compressed, &compressedSize);
        fs.WriteUInt32(compressedSize);
        fs.WriteFromBuffer(compressed, compressedSize);
        delete[] compressed;
    }
    else
    {
        fs.CopyFrom(mem, mem.Length());
    }

    if (!generateBuiltinMapFiles)
    {
        if (GameData::gameType == MeType::ME1_TYPE)
        {
            mipMaps.removeMipMapsME1(1, textures, pkgsToMarker, ipc);
            mipMaps.removeMipMapsME1(2, textures, pkgsToMarker, ipc);
        }
        else
        {
            mipMaps.removeMipMapsME2ME3(textures, pkgsToMarker, pkgsToRepack, ipc, repack);
        }
        if (GameData::gameType == MeType::ME3_TYPE)
        {
            TOCBinFile::UpdateAllTOCBinFiles();
        }
    }

    return 0;
}

void TreeScan::FindTextures(MeType gameId, QList<FoundTexture> &textures, const QString &packagePath,
                            bool modified, bool ipc)
{
    Package package;
    int status = package.Open(g_GameData->GamePath() + packagePath);
    if (status != 0)
    {
        if (ipc)
        {
            ConsoleWrite(QString("[IPC]ERROR Issue opening package file: ") + packagePath);
            ConsoleSync();
        }
        else
        {
            ConsoleWrite(QString("ERROR: Issue opening package file: ") + packagePath);
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
            id == package.nameIdTextureFlipBook)
        {
            ByteBuffer exportData = package.getExportData(i);
            auto texture = new Texture(package, i, exportData);
            exportData.Free();
            if (!texture->hasImageData())
            {
                delete texture;
                continue;
            }

            const Texture::TextureMipMap& mipmap = texture->getTopMipmap();
            QString name = exp.objectName;
            MatchedTexture matchTexture;
            matchTexture.exportID = i;
            matchTexture.path = packagePath;
            matchTexture.packageName = texture->packageName;
            matchTexture.removeEmptyMips = texture->hasEmptyMips();
            matchTexture.numMips = texture->numNotEmptyMips();
            if (gameId == MeType::ME1_TYPE)
            {
                matchTexture.basePackageName = texture->basePackageName;
                matchTexture.slave = texture->slave;
                matchTexture.weakSlave = texture->weakSlave;
                matchTexture.linkToMaster = -1;
                if (matchTexture.slave)
                    matchTexture.mipmapOffset = mipmap.dataOffset;
                else
                    matchTexture.mipmapOffset = exp.getDataOffset() + texture->getProperties().propertyEndOffset + mipmap.internalOffset;
            }

            uint crc = texture->getCrcTopMipmap();
            if (crc == 0)
            {
                if (ipc)
                {
                    ConsoleWrite(QString("[IPC]ERROR Texture ") + exp.objectName + " is broken in package: " +
                                 packagePath + "\nExport Id: " + (i + 1) + "\nSkipping...");
                    ConsoleSync();
                }
                else
                {
                    ConsoleWrite(QString("Error: Texture ") + exp.objectName + " is broken in package: " +
                                 packagePath +"\nExport Id: " + (i + 1) + "\nSkipping...");
                }
                delete texture;
                continue;
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
                const FoundTexture& foundTexName = textures[foundTextureIndex];
                if (modified)
                {
                    bool found = false;
                    for (int s = 0; s < foundTexName.list.count(); s++)
                    {
                        if (foundTexName.list[s].exportID == i &&
                            foundTexName.list[s].path.compare(packagePathLower, Qt::CaseInsensitive) == 0)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (found)
                    {
                        delete texture;
                        continue;
                    }
                }
                if (matchTexture.slave || gameId != MeType::ME1_TYPE)
                    textures[foundTextureIndex].list.push_back(matchTexture);
                else
                    textures[foundTextureIndex].list.push_front(matchTexture);
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
                                textures[k].list[t].path.compare(packagePathLower, Qt::CaseInsensitive) == 0)
                            {
                                MatchedTexture f = textures[k].list[t];
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
                FoundTexture foundTex;
                foundTex.list.push_back(matchTexture);
                foundTex.name = name;
                foundTex.crc = crc;
                if (generateBuiltinMapFiles)
                {
                    foundTex.width = texture->getTopMipmap().width;
                    foundTex.height = texture->getTopMipmap().height;
                    foundTex.pixfmt = Image::getPixelFormatType(texture->getProperties().getProperty("Format").valueName);
                    if (texture->getProperties().exists("CompressionSettings"))
                    {
                        QString cmp = texture->getProperties().getProperty("CompressionSettings").valueName;
                        if (cmp == "TC_OneBitAlpha")
                            foundTex.flags = TexProperty::TextureTypes::OneBitAlpha;
                        else if (cmp == "TC_Displacementmap")
                            foundTex.flags = TexProperty::TextureTypes::Displacementmap;
                        else if (cmp == "TC_Grayscale")
                            foundTex.flags = TexProperty::TextureTypes::GreyScale;
                        else if (cmp == "TC_Normalmap" ||
                            cmp == "TC_NormalmapHQ" ||
                            cmp == "TC_NormalmapAlpha" ||
                            cmp == "TC_NormalmapUncompressed")
                        {
                            foundTex.flags = TexProperty::TextureTypes::Normalmap;
                        }
                        else
                        {
                            CRASH();
                        }
                    }
                    else
                    {
                        foundTex.flags = TexProperty::TextureTypes::Normal;
                    }
                }
                textures.push_back(foundTex);
            }
            delete texture;
        }
    }
}
