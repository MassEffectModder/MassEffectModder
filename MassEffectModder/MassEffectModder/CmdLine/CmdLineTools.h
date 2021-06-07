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

#ifndef CMD_LINE_TOOLS_H
#define CMD_LINE_TOOLS_H

#include <Types/MemTypes.h>
#include <Image/Image.h>
#include <Texture/TextureScan.h>
#include <MipMaps/MipMaps.h>

class CmdLineTools
{
public:
    typedef void (*ProgressCallback)(void *handle, int progress, const QString &stage);

    int scanTextures(MeType gameId, bool removeEmptyMips);
    void Repack(MeType gameId, ProgressCallback callback, void *callbackHandle);
    bool updateTOCs();
    bool unpackAllDLCs();
    bool GetGamePaths();
    bool repackGame(MeType gameId);
    bool unpackArchive(const QString &inputFile, QString &outputDir, QString &filterWithExt, bool flattenPath);
    bool listArchive(const QString &inputFile);
    bool applyModTag(MeType gameId, int MeuitmV, int AlotV);
    bool ConvertToMEM(MeType gameId, QString &inputDir, QString &memFile, bool markToConvert);
    bool convertGameTexture(MeType gameId, const QString &inputFile, QString &outputFile,
                            QList<TextureMapEntry> &textures, bool markToConvert);
    bool convertGameImage(MeType gameId, QString &inputFile, QString &outputFile, bool markToConvert);
    bool convertGameImages(MeType gameId, QString &inputDir, QString &outputDir, bool markToConvert);
    bool convertImage(QString &inputFile, QString &outputFile, QString &format, int dxt1Threshold);
    bool extractTPF(QString &inputDir, QString &outputDir);
    bool extractMOD(MeType gameId, QString &inputDir, QString &outputDir);
    bool extractMEM(MeType gameId, QString &inputDir, QString &outputDir);
    bool ApplyME1LAAPatch();
    bool ApplyLODAndGfxSettings(MeType gameId, bool limit2k);
    bool RemoveLODSettings(MeType gameId);
    bool PrintLODSettings(MeType gameId);
    bool CheckGameDataAndMods(MeType gameId);
    bool CheckForMarkers(MeType gameId);
    bool DetectBadMods(MeType gameId);
    bool DetectMods(MeType gameId);
    void AddMarkers();
    bool InstallMods(MeType gameId, QString &inputDir, bool repack,
                     bool alotInstaller, bool skipMarkers, bool limit2k, bool verify, int cacheAmount);
    bool RepackTFCInDLC(MeType gameId, QString &dlcName, bool pullTextures,
                        bool compressed);
    bool extractAllTextures(MeType gameId, QString &outputDir, QString &inputFile,
                            bool png, bool pccOnly, bool tfcOnly, bool mapCrc,
                            QString &textureTfcFilter, bool clearAlpha = false);
    bool extractAllMovieTextures(MeType gameId, QString &outputDir, QString &inputFile,
                                    bool pccOnly, bool tfcOnly, bool mapCrc,
                                    QString &textureTfcFilter);
    bool CheckTextures(MeType gameId);
    bool FixMissingPropertyInTextures(MeType gameId, const QString& filter);
    bool checkGameFilesAfter(MeType gameType);
    bool detectsMismatchPackagesAfter(MeType gameType);
};

#endif
