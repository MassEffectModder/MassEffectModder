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
#include <GameData/DLC.h>
#include <GameData/LODSettings.h>
#include <MipMaps/MipMaps.h>
#include <Wrappers.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <Helpers/FileStream.h>

bool Misc::applyMods(QStringList &files, QList<TextureMapEntry> &textures,
                     QStringList &pkgsToMarker,
                     MipMaps &mipMaps, bool appendMarker,
                     bool modded, bool verify, int cacheAmount,
                     ProgressCallback callback, void *callbackHandle)
{
    bool status = true;

    int totalNumberOfMods = 0;
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
                PERROR(QString("MEM mod file has 0 length: ") + files[i] + "\n");
            }
            continue;
        }
        FileStream fs = FileStream(files[i], FileMode::Open, FileAccess::ReadOnly);
        if (!Misc::CheckMEMHeader(fs, files[i]))
            continue;
        fs.JumpTo(fs.ReadInt64());
        fs.SkipInt32();
        totalNumberOfMods += fs.ReadInt32();
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
            modFiles.push_back(fileMod);
        }
        numFiles = modFiles.count();
        for (int l = 0; l < numFiles; l++, currentNumberOfTotalMods++)
        {
            QString name;
            uint crc = 0;
            fs.JumpTo(modFiles[l].offset);
            long size = modFiles[l].size;
            if (modFiles[l].tag == FileTextureTag ||
                modFiles[l].tag == FileTextureTag2 ||
                modFiles[l].tag == FileMovieTextureTag)
            {
                fs.ReadStringASCIINull(name);
                crc = fs.ReadUInt32();
            }
            else
            {
                if (g_ipc)
                {
                    ConsoleWrite(QString("[IPC]ERROR Unknown tag for file: ") + name);
                    ConsoleSync();
                }
                else
                {
                    PERROR(QString("Unknown tag for file: ") + name + "\n");
                }
                continue;
            }

            if (modFiles[l].tag == FileTextureTag ||
                modFiles[l].tag == FileTextureTag2 ||
                modFiles[l].tag == FileMovieTextureTag)
            {
                TextureMapEntry f = Misc::FoundTextureInTheMap(textures, crc);
                if (f.crc != 0)
                {
                    ModEntry entry{};
                    entry.textureCrc = f.crc;
                    entry.textureName = f.name;
                    if (modFiles[l].tag == FileTextureTag2)
                        entry.markConvert = true;
                    entry.memPath = files[i];
                    entry.memEntryOffset = fs.Position();
                    entry.memEntrySize = size;
                    entry.injectedTexture = nullptr;
                    modsToReplace.push_back(entry);
                }
                else
                {
                    PINFO(QString("Texture skipped. Texture ") + name +
                                 QString::asprintf("_0x%08X", crc) + " is not present in your game setup.\n");
                }
            }
        }
    }

    mipMaps.replaceModsFromList(textures, pkgsToMarker, modsToReplace,
                                appendMarker, verify, !modded, cacheAmount,
                                callback, callbackHandle);

    PINFO("Process textures finished.\n\n");

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

    if (GameData::ConfigIniPath(gameId).length() == 0)
    {
        PERROR("Game User path is not defined.\n");
        return false;
    }

    if (!alotInstallerMode && !guiInstallerMode)
    {
        if (gameId == MeType::ME1_TYPE && !QFile(GameData::EngineConfigIniPath(gameId)).exists())
        {
            PERROR("Error: Missing game configuration file.\nYou need atleast once launch the game first.\n");
            return false;
        }
        bool writeAccess = Misc::CheckAndCorrectAccessToGame();
        if (!writeAccess)
        {
            PERROR("Error: Detected no write access to game folders.\n");
            return false;
        }

        QStringList badMods;
        Misc::detectBrokenMod(badMods);
        if (badMods.count() != 0)
        {
            PERROR("Error: Detected not compatible mods:\n");
            for (int l = 0; l < badMods.count(); l++)
            {
                PERROR(badMods[l] + "\n");
            }
            return false;
        }
    }

    Misc::startTimer();

    bool modded = detectMod(gameId);
    bool unpackNeeded = false;
    if (gameId == MeType::ME3_TYPE && !modded)
        unpackNeeded = Misc::unpackSFARisNeeded();
    if (g_ipc)
    {
        if (!modded)
        {
            if (gameId == MeType::ME3_TYPE && unpackNeeded)
                ConsoleWrite("[IPC]STAGE_ADD STAGE_UNPACKDLC");
            if (!g_GameData->FullScanGame)
                ConsoleWrite("[IPC]STAGE_ADD STAGE_PRESCAN");
            ConsoleWrite("[IPC]STAGE_ADD STAGE_SCAN");
        }
        ConsoleWrite("[IPC]STAGE_ADD STAGE_INSTALLTEXTURES");
        if (verify)
            ConsoleWrite("[IPC]STAGE_ADD STAGE_VERIFYTEXTURES");
        if (!modded)
            ConsoleWrite("[IPC]STAGE_ADD STAGE_REMOVEMIPMAPS");
        if (!skipMarkers && !modded)
            ConsoleWrite("[IPC]STAGE_ADD STAGE_MARKERS");
        ConsoleSync();
    }

    Misc::startStageTimer();
    if (gameId == MeType::ME3_TYPE && !modded && unpackNeeded)
    {
        PINFO("Unpacking DLCs started...\n");
        if (g_ipc)
        {
            ConsoleWrite("[IPC]STAGE_CONTEXT STAGE_UNPACKDLC");
            ConsoleSync();
        }

        ME3DLC::unpackAllDLC(callback, callbackHandle);

        ConfigIni configIni = ConfigIni();
        g_GameData->Init(gameId, configIni, true);

        PINFO("Unpacking DLCs finished.\n\n");
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
        if (GameData::gameType == MeType::ME1_TYPE)
            pkgsToMarker.removeOne("/BioGame/CookedPC/testVolumeLight_VFX.upk");
        else if (GameData::gameType == MeType::ME2_TYPE)
            pkgsToMarker.removeOne("/BioGame/CookedPC/BIOC_Materials.pcc");

        PINFO("Scan textures started...\n");
        if (!TreeScan::PrepareListOfTextures(gameId, resources, textures, false, true,
                                        callback, callbackHandle))
        {
            PERROR("Failed to scan textures!\n");
            return false;
        }
        PINFO("Scan textures finished.\n\n");
    }


    PINFO("Process textures started...\n");
    if (g_ipc)
    {
        ConsoleWrite("[IPC]STAGE_CONTEXT STAGE_INSTALLTEXTURES");
        ConsoleSync();
    }
    if (modded)
    {
        QString path = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation).first() +
                "/MassEffectModder";
        QString mapFile = path + QString("/me%1map.bin").arg((int)gameId);
        if (!TreeScan::loadTexturesMapFile(mapFile, textures))
            return false;
    }

    Misc::applyMods(modFiles, textures, pkgsToMarker, mipMaps,
                    true, modded, verify, cacheAmount, callback, callbackHandle);


    if (!modded)
        RemoveMipmaps(mipMaps, textures, pkgsToMarker, true, false, callback, callbackHandle);


    if (!skipMarkers && !modded)
        Misc::AddMarkers(pkgsToMarker, callback, callbackHandle);

    if (!alotInstallerMode)
    {
        if (!ApplyPostInstall(gameId))
            return false;
    }

    if (gameId == MeType::ME3_TYPE)
        TOCBinFile::UpdateAllTOCBinFiles();

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

bool Misc::ApplyPostInstall(MeType gameId)
{
    if (!applyModTag(gameId, 0, 0))
        PERROR("Failed applying stamp for installation!\n");

    PINFO("Updating LODs and other settings started...\n");
    QString path = GameData::EngineConfigIniPath(gameId);
    QDir().mkpath(DirName(path));
#if defined(_WIN32)
    ConfigIni engineConf = ConfigIni(path);
#else
    ConfigIni engineConf = ConfigIni(path, true);
#endif
    LODSettings::updateLOD(gameId, engineConf);
    LODSettings::updateGFXSettings(gameId, engineConf);

    PINFO("Updating LODs and other settings finished.\n\n");

    return true;
}
