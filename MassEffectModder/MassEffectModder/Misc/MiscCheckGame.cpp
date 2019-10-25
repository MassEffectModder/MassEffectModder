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

#include <Misc/Misc.h>
#include <GameData/GameData.h>
#include <Md5/MD5ModEntries.h>
#include <Md5/MD5BadEntries.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <Helpers/FileStream.h>

static bool generateModsMd5Entries = false;
static bool generateMd5Entries = false;

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

bool Misc::checkGameFiles(MeType gameType, Resources &resources, QString &errors,
                          QStringList &mods, ProgressCallback callback, void *callbackHandle)
{
    bool vanilla = true;
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
    allFilesCount += g_GameData->sfarFiles.count();
    allFilesCount += g_GameData->tfcFiles.count();
    allFilesCount += g_GameData->coalescedFiles.count();
    allFilesCount += g_GameData->afcFiles.count();
    allFilesCount += g_GameData->bikFiles.count();

    mods.clear();
    FileStream *fs;
    if (generateModsMd5Entries)
        fs = new FileStream("MD5ModEntries.cpp", FileMode::Create, FileAccess::WriteOnly);
    if (generateMd5Entries)
        fs = new FileStream("MD5FileEntryME" + QString::number((int)gameType) + ".cpp", FileMode::Create, FileAccess::WriteOnly);

    int lastProgress = -1;
    for (int l = 0; l < g_GameData->packageFiles.count(); l++)
    {
#ifdef GUI
        QApplication::processEvents();
#endif
        int newProgress = (l + progress) * 100 / allFilesCount;
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
                callback(callbackHandle, newProgress, "Checking");
            }
        }
        if (!g_ipc && !callback)
        {
            PINFO("Checking: " + g_GameData->packageFiles[l] + "\n");
        }
        QByteArray md5 = calculateMD5(g_GameData->GamePath() + g_GameData->packageFiles[l]);
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
                    if (mods[s].compare(modsEntries[p].modName) == 0)
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

        bool foundPkg = false;
        quint8 md5Entry[16];
        QString package = g_GameData->packageFiles[l].toLower();
        auto range = std::equal_range(entries.begin(), entries.end(),
                                      package, Resources::ComparePath());
        for (auto it = range.first; it != range.second; it++)
        {
            if (it->path.compare(package, Qt::CaseSensitive) != 0)
                break;
            if (generateMd5Entries)
            {
                if (memcmp(md5.data(), it->md5, 16) == 0)
                {
                    foundPkg = true;
                    break;
                }
            }
            else
            {
                foundPkg = true;
                memcpy(md5Entry, it->md5, 16);
                break;
            }
        }
        if (!generateMd5Entries && !foundPkg)
            continue;
        if (generateMd5Entries && foundPkg)
            continue;

        vanilla = false;

        if (generateModsMd5Entries)
        {
            fs->WriteStringASCII(QString("{\n\"") + g_GameData->packageFiles[l] + "\",\n{ ");
            for (int i = 0; i < md5.count(); i++)
            {
                fs->WriteStringASCII(QString().sprintf("0x%02X, ", (quint8)md5[i]));
            }
            fs->WriteStringASCII("},\n\"\",\n},\n");
        }
        if (generateMd5Entries)
        {
            fs->WriteStringASCII(QString("{\n\"") + g_GameData->packageFiles[l] + "\",\n{ ");
            for (int i = 0; i < md5.count(); i++)
            {
                fs->WriteStringASCII(QString().sprintf("0x%02X, ", (quint8)md5[i]));
            }
            fs->WriteStringASCII(QString("},\n") +
                                 QString::number(QFile(g_GameData->GamePath() + g_GameData->packageFiles[l]).size()) + ",\n},\n");
        }

        if (!generateMd5Entries && !generateModsMd5Entries)
        {
            errors += "File " + g_GameData->packageFiles[l] + " has wrong MD5 checksum: ";
            for (int i = 0; i < md5.count(); i++)
            {
                errors += QString().sprintf("%02X", (quint8)md5[i]);
            }
            errors += ", expected: ";
            for (unsigned char i : md5Entry)
            {
                errors += QString().sprintf("%02X", i);
            }
            errors += "\n";
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR ") + g_GameData->packageFiles[l]);
                ConsoleSync();
            }
        }
    }
    progress += g_GameData->packageFiles.count();

    for (int l = 0; l < g_GameData->sfarFiles.count(); l++)
    {
#ifdef GUI
        QApplication::processEvents();
#endif
        int newProgress = (l + progress) * 100 / allFilesCount;
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
                callback(callbackHandle, newProgress, "Checking");
            }
        }
        if (!g_ipc && !callback)
        {
            PINFO("Checking: " + g_GameData->sfarFiles[l] + "\n");
        }
        QByteArray md5 = calculateMD5(g_GameData->GamePath() + g_GameData->sfarFiles[l]);
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
        int index = -1;
        for (int p = 0; p < entries.count(); p++)
        {
            if (g_GameData->sfarFiles[l].compare(entries[p].path, Qt::CaseInsensitive) == 0)
            {
                index = p;
                break;
            }
        }
        if (index == -1)
            continue;

        vanilla = false;

        if (!generateMd5Entries && !generateModsMd5Entries)
        {
            errors += "File " + g_GameData->sfarFiles[l] + " has wrong MD5 checksum: ";
            for (int i = 0; i < md5.count(); i++)
            {
                errors += QString().sprintf("%02X", (quint8)md5[i]);
            }
            errors += ", expected: ";
            for (unsigned char i : entries[index].md5)
            {
                errors += QString().sprintf("%02X", i);
            }
            errors += "\n";

            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR ") + g_GameData->sfarFiles[l]);
                ConsoleSync();
            }
        }
    }
    progress += g_GameData->sfarFiles.count();

    for (int l = 0; l < g_GameData->tfcFiles.count(); l++)
    {
#ifdef GUI
        QApplication::processEvents();
#endif
        int newProgress = (l + progress) * 100 / allFilesCount;
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
                callback(callbackHandle, newProgress, "Checking");
            }
        }
        if (!g_ipc && !callback)
        {
            PINFO("Checking: " + g_GameData->tfcFiles[l] + "\n");
        }
        QByteArray md5 = calculateMD5(g_GameData->GamePath() + g_GameData->tfcFiles[l]);
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
        int index = -1;
        for (int p = 0; p < entries.count(); p++)
        {
            if (g_GameData->tfcFiles[l].compare(entries[p].path, Qt::CaseInsensitive) == 0)
            {
                index = p;
                break;
            }
        }
        if (index == -1)
            continue;

        vanilla = false;

        if (!generateMd5Entries && !generateModsMd5Entries)
        {
            errors += "File " + g_GameData->tfcFiles[l] + " has wrong MD5 checksum: ";
            for (int i = 0; i < md5.count(); i++)
            {
                errors += QString().sprintf("%02X", (quint8)md5[i]);
            }
            errors += ", expected: ";
            for (unsigned char i : entries[index].md5)
            {
                errors += QString().sprintf("%02X", i);
            }
            errors += "\n";
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR ") + g_GameData->tfcFiles[l]);
                ConsoleSync();
            }
        }
    }
    progress += g_GameData->tfcFiles.count();

    for (int l = 0; l < g_GameData->coalescedFiles.count(); l++)
    {
#ifdef GUI
        QApplication::processEvents();
#endif
        int newProgress = (l + progress) * 100 / allFilesCount;
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
                callback(callbackHandle, newProgress, "Checking");
            }
        }
        if (!g_ipc && !callback)
        {
            PINFO("Checking: " + g_GameData->coalescedFiles[l] + "\n");
        }
        QByteArray md5 = calculateMD5(g_GameData->GamePath() + g_GameData->coalescedFiles[l]);
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
        int index = -1;
        for (int p = 0; p < entries.count(); p++)
        {
            if (g_GameData->coalescedFiles[l].compare(entries[p].path, Qt::CaseInsensitive) == 0)
            {
                index = p;
                break;
            }
        }
        if (index == -1 && !generateMd5Entries && !generateModsMd5Entries)
            continue;

        vanilla = false;

        if (generateModsMd5Entries)
        {
            fs->WriteStringASCII(QString("{\n\"") + g_GameData->coalescedFiles[l] + "\",\n{ ");
            for (int i = 0; i < md5.count(); i++)
            {
                fs->WriteStringASCII(QString().sprintf("0x%02X, ", (quint8)md5[i]));
            }
            fs->WriteStringASCII("},\n\"\",\n},\n");
        }
        if (generateMd5Entries)
        {
            fs->WriteStringASCII(QString("{\n\"") + g_GameData->coalescedFiles[l] + "\",\n{ ");
            for (int i = 0; i < md5.count(); i++)
            {
                fs->WriteStringASCII(QString().sprintf("0x%02X, ", (quint8)md5[i]));
            }
            fs->WriteStringASCII(QString("},\n") +
                                 QString::number(QFile(g_GameData->GamePath() + g_GameData->coalescedFiles[l]).size()) + ",\n},\n");
        }

        if (!generateMd5Entries && !generateModsMd5Entries)
        {
            errors += "File " + g_GameData->coalescedFiles[l] + " has wrong MD5 checksum: ";
            for (int i = 0; i < md5.count(); i++)
            {
                errors += QString().sprintf("%02X", (quint8)md5[i]);
            }
            errors += ", expected: ";
            for (unsigned char i : entries[index].md5)
            {
                errors += QString().sprintf("%02X", i);
            }
            errors += "\n";
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR ") + g_GameData->coalescedFiles[l]);
                ConsoleSync();
            }
        }
    }
    progress += g_GameData->coalescedFiles.count();

    for (int l = 0; l < g_GameData->afcFiles.count(); l++)
    {
#ifdef GUI
        QApplication::processEvents();
#endif
        int newProgress = (l + progress) * 100 / allFilesCount;
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
                callback(callbackHandle, newProgress, "Checking");
            }
        }
        if (!g_ipc && !callback)
        {
            PINFO("Checking: " + g_GameData->afcFiles[l] + "\n");
        }
        QByteArray md5 = calculateMD5(g_GameData->GamePath() + g_GameData->afcFiles[l]);
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
        int index = -1;
        for (int p = 0; p < entries.count(); p++)
        {
            if (g_GameData->afcFiles[l].compare(entries[p].path, Qt::CaseInsensitive) == 0)
            {
                index = p;
                break;
            }
        }
        if (index == -1 && !generateMd5Entries && !generateModsMd5Entries)
            continue;

        vanilla = false;

        if (generateModsMd5Entries)
        {
            fs->WriteStringASCII(QString("{\n\"") + g_GameData->afcFiles[l] + "\",\n{ ");
            for (int i = 0; i < md5.count(); i++)
            {
                fs->WriteStringASCII(QString().sprintf("0x%02X, ", (quint8)md5[i]));
            }
            fs->WriteStringASCII("},\n\"\",\n},\n");
        }
        if (generateMd5Entries)
        {
            fs->WriteStringASCII(QString("{\n\"") + g_GameData->afcFiles[l] + "\",\n{ ");
            for (int i = 0; i < md5.count(); i++)
            {
                fs->WriteStringASCII(QString().sprintf("0x%02X, ", (quint8)md5[i]));
            }
            fs->WriteStringASCII(QString("},\n") +
                                 QString::number(QFile(g_GameData->GamePath() + g_GameData->afcFiles[l]).size()) + ",\n},\n");
        }

        if (!generateMd5Entries && !generateModsMd5Entries)
        {
            errors += "File " + g_GameData->afcFiles[l] + " has wrong MD5 checksum: ";
            for (int i = 0; i < md5.count(); i++)
            {
                errors += QString().sprintf("%02X", (quint8)md5[i]);
            }
            errors += ", expected: ";
            for (unsigned char i : entries[index].md5)
            {
                errors += QString().sprintf("%02X", i);
            }
            errors += "\n";
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR ") + g_GameData->afcFiles[l]);
                ConsoleSync();
            }
        }
    }
    progress += g_GameData->afcFiles.count();

    for (int l = 0; l < g_GameData->bikFiles.count(); l++)
    {
#ifdef GUI
        QApplication::processEvents();
#endif
        int newProgress = (l + progress) * 100 / allFilesCount;
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
                callback(callbackHandle, newProgress, "Checking");
            }
        }
        if (!g_ipc && !callback)
        {
            PINFO("Checking: " + g_GameData->bikFiles[l] + "\n");
        }
        QByteArray md5 = calculateMD5(g_GameData->GamePath() + g_GameData->bikFiles[l]);
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
        int index = -1;
        for (int p = 0; p < entries.count(); p++)
        {
            if (g_GameData->bikFiles[l].compare(entries[p].path, Qt::CaseInsensitive) == 0)
            {
                index = p;
                break;
            }
        }
        if (index == -1 && !generateMd5Entries && !generateModsMd5Entries)
            continue;

        vanilla = false;

        if (generateModsMd5Entries)
        {
            fs->WriteStringASCII(QString("{\n\"") + g_GameData->bikFiles[l] + "\",\n{ ");
            for (int i = 0; i < md5.count(); i++)
            {
                fs->WriteStringASCII(QString().sprintf("0x%02X, ", (quint8)md5[i]));
            }
            fs->WriteStringASCII("},\n\"\",\n},\n");
        }
        if (generateMd5Entries)
        {
            fs->WriteStringASCII(QString("{\n\"") + g_GameData->bikFiles[l] + "\",\n{ ");
            for (int i = 0; i < md5.count(); i++)
            {
                fs->WriteStringASCII(QString().sprintf("0x%02X, ", (quint8)md5[i]));
            }
            fs->WriteStringASCII(QString("},\n") +
                                 QString::number(QFile(g_GameData->GamePath() + g_GameData->bikFiles[l]).size()) + ",\n},\n");
        }

        if (!generateMd5Entries && !generateModsMd5Entries)
        {
            errors += "File " + g_GameData->bikFiles[l] + " has wrong MD5 checksum: ";
            for (int i = 0; i < md5.count(); i++)
            {
                errors += QString().sprintf("%02X", (quint8)md5[i]);
            }
            errors += ", expected: ";
            for (unsigned char i : entries[index].md5)
            {
                errors += QString().sprintf("%02X", i);
            }
            errors += "\n";
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR ") + g_GameData->bikFiles[l]);
                ConsoleSync();
            }
        }
    }
    progress += g_GameData->bikFiles.count();

    if (generateModsMd5Entries || generateMd5Entries)
    {
        fs->Close();
        delete fs;
    }

    return vanilla;
}
