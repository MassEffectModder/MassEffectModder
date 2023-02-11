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
#include <GameData/UserSettings.h>
#include <MipMaps/MipMaps.h>
#include <Wrappers.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <Helpers/FileStream.h>

bool Misc::applyMods(QStringList &files, QList<TextureMapEntry> &textures,
                     QStringList &pkgsToMarker,
                     MipMaps &mipMaps, bool appendMarker,
                     bool verify, int cacheAmount,
                     ProgressCallback callback, void *callbackHandle)
{
    bool status = true;
    int currentNumberOfTotalMods = 1;

    QList<ModEntry> modsToReplace;

    Misc::restartStageTimer();

    for (int i = 0; i < files.count(); i++)
    {
        if (QFile(files[i]).size() == 0)
        {
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR MEM mod file has 0 length: ") + files[i]);
                ConsoleSync();
            }
            else
            {
                PERROR(QString("MEM mod file is 0 bytes in length: ") + files[i] + "\n");
            }
            continue;
        }
        FileStream fs = FileStream(files[i], FileMode::Open, FileAccess::ReadOnly);
        if (!Misc::CheckMEMHeader(fs, files[i]))
            continue;
        fs.JumpTo(fs.ReadInt64());
        fs.SkipInt32();
		fs.SkipInt32();
	}

    for (int i = 0; i < files.count(); i++)
    {
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + files[i]);
            ConsoleSync();
        }
        else
        {
            PINFO(QString("Preparing mod: ") + QString::number(i + 1) + " of " +
                         QString::number(files.count()) + " - " + BaseName(files[i]) + "\n");
        }

        FileStream fs = FileStream(files[i], FileMode::Open, FileAccess::ReadOnly);
        if (!Misc::CheckMEMHeader(fs, files[i]))
            continue;

        if (!Misc::CheckMEMGameVersion(fs, files[i], GameData::gameType))
            continue;

        int numFiles = fs.ReadInt32();
        QList<FileMod> modFiles{};
        for (int k = 0; k < numFiles; k++)
        {
            FileMod fileMod{};
            fileMod.tag = fs.ReadUInt32();
            fs.ReadStringASCIINull(fileMod.name);
            fileMod.offset = fs.ReadInt64();
            fileMod.size = fs.ReadInt64();
            fileMod.flags = fs.ReadInt64();
            modFiles.push_back(fileMod);
        }
        numFiles = modFiles.count();
        for (int l = 0; l < numFiles; l++, currentNumberOfTotalMods++)
        {
            quint32 crc = 0, textureFlags = 0;
            fs.JumpTo(modFiles[l].offset);
            long size = modFiles[l].size;
            if (modFiles[l].tag == FileTextureTag ||
                modFiles[l].tag == FileMovieTextureTag)
            {
                textureFlags = fs.ReadUInt32();
                crc = fs.ReadUInt32();
            }
            else
            {
                if (g_ipc)
                {
                    ConsoleWrite(QString("[IPC]ERROR Unknown tag for file: ") + modFiles[l].name);
                    ConsoleSync();
                }
                else
                {
                    PERROR(QString("Unknown tag for file: ") + modFiles[l].name + "\n");
                }
                continue;
            }

            if (modFiles[l].tag == FileTextureTag ||
                modFiles[l].tag == FileMovieTextureTag)
            {
                TextureMapEntry f = Misc::FoundTextureInTheMap(textures, crc);
                if (f.crc != 0)
                {
                    ModEntry entry{};
                    entry.textureCrc = f.crc;
                    entry.textureName = f.name;
                    if (textureFlags & ModTextureFlags::MarkToConvert)
                        entry.markConvert = true;
                    entry.memPath = files[i];
                    entry.memEntryOffset = fs.Position();
                    entry.memEntrySize = size;
                    entry.injectedTexture = nullptr;
                    modsToReplace.push_back(entry);
                }
                else
                {
                    PINFO(QString("Texture skipped. Texture ") + modFiles[l].name +
                          " is not present in your game setup.\n");
                }
            }
        }
    }

    mipMaps.replaceModsFromList(textures, pkgsToMarker, modsToReplace,
                                appendMarker, verify, cacheAmount,
                                callback, callbackHandle);

    PINFO("Processing textures finished.\n\n");

    long elapsed = Misc::elapsedStageTime();
    if (g_ipc)
    {
        ConsoleWrite(QString("[IPC]STAGE_TIMING %1").arg(elapsed));
        ConsoleSync();
    }

    if (verify)
    {
        Misc::restartStageTimer();
        if (g_ipc)
        {
            ConsoleWrite("[IPC]STAGE_CONTEXT STAGE_VERIFYTEXTURES");
            ConsoleSync();
        }
        else
        {
            PINFO("\nVerifying installed textures started...\n");
        }
        status = mipMaps.VerifyTextures(textures, callback, callbackHandle);
        if (!g_ipc)
            PINFO("\nVerifying installed textures finished.\n");
        long elapsed = Misc::elapsedStageTime();
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]STAGE_TIMING %1").arg(elapsed));
            ConsoleSync();
        }
    }

    return status;
}

bool Misc::InstallMods(MeType gameId, Resources &resources, QStringList &modFiles,
                       bool guiInstallerMode, bool alotInstallerMode,
                       bool skipMarkers, bool verify, int cacheAmount,
                       ProgressCallback callback, void *callbackHandle)
{
    MipMaps mipMaps;
    QStringList pkgsToRepack;
    QStringList pkgsToMarker;

    if (!alotInstallerMode && !guiInstallerMode)
    {
        bool writeAccess = Misc::CheckAndCorrectAccessToGame();
        if (!writeAccess)
        {
            PERROR("Error: The current user does not have write access to game folders.\n");
            return false;
        }

        QStringList badMods;
        Misc::detectBrokenMod(badMods);
        if (badMods.count() != 0)
        {
            PERROR("Error: Detected incompatible mods:\n");
            for (int l = 0; l < badMods.count(); l++)
            {
                PERROR(badMods[l] + "\n");
            }
            return false;
        }
    }

    Misc::startTimer();

	bool modded = detectMod(gameId);
    if (g_ipc)
    {
        if (!modded)
        {
            ConsoleWrite("[IPC]STAGE_ADD STAGE_PRESCAN");
            ConsoleWrite("[IPC]STAGE_ADD STAGE_SCAN");
        }
        ConsoleWrite("[IPC]STAGE_ADD STAGE_INSTALLTEXTURES");
        if (verify)
            ConsoleWrite("[IPC]STAGE_ADD STAGE_VERIFYTEXTURES");
        if (!skipMarkers && !modded)
            ConsoleWrite("[IPC]STAGE_ADD STAGE_MARKERS");
        ConsoleSync();
    }

    QList<TextureMapEntry> textures;

    if (!modded)
    {
        if (callback)
            callback(callbackHandle, -1, "Preparing to scan textures...");
        for (int i = 0; i < g_GameData->packageFiles.count(); i++)
        {
            pkgsToMarker.push_back(g_GameData->packageFiles[i]);
        }
        pkgsToMarker.removeOne(g_GameData->MainData() + "/SFXTest.pcc");

        PINFO("Scan textures started...\n");
        if (!TreeScan::PrepareListOfTextures(gameId, resources, textures, true,
                                        callback, callbackHandle))
        {
            PERROR("Failed to scan textures!\n");
            return false;
        }
        PINFO("Scan textures finished.\n\n");
    }


    PINFO("Texture installation started...\n");
    if (g_ipc)
    {
        ConsoleWrite("[IPC]STAGE_CONTEXT STAGE_INSTALLTEXTURES");
        ConsoleSync();
    }
    if (modded)
    {
        QString path = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation).first() +
                "/MassEffectModder";
        QString mapFile = path + QString("/mele%1map.bin").arg((int)gameId);
        if (!TreeScan::loadTexturesMapFile(mapFile, textures))
            return false;
    }

    Misc::applyMods(modFiles, textures, pkgsToMarker, mipMaps,
                    true, verify, cacheAmount, callback, callbackHandle);



    if (!skipMarkers && !modded)
        Misc::AddMarkers(pkgsToMarker, callback, callbackHandle);

    if (!alotInstallerMode)
    {
        if (!ApplyPostInstall(gameId, modFiles))
            return false;
    }

    TOCBinFile::UpdateAllTOCBinFiles(gameId);

    if (g_ipc)
    {
        ConsoleWrite("[IPC]STAGE_CONTEXT STAGE_DONE");
        ConsoleSync();
    }

    long elapsed = Misc::elapsedTime();
    PINFO(Misc::getTimerFormat(elapsed) + "\n");

    PINFO("\nInstallation finished.\n\n");

    return true;
}

bool Misc::ApplyPostInstall(MeType gameId, QStringList &mods)
{
    applyModTag(mods);

    PINFO("Updating GFXs settings started...\n");
    QString path = g_GameData->EngineConfigIniPath(gameId);
    QDir().mkpath(DirName(path));
#if defined(_WIN32)
    ConfigIni engineConf = ConfigIni(path);
#else
    ConfigIni engineConf = ConfigIni(path, true);
#endif
    UserSettings::updateGFXSettings(gameId, engineConf);

    PINFO("Updating GFXs settings finished.\n\n");

    return true;
}
