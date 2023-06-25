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
#include <Md5/MD5ModEntries.h>
#include <Md5/MD5BadEntries.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <Helpers/FileStream.h>

namespace {

bool generateModsMd5Entries = false;
bool generateMd5Entries = false;

} // namespace

bool Misc::CheckGameDataAndMods(MeType gameId, Resources &resources)
{
    QString errors;
    QStringList modList;

    bool vanilla = Misc::checkGameFiles(gameId, resources, errors, modList, nullptr, nullptr);

    if (!g_ipc)
    {
        if (modList.count() != 0)
        {
            PERROR("\n------- Detected mods --------\n");
            for (int l = 0; l < modList.count(); l++)
            {
                PERROR(modList[l] + "\n");
            }
            PERROR("------------------------------\n\n");
        }
    }

    if (!vanilla && !g_ipc)
    {
        PERROR("===========================================================================\n");
        PERROR("WARNING: looks like the following file(s) are not vanilla or not recognized\n");
        PERROR("===========================================================================\n\n");
        PERROR(errors);
    }

    return vanilla;
}

bool Misc::checkGameFilesSub(FileStream *fs, QStringList &files, QList<MD5FileEntry> &entries,
                             int &lastProgress, int &progress, int allFilesCount,
                             QString &errors, QStringList &mods,
                             ProgressCallback callback, void *callbackHandle)
{
    int vanilla = true;
    for (int index = 0; index < files.count(); index++)
    {
#ifdef GUI
        QApplication::processEvents();
#endif
        int newProgress = (index + progress) * 100 / allFilesCount;
        if (lastProgress != newProgress)
        {
            lastProgress = newProgress;
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
                ConsoleSync();
            }
        }
        if (!g_ipc && !callback)
        {
            PINFO("Checking: " + files[index] + "\n");
        }
        if (callback)
        {
            callback(callbackHandle, newProgress, "Checking file: " + files[index]);
        }
        QByteArray md5 = calculateMD5(g_GameData->GamePath() + files[index]);
        bool found = false;
        for (int p = 0; p < entries.count(); p++)
        {
            if (memcmp(md5.data(), entries[p].md5, 16) == 0)
            {
                found = true;
                break;
            }
        }
        if (found)
            continue;

        found = false;
        for (int p = 0; p < modsEntriesSize; p++)
        {
            if (memcmp(md5.data(), modsEntries[p].md5, 16) == 0)
            {
                found = true;
                bool found2 = false;
                for (int s = 0; s < mods.count(); s++)
                {
                    if (AsciiStringMatch(mods[s], modsEntries[p].modName))
                    {
                        found2 = true;
                        break;
                    }
                }
                if (!found2)
                    mods.push_back(modsEntries[p].modName);
                break;
            }
        }
        if (found)
            continue;

        found = false;
        for (int p = 0; p < badMODSize; p++)
        {
            if (memcmp(md5.data(), badMOD[p].md5, 16) == 0)
            {
                found = true;
                break;
            }
        }
        if (found)
            continue;

        bool foundFile = false;
        quint8 md5Entry[16];
        QString file = files[index].toLower();
        auto range = std::equal_range(entries.begin(), entries.end(),
                                      file, Resources::ComparePath());
        for (auto it = range.first; it != range.second; it++)
        {
            if (!AsciiStringMatch(it->path, file))
                break;
            if (generateMd5Entries)
            {
                if (memcmp(md5.data(), it->md5, 16) == 0)
                {
                    foundFile = true;
                    break;
                }
            }
            else
            {
                foundFile = true;
                memcpy(md5Entry, it->md5, 16);
                break;
            }
        }
        if (!generateMd5Entries && !foundFile)
            continue;
        if (generateMd5Entries && foundFile)
            continue;

        vanilla = false;

        if (generateModsMd5Entries)
        {
            fs->WriteStringASCII(QString("{\n\"") + files[index] + "\",\n{ ");
            for (int i = 0; i < md5.count(); i++)
            {
                fs->WriteStringASCII(QString::asprintf("0x%02X, ", (quint8)md5[i]));
            }
            fs->WriteStringASCII("},\n\"\",\n},\n");
        }
        if (generateMd5Entries)
        {
            fs->WriteStringASCII(QString("{\n\"") + files[index] + "\",\n{ ");
            for (int i = 0; i < md5.count(); i++)
            {
                fs->WriteStringASCII(QString::asprintf("0x%02X, ", (quint8)md5[i]));
            }
            fs->WriteStringASCII(QString("},\n") +
                                 QString::number(QFile(g_GameData->GamePath() + files[index]).size()) + ",\n},\n");
        }

        if (!generateMd5Entries && !generateModsMd5Entries)
        {
            errors += "File " + files[index] + " has wrong MD5 checksum: ";
            for (int i = 0; i < md5.count(); i++)
            {
                errors += QString::asprintf("%02X", (quint8)md5[i]);
            }
            errors += ", expected: ";
            for (unsigned char i : md5Entry)
            {
                errors += QString::asprintf("%02X", i);
            }
            errors += "\n";
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR ") + files[index]);
                ConsoleSync();
            }
        }
    }
    progress += files.count();
    return vanilla;
}

bool Misc::checkGameFiles(MeType gameType, Resources &resources, QString &errors, QStringList &mods,
                            ProgressCallback callback, void *callbackHandle)
{
    QList<MD5FileEntry> entries;

    if (gameType == MeType::ME1_TYPE)
    {
        entries += resources.entriesME1;
    }
    else if (gameType == MeType::ME2_TYPE)
    {
        entries += resources.entriesME2;
    }
    else if (gameType == MeType::ME3_TYPE)
    {
        entries += resources.entriesME3;
    }

    int progress = 0;
    int allFilesCount = g_GameData->packageFiles.count();
    allFilesCount += g_GameData->tfcFiles.count();
    allFilesCount += g_GameData->othersFiles.count();

    mods.clear();
    FileStream *fs = nullptr;
    if (generateModsMd5Entries)
        fs = new FileStream("MD5ModEntries.cpp", FileMode::Create, FileAccess::WriteOnly);
    if (generateMd5Entries)
        fs = new FileStream("MD5FileEntryME" + QString::number((int)gameType) + ".cpp", FileMode::Create, FileAccess::WriteOnly);

    int lastProgress = -1;
    bool vanilla = true;
    bool state;
    state = checkGameFilesSub(fs, g_GameData->packageFiles, entries, lastProgress, progress, allFilesCount,
                                errors, mods, callback, callbackHandle);
    if (!state)
        vanilla = false;
    state = checkGameFilesSub(fs, g_GameData->tfcFiles, entries, lastProgress, progress, allFilesCount,
                                errors, mods, callback, callbackHandle);
    if (!state)
        vanilla = false;
    state = checkGameFilesSub(fs, g_GameData->othersFiles, entries, lastProgress, progress, allFilesCount,
                                errors, mods, callback, callbackHandle);

    if (generateModsMd5Entries || generateMd5Entries)
    {
        fs->Close();
        delete fs;
    }

    return vanilla;
}
