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
#include "Helpers/MiscHelpers.h"

void MipMaps::prepareListToRemove(QList<FoundTexture> &textures, QList<RemoveMipsEntry> &list)
{
    for (int k = 0; k < textures.count(); k++)
    {
        for (int t = 0; t < textures[k].list.count(); t++)
        {
            if (textures[k].list[t].path.length() == 0)
                continue;
            if (textures[k].list[t].removeEmptyMips)
            {
                bool found = false;
                for (int e = 0; e < list.count(); e++)
                {
                    if (list[e].pkgPath == textures[k].list[t].path)
                    {
                        RemoveMipsEntry entry = list[e];
                        entry.exportIDs.push_back(textures[k].list[t].exportID);
                        list.replace(e, entry);
                        found = true;
                        break;
                    }
                }
                if (found)
                    continue;
                RemoveMipsEntry entry{};
                entry.pkgPath = textures[k].list[t].path;
                entry.exportIDs.push_back(textures[k].list[t].exportID);
                list.push_back(entry);
            }
        }
    }
}

void MipMaps::removeMipMapsME1(int phase, QList<FoundTexture> &textures, QStringList &pkgsToMarker,
                                 bool ipc, bool appendMarker)
{
    int lastProgress = -1;

    QList<RemoveMipsEntry> list;
    prepareListToRemove(textures, list);
    QString path = "/BioGame/CookedPC/testVolumeLight_VFX.upk";
    for (int i = 0; i < list.count(); i++)
    {
        if (path.compare(list[i].pkgPath, Qt::CaseInsensitive) == 0)
            continue;

        if (ipc)
        {
            ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + list[i].pkgPath);
            int newProgress = (list.count() * (phase - 1) + i + 1) * 100 / (list.count() * 2);
            if (lastProgress != newProgress)
            {
                ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
                ConsoleSync();
                lastProgress = newProgress;
            }
        }
        else
        {
            ConsoleWrite("Removing empty mipmaps (" + QString::number(phase) + ") " +
                         QString::number(i + 1) + "/" +
                         QString::number(list.count()) + " " + list[i].pkgPath);
        }

        Package package{};
        if (package.Open(g_GameData->GamePath() + list[i].pkgPath) != 0)
        {
            if (ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR Issue opening package file: ") + list[i].pkgPath);
                ConsoleSync();
            }
            else
            {
                QString err;
                err += "---- Start --------------------------------------------\n";
                err += "Issue opening package file: " + list[i].pkgPath + "\n";
                err += "---- End ----------------------------------------------\n\n";
                ConsoleWrite(err);
            }
            return;
        }

        removeMipMapsME1(phase, textures, package, list[i], pkgsToMarker, ipc, appendMarker);
    }
}

void MipMaps::removeMipMapsME1(int phase, QList<FoundTexture> &textures, Package &package,
                                 RemoveMipsEntry &removeEntry, QStringList &pkgsToMarker,
                                 bool ipc, bool appendMarker)
{
    for (int l = 0; l < removeEntry.exportIDs.count(); l++)
    {
        int exportID = removeEntry.exportIDs[l];
        ByteBuffer exportData = package.getExportData(exportID);
        if (exportData.ptr() == nullptr)
        {
            if (ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR Texture has broken export data in package: ") +
                             package.packagePath + "\nExport Id: " + QString::number(exportID + 1) + "\nSkipping...");
                ConsoleSync();
            }
            else
            {
                ConsoleWrite(QString("Error: Texture has broken export data in package: ") +
                             package.packagePath +"\nExport Id: " + QString::number(exportID + 1) + "\nSkipping...");
            }
            continue;
        }
        Texture texture = Texture(package, exportID, exportData, false);
        exportData.Free();
        if (!texture.hasEmptyMips())
        {
            continue;
        }
        texture.removeEmptyMips();
        texture.getProperties().setIntValue("SizeX", texture.mipMapsList.first().width);
        texture.getProperties().setIntValue("SizeY", texture.mipMapsList.first().height);
        texture.getProperties().setIntValue("MipTailBaseIdx", texture.mipMapsList.count() - 1);

        int foundListEntry = -1;
        int foundTextureEntry = -1;
        QString pkgName = package.packagePath.toLower();
        for (int k = 0; k < textures.count(); k++)
        {
            for (int t = 0; t < textures[k].list.count(); t++)
            {
                if (textures[k].list[t].exportID == exportID &&
                    textures[k].list[t].path.toLower() == pkgName)
                {
                    foundTextureEntry = k;
                    foundListEntry = t;
                    break;
                }
            }
        }
        if (foundListEntry == -1)
        {
            if (ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR Texture ") + package.exportsTable[exportID].objectName +
                             " not found in tree: " + removeEntry.pkgPath + ", skipping...");
                ConsoleSync();
            }
            else
            {
                ConsoleWrite(QString("Error: Texture ") + package.exportsTable[exportID].objectName +
                             " not found in package: " + removeEntry.pkgPath + ", skipping...\n");
            }
            continue;
        }

        MatchedTexture m = textures[foundTextureEntry].list[foundListEntry];
        if (m.linkToMaster != -1)
        {
            if (phase == 1)
            {
                continue;
            }

            const MatchedTexture& foundMasterTex = textures[foundTextureEntry].list[m.linkToMaster];
            if (texture.mipMapsList.count() != foundMasterTex.masterDataOffset.count())
            {
                if (ipc)
                {
                    ConsoleWrite(QString("[IPC]ERROR Texture ") + package.exportsTable[exportID].objectName + " in package: " + foundMasterTex.path + " has wrong reference, skipping...");
                    ConsoleSync();
                }
                else
                {
                    ConsoleWrite(QString("Error: Texture ") + package.exportsTable[exportID].objectName + " in package: " + foundMasterTex.path + " has wrong reference, skipping...\n");
                }
                continue;
            }
            for (int t = 0; t < texture.mipMapsList.count(); t++)
            {
                Texture::TextureMipMap mipmap = texture.mipMapsList[t];
                if (mipmap.storageType == Texture::StorageTypes::extLZO ||
                    mipmap.storageType == Texture::StorageTypes::extZlib ||
                    mipmap.storageType == Texture::StorageTypes::extUnc)
                {
                    mipmap.dataOffset = foundMasterTex.masterDataOffset[t];
                    texture.mipMapsList[t] = mipmap;
                }
            }
        }

        uint packageDataOffset;
        {
            MemoryStream newData;
            ByteBuffer buffer = texture.getProperties().toArray();
            newData.WriteFromBuffer(buffer);
            buffer.Free();
            packageDataOffset = package.exportsTable[exportID].getDataOffset() + (uint)newData.Position();
            buffer = texture.toArray(packageDataOffset);
            newData.WriteFromBuffer(buffer);
            buffer.Free();
            buffer = newData.ToArray();
            package.setExportData(exportID, buffer);
            buffer.Free();
        }

        if (m.linkToMaster == -1)
        {
            if (phase == 2)
                CRASH();
            m.masterDataOffset.clear();
            for (int t = 0; t < texture.mipMapsList.count(); t++)
            {
                m.masterDataOffset.push_back(packageDataOffset + texture.mipMapsList[t].internalOffset);
            }
        }

        m.removeEmptyMips = false;
        textures[foundTextureEntry].list[foundListEntry] = m;
    }
    if (package.SaveToFile(false, false, appendMarker))
    {
        pkgsToMarker.removeOne(package.packagePath);
    }
}

void MipMaps::removeMipMapsME2ME3(QList<FoundTexture> &textures, QStringList &pkgsToMarker,
                                    QStringList &pkgsToRepack, bool ipc, bool repack, bool appendMarker)
{
    int lastProgress = -1;
    QList<RemoveMipsEntry> list;
    prepareListToRemove(textures, list);
    QString path;
    if (GameData::gameType == MeType::ME2_TYPE)
    {
        path = g_GameData->GamePath() + "/BioGame/CookedPC/BIOC_Materials.pcc";
    }
    for (int i = 0; i < list.count(); i++)
    {
        if (path.compare(list[i].pkgPath, Qt::CaseInsensitive) == 0)
            continue;

        if (ipc)
        {
            ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + list[i].pkgPath);
            int newProgress = (i + 1) * 100 / list.count();
            if (lastProgress != newProgress)
            {
                ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
                ConsoleSync();
                lastProgress = newProgress;
            }
        }
        else
        {
            ConsoleWrite("Removing empty mipmaps " +
                         QString::number(i + 1) + "/" +
                         QString::number(list.count()) + " " + list[i].pkgPath);
        }

        Package package{};
        if (package.Open(g_GameData->GamePath() + list[i].pkgPath) != 0)
        {
            if (ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR Issue opening package file: ") + list[i].pkgPath);
                ConsoleSync();
            }
            else
            {
                QString err;
                err += "---- Start --------------------------------------------\n";
                err += "Issue opening package file: " + list[i].pkgPath + "\n";
                err += "---- End ----------------------------------------------\n\n";
                ConsoleWrite(err);
            }
            return;
        }

        removeMipMapsME2ME3(package, list[i], pkgsToMarker, pkgsToRepack, ipc, repack, appendMarker);
    }
}

void MipMaps::removeMipMapsME2ME3(Package &package, RemoveMipsEntry &removeEntry,
                                    QStringList &pkgsToMarker, QStringList &pkgsToRepack,
                                    bool ipc, bool repack, bool appendMarker)
{
    for (int l = 0; l < removeEntry.exportIDs.count(); l++)
    {
        int exportID = removeEntry.exportIDs[l];
        ByteBuffer exportData = package.getExportData(exportID);
        if (exportData.ptr() == nullptr)
        {
            if (ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR Texture has broken export data in package: ") +
                             package.packagePath + "\nExport Id: " + QString::number(exportID + 1) + "\nSkipping...");
                ConsoleSync();
            }
            else
            {
                ConsoleWrite(QString("Error: Texture has broken export data in package: ") +
                             package.packagePath +"\nExport Id: " + QString::number(exportID + 1) + "\nSkipping...");
            }
            continue;
        }
        Texture texture = Texture(package, exportID, exportData, false);
        exportData.Free();
        if (!texture.hasEmptyMips())
        {
            continue;
        }
        texture.removeEmptyMips();
        texture.getProperties().setIntValue("SizeX", texture.mipMapsList.first().width);
        texture.getProperties().setIntValue("SizeY", texture.mipMapsList.first().height);
        texture.getProperties().setIntValue("MipTailBaseIdx", texture.mipMapsList.count() - 1);

        {
            MemoryStream newData;
            ByteBuffer buffer = texture.getProperties().toArray();
            newData.WriteFromBuffer(buffer);
            buffer.Free();
            buffer = texture.toArray(package.exportsTable[exportID].getDataOffset() +
                                     (uint)newData.Position());
            newData.WriteFromBuffer(buffer);
            buffer.Free();
            buffer = newData.ToArray();
            package.setExportData(exportID, buffer);
            buffer.Free();
        }
    }

    if (package.SaveToFile(repack, false, appendMarker))
    {
        if (repack)
            pkgsToRepack.removeOne(package.packagePath);
        pkgsToMarker.removeOne(package.packagePath);
    }
}
