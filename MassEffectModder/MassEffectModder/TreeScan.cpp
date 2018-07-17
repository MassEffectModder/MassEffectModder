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

static bool generateBuiltinMapFiles = false; // change to true to enable map files generation

void TreeScan::loadTexturesMap(MeType gameId, Resources *resources, QList<FoundTexture> *textures)
{
    QStringList pkgs;
    if (gameId == MeType::ME1_TYPE)
        pkgs = resources->tablePkgsME1;
    else if (gameId == MeType::ME2_TYPE)
        pkgs = resources->tablePkgsME2;
    else
        pkgs = resources->tablePkgsME3;

    FileStream *tmp = new FileStream(QString(":/Resources/me%1map.bin").arg((int)gameId), FileMode::Open);
    if (tmp->ReadUInt32() != 0x504D5443)
        CRASH();
    ByteBuffer decompressed = ByteBuffer(tmp->ReadInt32());
    ByteBuffer compressed = tmp->ReadToBuffer(tmp->ReadUInt32());
    uint dstLen = decompressed.size();
    ZlibDecompress(compressed.ptr(), compressed.size(), decompressed.ptr(), &dstLen);
    if (decompressed.size() != dstLen)
        CRASH();
    delete tmp;

    Stream *fs = new MemoryStream(decompressed);
    fs->Skip(8);
    uint countTexture = fs->ReadUInt32();
    for (uint i = 0; i < countTexture; i++)
    {
        FoundTexture texture{};
        int len = fs->ReadByte();
        fs->ReadStringASCII(texture.name, len);
        texture.crc = fs->ReadUInt32();
        texture.width = fs->ReadInt16();
        texture.height = fs->ReadInt16();
        texture.pixfmt = (PixelFormat)fs->ReadByte();
        texture.flags = (TexProperty::TextureTypes)fs->ReadByte();
        int countPackages = fs->ReadInt16();
        texture.list = new QList<MatchedTexture>();
        for (int k = 0; k < countPackages; k++)
        {
            MatchedTexture matched{};
            matched.exportID = fs->ReadInt32();
            if (gameId == MeType::ME1_TYPE)
            {
                matched.linkToMaster = fs->ReadInt16();
                if (matched.linkToMaster != -1)
                {
                    matched.slave = true;
                    fs->ReadStringASCIINull(matched.basePackageName);
                }
            }
            matched.removeEmptyMips = fs->ReadByte() != 0;
            matched.numMips = fs->ReadByte();
            matched.path = pkgs[fs->ReadInt16()];
            matched.packageName = BaseNameWithoutExt(matched.path).toUpper();
            texture.list->push_back(matched);
        }
        textures->push_back(texture);
    }
}

int TreeScan::PrepareListOfTextures(MeType gameId, Resources *resources, QList<FoundTexture> *textures, bool ipc)
{
    QStringList pkgs;
    QList<MD5FileEntry> md5Entries;
    if (gameId == MeType::ME1_TYPE)
    {
        pkgs = resources->tablePkgsME1;
        md5Entries = resources->entriesME1;
    }
    else if (gameId == MeType::ME2_TYPE)
    {
        pkgs = resources->tablePkgsME2;
        md5Entries = resources->entriesME2;
    }
    else
    {
        pkgs = resources->tablePkgsME3;
        md5Entries = resources->entriesME3;
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

    if (!g_GameData->FullScanME1Game)
    {
        g_GameData->packageFiles.sort(Qt::CaseInsensitive);
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
        sortedFiles.sort(Qt::CaseInsensitive);

        for (int k = 0; k < textures->count(); k++)
        {
            for (int t = 0; t < textures->at(k).list->count(); t++)
            {
                QString pkgPath = textures->at(k).list->at(t).path;
                if (std::binary_search(sortedFiles.begin(), sortedFiles.end(), pkgPath))
                    continue;
                MatchedTexture f = textures->at(k).list->at(t);
                f.path = "";
                textures->at(k).list->replace(t, f);
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
            ConsoleWrite("[IPC]STAGE_WEIGHT STAGE_SCAN " +
                QString::number(((float)totalPackages / g_GameData->packageFiles.count())));
            ConsoleSync();
        }
        for (int i = 0; i < modifiedFiles.count(); i++, currentPackage++)
        {
            if (ipc)
            {
                ConsoleWrite("[IPC]PROCESSING_FILE " + modifiedFiles[i]);
                int newProgress = currentPackage * 100 / totalPackages;
                if (lastProgress != newProgress)
                {
                    ConsoleWrite("[IPC]TASK_PROGRESS " + QString::number(newProgress));
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
                ConsoleWrite("[IPC]PROCESSING_FILE " + addedFiles[i]);
                int newProgress = currentPackage * 100 / totalPackages;
                if (lastProgress != newProgress)
                {
                    ConsoleWrite("[IPC]TASK_PROGRESS " + QString::number(newProgress));
                    lastProgress = newProgress;
                }
                ConsoleSync();
            }
            FindTextures(gameId, textures, addedFiles[i], false, ipc);
        }

        for (int k = 0; k < textures->count(); k++)
        {
            bool found = false;
            for (int t = 0; t < textures->at(k).list->count(); t++)
            {
                if (textures->at(k).list->at(t).path != "")
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                textures->at(k).list->clear();
                textures->removeAt(k);
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
        for (int k = 0; k < textures->count(); k++)
        {
            for (int t = 0; t < textures->at(k).list->count(); t++)
            {
                uint mipmapOffset = textures->at(k).list->at(t).mipmapOffset;
                if (textures->at(k).list->at(t).slave)
                {
                    MatchedTexture slaveTexture = textures->at(k).list->at(t);
                    QString basePkgName = slaveTexture.basePackageName;
                    if (basePkgName == BaseNameWithoutExt(slaveTexture.path).toUpper())
                        CRASH();
                    for (int j = 0; j < textures->at(k).list->count(); j++)
                    {
                        if (!textures->at(k).list->at(j).slave &&
                           textures->at(k).list->at(j).mipmapOffset == mipmapOffset &&
                           textures->at(k).list->at(j).packageName == basePkgName)
                        {
                            slaveTexture.linkToMaster = j;
                            slaveTexture.slave = true;
                            textures->at(k).list->replace(t, slaveTexture);
                            break;
                        }
                    }
                }
            }

            bool foundWeakSlave = false;
            for (int w = 0; w < textures->at(k).list->count(); w++)
            {
                if (!textures->at(k).list->at(w).slave &&
                     textures->at(k).list->at(w).weakSlave)
                {
                    foundWeakSlave = true;
                }
            }
            if (foundWeakSlave)
            {
                auto *texList = new QList<MatchedTexture>();
                for (int t = 0; t < textures->at(k).list->count(); t++)
                {
                    MatchedTexture tex = textures->at(k).list->at(t);
                    if (tex.weakSlave)
                        texList->push_back(tex);
                    else
                        texList->push_front(tex);
                }
                FoundTexture f = textures->at(k);
                delete f.list;
                f.list = texList;
                textures->replace(k, f);
                if (textures->at(k).list->first().weakSlave)
                    continue;

                for (int t = 0; t < textures->at(k).list->count(); t++)
                {
                    if (textures->at(k).list->at(t).weakSlave)
                    {
                        MatchedTexture slaveTexture = textures->at(k).list->at(t);
                        QString basePkgName = slaveTexture.basePackageName;
                        if (basePkgName == BaseNameWithoutExt(slaveTexture.path).toUpper())
                            CRASH();
                        for (int j = 0; j < textures->at(k).list->count(); j++)
                        {
                            if (!textures->at(k).list->at(j).weakSlave &&
                               textures->at(k).list->at(j).packageName == basePkgName)
                            {
                                slaveTexture.linkToMaster = j;
                                slaveTexture.slave = true;
                                textures->at(k).list->replace(t, slaveTexture);
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

    auto fs = new FileStream(filename, FileMode::Create, FileAccess::WriteOnly);
    MemoryStream mem;
    mem.WriteUInt32(textureMapBinTag);
    mem.WriteUInt32(textureMapBinVersion);
    mem.WriteInt32(textures->count());

    for (int i = 0; i < textures->count(); i++)
    {
        const FoundTexture& texture = textures->at(i);
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

            mem.WriteInt16(texture.list->count());
        }
        else
        {
            mem.WriteInt32(texture.list->count());
        }
        for (int k = 0; k < texture.list->count(); k++)
        {
            const MatchedTexture& m = texture.list->at(k);
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
            mem.WriteStringASCII(g_GameData->packageFiles[i]);
        }
    }
    mem.SeekBegin();

    if (generateBuiltinMapFiles)
    {
        fs->WriteUInt32(0x504D5443);
        fs->WriteUInt32(mem.Length());
        quint8 *compressed = nullptr;
        uint compressedSize = 0;
        ZlibCompress(mem.ToArray().ptr(), mem.Length(), &compressed, &compressedSize);
        fs->WriteUInt32(compressedSize);
        fs->WriteFromBuffer(compressed, compressedSize);
    }
    else
    {
        fs->CopyFrom(&mem, mem.Length());
    }
    delete fs;

    return 0;
}

void TreeScan::FindTextures(MeType gameId, QList<FoundTexture> *textures, const QString &packagePath,
                            bool modified, bool ipc)
{
    Package package;
    int status = package.Open(g_GameData->GamePath() + packagePath);
    if (status != 0)
    {
        if (ipc)
        {
            ConsoleWrite("[IPC]ERROR Issue opening package file: " + packagePath);
            ConsoleSync();
        }
        else
        {
            ConsoleWrite("ERROR: Issue opening package file: " + packagePath);
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

            const Texture::MipMap& mipmap = texture->getTopMipmap();
            QString name = package.exportsTable.at(i).objectName;
            MatchedTexture matchTexture;
            matchTexture.exportID = i;
            matchTexture.path = packagePath;
            matchTexture.packageName = texture->packageName;
            matchTexture.removeEmptyMips = texture->hasEmptyMips();
            matchTexture.numMips = texture->numNotEmptyMips();
            matchTexture.crcs = nullptr;
            matchTexture.masterDataOffset = nullptr;
            if (gameId == MeType::ME1_TYPE)
            {
                matchTexture.basePackageName = texture->basePackageName;
                matchTexture.slave = texture->slave;
                matchTexture.weakSlave = texture->weakSlave;
                matchTexture.linkToMaster = -1;
                if (matchTexture.slave)
                    matchTexture.mipmapOffset = mipmap.dataOffset;
                else
                    matchTexture.mipmapOffset = exp.getDataOffset() + texture->properties->propertyEndOffset + mipmap.internalOffset;
            }

            uint crc = texture->getCrcTopMipmap();
            if (crc == 0)
            {
                if (ipc)
                {
                    ConsoleWrite("[IPC]ERROR Texture " + exp.objectName + " is broken in package: " + packagePath + ", skipping...");
                    ConsoleSync();
                }
                else
                {
                    ConsoleWrite("Error: Texture " + exp.objectName + " is broken in package: " + packagePath + ", skipping...");
                }
                delete texture;
                continue;
            }

            FoundTexture foundTexName = {};
            for (int k = 0; k < textures->count(); k++)
            {
                if (textures->at(k).crc == crc)
                {
                    foundTexName = textures->at(k);
                    break;
                }
            }
            QString packagePathLower = packagePath.toLower();
            if (foundTexName.crc != 0)
            {
                if (modified)
                {
                    for (int s = 0; s < foundTexName.list->count(); s++)
                    {
                        if (foundTexName.list->at(s).exportID == i &&
                            foundTexName.list->at(s).path.toLower() == packagePathLower)
                        {
                            continue;
                        }
                    }
                }
                if (matchTexture.slave || gameId != MeType::ME1_TYPE)
                    foundTexName.list->push_back(matchTexture);
                else
                    foundTexName.list->push_front(matchTexture);
            }
            else
            {
                if (modified)
                {
                    for (int k = 0; k < textures->count(); k++)
                    {
                        bool found = false;
                        for (int t = 0; t < textures->at(k).list->count(); t++)
                        {
                            if (textures->at(k).list->at(t).exportID == i &&
                                textures->at(k).list->at(t).path.toLower() == packagePathLower)
                            {
                                MatchedTexture f = textures->at(k).list->at(t);
                                f.path = "";
                                textures->at(k).list->replace(t, f);
                                found = true;
                                break;
                            }
                        }
                        if (found)
                            break;
                    }
                }
                FoundTexture foundTex;
                foundTex.list = new QList<MatchedTexture>();
                foundTex.list->push_back(matchTexture);
                foundTex.name = name;
                foundTex.crc = crc;
                if (generateBuiltinMapFiles)
                {
                    foundTex.width = texture->getTopMipmap().width;
                    foundTex.height = texture->getTopMipmap().height;
                    foundTex.pixfmt = Image::getPixelFormatType(texture->properties->getProperty("Format").valueName);
                    if (texture->properties->exists("CompressionSettings"))
                    {
                        QString cmp = texture->properties->getProperty("CompressionSettings").valueName;
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
                textures->push_back(foundTex);
            }
            delete texture;
        }
    }
}

void TreeScan::ReleaseTreeScan(QList<FoundTexture> *textures)
{
    for (int k = 0; k < textures->count(); k++)
    {
        const FoundTexture& t = textures->at(k);
        for (int e = 0; e < t.list->count(); e++)
        {
            const MatchedTexture& m = t.list->at(e);
            delete m.crcs;
            delete m.masterDataOffset;
        }
        delete t.list;
    }
}
