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

#include <Misc/Misc.h>
#include <GameData/GameData.h>
#include <GameData/TOCFile.h>
#include <MipMaps/MipMaps.h>
#include <Wrappers.h>
#include <Md5/MD5ModEntries.h>
#include <Md5/MD5BadEntries.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <Helpers/FileStream.h>

bool Misc::ApplyLAAForME1Exe()
{
    if (QFile(g_GameData->GameExePath()).exists())
    {
        FileStream fs = FileStream(g_GameData->GameExePath(), FileMode::Open, FileAccess::ReadWrite);
        {
            fs.JumpTo(0x3C); // jump to offset of COFF header
            uint offset = fs.ReadUInt32() + 4; // skip PE signature too
            fs.JumpTo(offset + 0x12); // jump to flags entry
            ushort flag = fs.ReadUInt16(); // read flags
            if ((flag & 0x20) != 0x20) // check for LAA flag
            {
                PINFO(QString("Patching ME1 for LAA: ") + g_GameData->GameExePath() + "\n");
                flag |= 0x20;
                fs.Skip(-2);
                fs.WriteUInt16(flag); // write LAA flag
            }
            else
            {
                PINFO(QString("File already has LAA flag enabled: ") + g_GameData->GameExePath() + "\n");
            }
        }
        return true;
    }

    PERROR(QString("File not found: ") + g_GameData->GameExePath() + "\n");
    return false;
}

bool Misc::ChangeProductNameForME1Exe()
{
    if (QFile(g_GameData->GameExePath()).exists())
    {
        // search for "ProductName Mass Effect"
        quint8 pattern[] = { 0x50, 0, 0x72, 0, 0x6F, 0, 0x64, 0, 0x75, 0, 0x63, 0, 0x74, 0, 0x4E, 0, 0x61, 0, 0x6D, 0, 0x65, 0, 0, 0, 0, 0,
                           0x4D, 0, 0x61, 0, 0x73, 0, 0x73, 0, 0x20, 0, 0x45, 0, 0x66, 0, 0x66, 0, 0x65, 0, 0x63, 0, 0x74, 0 };
        FileStream fs = FileStream(g_GameData->GameExePath(), FileMode::Open, FileAccess::ReadWrite);
        ByteBuffer buffer = fs.ReadAllToBuffer();
        quint8 *ptr = buffer.ptr();
        int pos = -1;
        for (qint64 i = 0; i < buffer.size(); i++)
        {
            if (ptr[i] == pattern[0])
            {
                bool found = true;
                for (unsigned long long l = 1; l < sizeof (pattern); l++)
                {
                    if (ptr[i + l] != pattern[l])
                    {
                        found = false;
                        break;
                    }
                }
                if (found)
                {
                    pos = i;
                    break;
                }
            }
        }
        if (pos != -1)
        {
            // replace to "Mass_Effect"
            fs.JumpTo(pos + 34);
            fs.WriteByte(0x5f);
            PINFO(QString("Patching ME1 for Product Name: ") + g_GameData->GameExePath() + "\n");
        }
        else
        {
            PINFO(QString("Specific Product Name not found or already changed: ") +
                         g_GameData->GameExePath() + "\n");
        }
        buffer.Free();
        return true;
    }

    PERROR(QString("File not found: ") + g_GameData->GameExePath() + "\n");
    return false;
}

bool Misc::ChangeRegKeyForME1Exe()
{
    if (QFile(g_GameData->GameExePath()).exists())
    {
        // search for "enableLocalPhysXCore"
        quint8 pattern[] = { 0x65, 0, 0x6E, 0, 0x61, 0, 0x62, 0, 0x6C, 0, 0x65, 0,
                             0x4C, 0, 0x6F, 0, 0x63, 0, 0x61, 0, 0x6C, 0, 0x50, 0,
                             0x68, 0, 0x79, 0, 0x73, 0, 0x58, 0, 0x43, 0, 0x6F, 0,
                             0x72, 0, 0x65, 0 };
        FileStream fs = FileStream(g_GameData->GameExePath(), FileMode::Open, FileAccess::ReadWrite);
        ByteBuffer buffer = fs.ReadAllToBuffer();
        quint8 *ptr = buffer.ptr();
        int pos = -1;
        for (qint64 i = 0; i < buffer.size(); i++)
        {
            if (ptr[i] == pattern[0])
            {
                bool found = true;
                for (unsigned long long l = 1; l < sizeof (pattern); l++)
                {
                    if (ptr[i + l] != pattern[l])
                    {
                        found = false;
                        break;
                    }
                }
                if (found)
                {
                    pos = i;
                    break;
                }
            }
        }
        if (pos != -1)
        {
            // replace to "enableLocalPhysXCor_"
            fs.JumpTo(pos + 38);
            fs.WriteByte(0x5f);
            PINFO(QString("Patching ME1 'enableLocalPhysXCore': ") + g_GameData->GameExePath() + "\n");
        }
        else
        {
            PINFO(QString("Specific 'enableLocalPhysXCore' not found or already changed: ") +
                         g_GameData->GameExePath() + "\n");
        }
        buffer.Free();
        return true;
    }

    PERROR(QString("File not found: ") + g_GameData->GameExePath() + "\n");
    return false;
}

bool Misc::applyModTag(MeType gameId)
{
    QString path;
    if (gameId == MeType::ME1_TYPE)
        path = "/BioGame/CookedPC/testVolumeLight_VFX.upk";
    else if (gameId == MeType::ME2_TYPE)
        path = "/BioGame/CookedPC/BIOC_Materials.pcc";
    else if (gameId == MeType::ME3_TYPE)
        path = "/BIOGame/CookedPCConsole/adv_combat_tutorial_xbox_D_Int.afc";

    MemoryStream marker;
    marker.WriteInt32(2); // version 2 for extended marker
    marker.WriteStringUnicode16Null(QString("Mass Effect Modder v%1").arg(MEM_VERSION));
    marker.WriteInt64(QDateTime::currentSecsSinceEpoch());
    marker.WriteInt32(0); // number of mods - 0 for MEM
    marker.SeekBegin();
    FileStream fs = FileStream(g_GameData->GamePath() + path, FileMode::Open, FileAccess::ReadWrite);
    fs.SeekEnd();
    fs.CopyFrom(marker, marker.Length());
    fs.WriteInt32(marker.Length());
    fs.WriteUInt32(0xDEADBEEF); // extended marker tag
    fs.WriteInt32(0); // meuitm major version - 0 for MEM
    fs.WriteInt32(0); // alot major version - 0 for MEM
    fs.WriteInt32(QString(MEM_VERSION).toInt());
    fs.WriteUInt32(MEMI_TAG);

    return true;
}

bool Misc::CheckForMarkers(ProgressCallback callback, void *callbackHandle)
{
    QString path;
    if (GameData::gameType == MeType::ME1_TYPE)
        path = "/BioGame/CookedPC/testVolumeLight_VFX.upk";
    else if (GameData::gameType == MeType::ME2_TYPE)
        path = "/BioGame/CookedPC/BIOC_Materials.pcc";

    QStringList packages;
    for (int i = 0; i < g_GameData->packageFiles.count(); i++)
    {
        if (path.length() != 0 && AsciiStringMatchCaseIgnore(g_GameData->packageFiles[i], path))
            continue;
        packages.push_back(g_GameData->packageFiles[i]);
    }

    int lastProgress = -1;
    for (int i = 0; i < packages.count(); i++)
    {
        int newProgress = (i + 1) * 100 / packages.count();
        if ((newProgress - lastProgress) >= 5)
        {
            lastProgress = newProgress;
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
                ConsoleSync();
            }
            else if (callback)
            {
                callback(callbackHandle, newProgress, "Checking markers");
            }
        }

        FileStream fs = FileStream(g_GameData->GamePath() + packages[i], FileMode::Open, FileAccess::ReadOnly);
        fs.Seek(-MEMMarkerLength, SeekOrigin::End);
        QString marker;
        fs.ReadStringASCII(marker, MEMMarkerLength);
        if (marker == QString(MEMendFileMarker))
        {
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR_FILEMARKER_FOUND ") + packages[i]);
                ConsoleSync();
            }
            else
            {
                PERROR(QString("Error: detected marker: ") + packages[i] + "\n");
            }
        }
    }

    return true;
}

bool Misc::MarkersPresent(ProgressCallback callback, void *callbackHandle)
{
    QString path;
    if (GameData::gameType == MeType::ME1_TYPE)
        path = "/BioGame/CookedPC/testVolumeLight_VFX.upk";
    else if (GameData::gameType == MeType::ME2_TYPE)
        path = "/BioGame/CookedPC/BIOC_Materials.pcc";

    QStringList packages;
    for (int i = 0; i < g_GameData->packageFiles.count(); i++)
    {
        if (path.length() != 0 && AsciiStringMatchCaseIgnore(g_GameData->packageFiles[i], path))
            continue;
        packages.push_back(g_GameData->packageFiles[i]);
    }

    int lastProgress = -1;
    for (int i = 0; i < packages.count(); i++)
    {
        int newProgress = (i + 1) * 100 / packages.count();
        if ((newProgress - lastProgress) >= 5)
        {
            lastProgress = newProgress;
            if (callback)
            {
                callback(callbackHandle, newProgress, "Checking markers");
            }
        }

        FileStream fs = FileStream(g_GameData->GamePath() + packages[i], FileMode::Open, FileAccess::ReadOnly);
        fs.Seek(-MEMMarkerLength, SeekOrigin::End);
        QString marker;
        fs.ReadStringASCII(marker, MEMMarkerLength);
        if (marker == QString(MEMendFileMarker))
        {
            return true;
        }
    }

    return false;
}

void Misc::detectBrokenMod(QStringList &mods)
{
    for (int l = 0; l < badMODSize; l++)
    {
        QString path = g_GameData->GamePath() + badMOD[l].path;
        if (!QFile(path).exists())
            continue;
        QByteArray md5 = calculateMD5(path);
        if (memcmp(md5.data(), badMOD[l].md5, 16) == 0)
        {
            bool found = false;
            for (int s = 0; s < mods.count(); s++)
            {
                if (AsciiStringMatch(mods[s], badMOD[l].modName))
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                mods.push_back(badMOD[l].modName);
        }
    }
}

bool Misc::ReportBadMods()
{
    QStringList badMods;
    Misc::detectBrokenMod(badMods);
    if (badMods.count() != 0)
    {
        if (!g_ipc)
            PERROR("Error: Detected not compatible mods:\n");
        for (int l = 0; l < badMods.count(); l++)
        {
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR ") + badMods[l]);
                ConsoleSync();
            }
            else
            {
                PERROR(badMods[l] + "\n");
            }
        }
    }

    return true;
}

bool Misc::ReportMods()
{
    QStringList mods;
    Misc::detectMods(mods);
    if (mods.count() != 0)
    {
        if (!g_ipc)
            PINFO("Detected mods:\n");
        for (int l = 0; l < mods.count(); l++)
        {
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]MOD ") + mods[l]);
                ConsoleSync();
            }
            else
            {
                PINFO(mods[l] + "\n");
            }
        }
    }

    return true;
}

bool Misc::detectMod(MeType gameId)
{
    QString path;
    if (gameId == MeType::ME1_TYPE)
        path = "/BioGame/CookedPC/testVolumeLight_VFX.upk";
    else if (gameId == MeType::ME2_TYPE)
        path = "/BioGame/CookedPC/BIOC_Materials.pcc";
    else
        path = "/BIOGame/CookedPCConsole/adv_combat_tutorial_xbox_D_Int.afc";

    FileStream fs = FileStream(g_GameData->GamePath() + path, FileMode::Open, FileAccess::ReadOnly);
    fs.Seek(-4, SeekOrigin::End);
    auto tag = fs.ReadUInt32();
    return tag == MEMI_TAG;
}

void Misc::AddMarkers(QStringList &pkgsToMarker,
                      ProgressCallback callback, void *callbackHandle)
{
    PINFO("Adding markers started...\n");
    Misc::restartStageTimer();
    if (g_ipc)
    {
        ConsoleWrite("[IPC]STAGE_CONTEXT STAGE_MARKERS");
        ConsoleSync();
    }
    int lastProgress = -1;
    for (int i = 0; i < pkgsToMarker.count(); i++)
    {
        int newProgress = (i + 1) * 100 / pkgsToMarker.count();
        if ((newProgress - lastProgress) >= 5)
        {
            lastProgress = newProgress;
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
                ConsoleSync();
            }
            else if (callback)
            {
                callback(callbackHandle, newProgress, "Adding markers");
            }
        }
        PDEBUG(QString("Misc::AddMarkers File: ") + pkgsToMarker[i] + "\n");
        FileStream fs = FileStream(g_GameData->GamePath() + pkgsToMarker[i], FileMode::Open, FileAccess::ReadWrite);
        fs.Seek(-MEMMarkerLength, SeekOrigin::End);
        QString marker;
        fs.ReadStringASCII(marker, MEMMarkerLength);
        QString str(MEMendFileMarker);
        if (marker != str)
        {
            fs.SeekEnd();
            fs.WriteStringASCII(str);
        }
    }
    long elapsed = Misc::elapsedStageTime();
    if (g_ipc)
    {
        ConsoleWrite(QString("[IPC]STAGE_TIMING %1").arg(elapsed));
        ConsoleSync();
    }
    PINFO("Adding markers finished.\n\n");
}


void Misc::detectMods(QStringList &mods)
{
    for (int l = 0; l < modsEntriesSize; l++)
    {
        QString path = g_GameData->GamePath() + modsEntries[l].path;
        if (!QFile(path).exists())
            continue;
        QByteArray md5 = calculateMD5(path);
        if (memcmp(md5.data(), modsEntries[l].md5, 16) == 0)
        {
            bool found = false;
            for (int s = 0; s < mods.count(); s++)
            {
                if (AsciiStringMatch(mods[s], modsEntries[l].modName))
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                mods.push_back(modsEntries[l].modName);
        }
    }
}

void Misc::RepackME23(MeType gameId, bool appendMarker, QStringList &pkgsToRepack,
                      ProgressCallback callback, void *callbackHandle)
{
    PINFO("Repack started...\n");
    Misc::restartStageTimer();
    if (g_ipc)
    {
        ConsoleWrite("[IPC]STAGE_CONTEXT STAGE_REPACK");
        ConsoleSync();
    }

    if (gameId == MeType::ME2_TYPE)
        pkgsToRepack.removeOne("/BioGame/CookedPC/BIOC_Materials.pcc");
    int lastProgress = -1;
    for (int i = 0; i < pkgsToRepack.count(); i++)
    {
#ifdef GUI
        QApplication::processEvents();
#endif
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + pkgsToRepack[i]);
            ConsoleSync();
        }
        else
        {
            PINFO(QString("Repack " + QString::number(i + 1) + "/" +
                                 QString::number(pkgsToRepack.count()) +
                                 " ") + pkgsToRepack[i] + "\n");
        }
        int newProgress = i * 100 / pkgsToRepack.count();
        if (lastProgress != newProgress)
        {
            lastProgress = newProgress;
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
                ConsoleSync();
            }
        }
        if (callback)
        {
            callback(callbackHandle, newProgress, QString("Repacking package: ") + pkgsToRepack[i]);
        }
        auto package = new Package();
        package->Open(g_GameData->GamePath() + pkgsToRepack[i], true);
        if (!package->getCompressedFlag() || (package->getCompressedFlag() &&
             package->compressionType != Package::CompressionType::Zlib))
        {
            delete package;
            package = new Package();
            package->Open(g_GameData->GamePath() + pkgsToRepack[i]);
            package->SaveToFile(true, false, appendMarker);
        }
        delete package;
    }
    long elapsed = Misc::elapsedStageTime();
    if (g_ipc)
    {
        ConsoleWrite(QString("[IPC]STAGE_TIMING %1").arg(elapsed));
        ConsoleSync();
    }
    PINFO("Repack finished.\n\n");
}

QByteArray Misc::calculateMD5(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly))
        return QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5);
    return QByteArray(16, 0);
}

void Misc::Repack(MeType gameId, ProgressCallback callback, void *callbackHandle)
{
    QStringList pkgsToRepack;
    for (int i = 0; i < g_GameData->packageFiles.count(); i++)
    {
        pkgsToRepack.push_back(g_GameData->packageFiles[i]);
    }
    Misc::RepackME23(gameId, false, pkgsToRepack, callback, callbackHandle);
    if (GameData::gameType == MeType::ME3_TYPE)
        TOCBinFile::UpdateAllTOCBinFiles();
}

bool Misc::RemoveMipmaps(MipMaps &mipMaps, QList<TextureMapEntry> &textures,
                         QStringList &pkgsToMarker, QStringList &pkgsToRepack,
                         bool repack, bool appendMarker, bool force,
                         ProgressCallback callback, void *callbackHandle)
{
    PINFO("Remove empty mipmaps started...\n");
    Misc::restartStageTimer();
    if (g_ipc)
    {
        ConsoleWrite("[IPC]STAGE_CONTEXT STAGE_REMOVEMIPMAPS");
        ConsoleSync();
    }

    mipMaps.removeMipMaps(1, textures, pkgsToMarker, pkgsToRepack, repack, appendMarker, force,
                          callback, callbackHandle);
    if (GameData::gameType == MeType::ME1_TYPE)
        mipMaps.removeMipMaps(2, textures, pkgsToMarker, pkgsToRepack, repack, appendMarker, force,
                              callback, callbackHandle);

    long elapsed = Misc::elapsedStageTime();
    if (g_ipc)
    {
        ConsoleWrite(QString("[IPC]STAGE_TIMING %1").arg(elapsed));
        ConsoleSync();
    }
    PINFO("Remove empty mipmaps finished.\n\n");

    return true;
}

bool Misc::unpackSFARisNeeded()
{
    if (QDir(g_GameData->DLCData()).exists())
    {
        QStringList DLCs = QDir(g_GameData->DLCData(), "DLC_*", QDir::NoSort, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryList();
        foreach (QString DLCDir, DLCs)
        {
            QStringList packages;
            QDirIterator iterator(g_GameData->DLCData() + "/" + DLCDir + g_GameData->DLCDataSuffix(), QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
            bool isValid = false;
            bool unpacked = false;
            while (iterator.hasNext())
            {
                iterator.next();
                if (iterator.filePath().endsWith("Default.sfar", Qt::CaseInsensitive))
                    isValid = true;
                if (iterator.filePath().endsWith("Mount.dlc", Qt::CaseInsensitive))
                    unpacked = true;
            }
            if (isValid && !unpacked)
                return true;
        }
    }

    return false;
}
