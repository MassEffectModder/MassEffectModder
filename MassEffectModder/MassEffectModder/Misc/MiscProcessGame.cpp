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

bool Misc::applyModTag(MeType gameId, int MeuitmV, int AlotV)
{
    QString path;
    if (gameId == MeType::ME1_TYPE)
        path = "/BioGame/CookedPC/testVolumeLight_VFX.upk";
    else if (gameId == MeType::ME2_TYPE)
        path = "/BioGame/CookedPC/BIOC_Materials.pcc";
    else if (gameId == MeType::ME3_TYPE)
        path = "/BIOGame/CookedPCConsole/adv_combat_tutorial_xbox_D_Int.afc";

    FileStream fs = FileStream(g_GameData->GamePath() + path, FileMode::Open, FileAccess::ReadWrite);
    fs.Seek(-16, SeekOrigin::End);
    int prevMeuitmV = fs.ReadInt32();
    int prevAlotV = fs.ReadInt32();
    int prevProductV = fs.ReadInt32();
    uint memiTag = fs.ReadUInt32();
    if (memiTag != MEMI_TAG)
        prevProductV = prevAlotV = prevMeuitmV = 0;
    if (MeuitmV != 0)
        prevMeuitmV = MeuitmV;
    if (AlotV != 0)
        prevAlotV = AlotV;
    fs.WriteInt32(prevMeuitmV);
    fs.WriteInt32(prevAlotV);
    fs.WriteInt32((prevProductV & 0xffff0000) | QString(MEM_VERSION).toInt());
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
        if ((newProgress - lastProgress) >= 10)
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

QByteArray Misc::calculateMD5(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly))
        return QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5);
    return QByteArray(16, 0);
}

bool Misc::RemoveMipmaps(MipMaps &mipMaps, QList<TextureMapEntry> &textures,
                         QStringList &pkgsToMarker, bool appendMarker, bool force,
                         ProgressCallback callback, void *callbackHandle)
{
    PINFO("Remove empty mipmaps started...\n");
    Misc::restartStageTimer();
    if (g_ipc)
    {
        ConsoleWrite("[IPC]STAGE_CONTEXT STAGE_REMOVEMIPMAPS");
        ConsoleSync();
    }

    mipMaps.removeMipMaps(textures, pkgsToMarker, appendMarker, force, callback, callbackHandle);

    long elapsed = Misc::elapsedStageTime();
    if (g_ipc)
    {
        ConsoleWrite(QString("[IPC]STAGE_TIMING %1").arg(elapsed));
        ConsoleSync();
    }
    PINFO("Remove empty mipmaps finished.\n\n");

    return true;
}
