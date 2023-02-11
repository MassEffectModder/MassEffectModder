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

void Misc::applyModTag(QStringList &mods)
{
    MemoryStream marker;
    marker.WriteInt32(2); // version 2 for extended marker
    marker.WriteStringUnicode16Null(QString("Mass Effect Modder v%1").arg(MEM_VERSION));
    marker.WriteInt64(QDateTime::currentSecsSinceEpoch());
    marker.WriteInt32(mods.count());
    foreach (QString file, mods)
    {
        marker.WriteByte(0); // user mod file type
        marker.WriteStringUnicode16Null(BaseName(file)); // MEM mod filename
    }
    marker.SeekBegin();
    FileStream fs = FileStream(g_GameData->MainData() + "/SFXTest.pcc", FileMode::Open, FileAccess::ReadWrite);
    fs.SeekEnd();
    fs.CopyFrom(marker, marker.Length());
    fs.WriteInt32(marker.Length());
    fs.WriteUInt32(0xDEADBEEF); // extended marker tag
    fs.WriteInt32(0); // meuitm major version - 0 for MEM
    fs.WriteInt32(0); // alot major version - 0 for MEM
    fs.WriteInt16(0);
    fs.WriteInt16(MEM_VERSION);
    fs.WriteUInt32(MEMI_TAG);
}

bool Misc::CheckForMarkers(ProgressCallback callback, void *callbackHandle)
{
    QStringList packages;
    QString path = g_GameData->MainData() + "/SFXTest.pcc";
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
                callback(callbackHandle, newProgress, "Checking for texture markers");
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
                PERROR(QString("Error: detected texture marker: ") + packages[i] + "\n");
            }
        }
    }

    return true;
}

bool Misc::MarkersPresent(ProgressCallback callback, void *callbackHandle)
{
    QString path = g_GameData->MainData() + "/SFXTest.pcc";
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
                callback(callbackHandle, newProgress, "Checking for texture markers");
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
            PERROR("Error: Detected incompatible mods:\n");
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

bool Misc::detectMod(MeType gameType)
{
	g_GameData->Init(gameType);
	if (!Misc::CheckGamePath())
		return false;
	QString marker = g_GameData->MainData() + "/SFXTest.pcc";
	if (!QFile::exists(marker))
		return false;
	FileStream fs = FileStream(marker, FileMode::Open, FileAccess::ReadOnly);
    fs.Seek(-4, SeekOrigin::End);
    auto tag = fs.ReadUInt32();
    return tag == MEMI_TAG;
}

void Misc::AddMarkers(QStringList &pkgsToMarker,
                      ProgressCallback callback, void *callbackHandle)
{
    PINFO("Adding texture markers started...\n");
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
                callback(callbackHandle, newProgress, "Adding texture markers");
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
    PINFO("Adding texture markers finished.\n\n");
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
