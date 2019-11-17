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
#include <GameData/TOCFile.h>
#include <GameData/DLC.h>
#include <GameData/LODSettings.h>
#include <MipMaps/MipMaps.h>
#include <Wrappers.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <Helpers/FileStream.h>

bool Misc::applyMods(QStringList &files, QList<TextureMapEntry> &textures,
                     QStringList &pkgsToRepack, QStringList &pkgsToMarker,
                     MipMaps &mipMaps, bool repack,
                     bool modded, bool verify, int cacheAmount,
                     ProgressCallback callback, void *callbackHandle)
{
    bool status = true;

    int totalNumberOfMods = 0;
    int currentNumberOfTotalMods = 1;

    QList<ModEntry> modsToReplace;

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
            int exportId = -1;
            QString pkgPath;
            fs.JumpTo(modFiles[l].offset);
            long size = modFiles[l].size;
            if (modFiles[l].tag == FileTextureTag || modFiles[l].tag == FileTextureTag2)
            {
                fs.ReadStringASCIINull(name);
                crc = fs.ReadUInt32();
            }
            else if (modFiles[l].tag == FileBinaryTag || modFiles[l].tag == FileXdeltaTag)
            {
                name = modFiles[l].name;
                exportId = fs.ReadInt32();
                fs.ReadStringASCIINull(pkgPath);
                pkgPath = pkgPath.replace('\\', '/');
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

            if (modFiles[l].tag == FileTextureTag || modFiles[l].tag == FileTextureTag2)
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
                    modsToReplace.push_back(entry);
                }
                else
                {
                    PINFO(QString("Texture skipped. Texture ") + name +
                                 QString().sprintf("_0x%08X", crc) + " is not present in your game setup.\n");
                }
            }
            else if (modFiles[l].tag == FileBinaryTag)
            {
                if (!QFile(g_GameData->GamePath() + pkgPath).exists())
                {
                    PINFO(QString("Warning: File ") + pkgPath +
                                 " not exists in your game setup.\n");
                    continue;
                }
                ModEntry entry{};
                entry.binaryModType = true;
                entry.packagePath = pkgPath;
                entry.exportId = exportId;
                entry.binaryModData = MipMaps::decompressData(fs, size);
                if (entry.binaryModData.size() == 0)
                {
                    if (g_ipc)
                    {
                        ConsoleWrite(QString("[IPC]ERROR Failed decompress data: ") + name);
                        ConsoleSync();
                    }
                    PERROR(QString("Failed decompress data: ") + name + "\n");
                    continue;
                }
                modsToReplace.push_back(entry);
            }
            else if (modFiles[l].tag == FileXdeltaTag)
            {
                QString path = g_GameData->GamePath() + pkgPath;
                if (!QFile(path).exists())
                {
                    PINFO(QString("Warning: File ") + pkgPath +
                                 " not exists in your game setup.\n");
                    continue;
                }
                ModEntry entry{};
                Package pkg;
                if (pkg.Open(path) != 0)
                {
                    PERROR(QString("Failed open package ") + pkgPath + "\n");
                    continue;
                }
                ByteBuffer src = pkg.getExportData(exportId);
                if (src.ptr() == nullptr)
                {
                    PERROR(QString("Failed get data, export id") +
                                 QString::number(exportId) + ", package: " + pkgPath + "\n");
                    continue;
                }
                ByteBuffer dst = MipMaps::decompressData(fs, size);
                if (dst.size() == 0)
                {
                    if (g_ipc)
                    {
                        ConsoleWrite(QString("[IPC]ERROR Failed decompress data: ") + name);
                        ConsoleSync();
                    }
                    PERROR(QString("Failed decompress data: ") + name + "\n");
                    continue;
                }
                auto buffer = ByteBuffer(src.size());
                uint dstLen = 0;
                int status = XDelta3Decompress(src.ptr(), src.size(), dst.ptr(), dst.size(), buffer.ptr(), &dstLen);
                src.Free();
                dst.Free();
                if (status != 0)
                {
                    PERROR(QString("Warning: Xdelta patch for ") + pkgPath + " failed to apply.\n");
                    buffer.Free();
                    continue;
                }
                entry.binaryModType = true;
                entry.packagePath = pkgPath;
                entry.exportId = exportId;
                entry.binaryModData = buffer;
                modsToReplace.push_back(entry);
            }
        }
    }

    mipMaps.replaceModsFromList(textures, pkgsToMarker, pkgsToRepack, modsToReplace,
                                repack, !modded, verify, !modded, cacheAmount,
                                callback, callbackHandle);

    PINFO("Process textures finished.\n\n");

    if (verify)
    {
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
    }

    return status;
}

bool Misc::InstallMods(MeType gameId, Resources &resources, QStringList &modFiles,
                       bool repack, bool alotMode, bool limit2k, bool verify, int cacheAmount,
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

    if (!alotMode)
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

    if (gameId == MeType::ME1_TYPE)
        repack = false;

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
        if (repack)
            ConsoleWrite("[IPC]STAGE_ADD STAGE_REPACK");
        if (!modded)
            ConsoleWrite("[IPC]STAGE_ADD STAGE_MARKERS");
        ConsoleSync();
    }

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

    if (repack)
    {
        for (int i = 0; i < g_GameData->packageFiles.count(); i++)
        {
            pkgsToRepack.push_back(g_GameData->packageFiles[i]);
        }
        if (GameData::gameType == MeType::ME1_TYPE)
            pkgsToRepack.removeOne("/BioGame/CookedPC/testVolumeLight_VFX.upk");
        else if (GameData::gameType == MeType::ME2_TYPE)
            pkgsToRepack.removeOne("/BioGame/CookedPC/BIOC_Materials.pcc");
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

    Misc::applyMods(modFiles, textures, pkgsToRepack, pkgsToMarker, mipMaps,
                    repack, modded, verify, cacheAmount, callback, callbackHandle);


    if (!modded)
        RemoveMipmaps(mipMaps, textures, pkgsToMarker, pkgsToRepack, repack, true, false,
                      callback, callbackHandle);


    if (repack)
        Misc::RepackME23(gameId, !modded, pkgsToRepack, callback, callbackHandle);


    if (!modded)
        Misc::AddMarkers(pkgsToMarker, callback, callbackHandle);


    if (!alotMode)
    {
        if (!applyModTag(gameId, 0, 0))
            PERROR("Failed applying stamp for installation!\n");
        PINFO("Updating LODs and other settings started...\n");
        QString path = GameData::EngineConfigIniPath(gameId);
        QDir().mkpath(DirName(path));
#if !defined(_WIN32)
        if (QFile(path).exists())
        {
            if (!Misc::ConvertEndLines(path, true))
                return false;
        }
#endif
        ConfigIni engineConf = ConfigIni(path);
        LODSettings::updateLOD(gameId, engineConf, limit2k);
        LODSettings::updateGFXSettings(gameId, engineConf, false, false);
#if !defined(_WIN32)
        if (!Misc::ConvertEndLines(path, false))
            return false;
#endif
        PINFO("Updating LODs and other settings finished.\n\n");
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
