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

    int scanTextures(MeType gameId);
    int scan(MeType gameId);
    bool updateTOCs(MeType gameId);
    bool GetGamePaths();
    bool unpackArchive(const QString &inputFile, QString &outputDir, QString &filterWithExt, bool flattenPath);
    bool listArchive(const QString &inputFile);
    bool applyModTag(MeType gameId, int MeuitmV, int AlotV);
    bool ConvertToMEM(MeType gameId, QString &inputDir, QString &memFile, bool fastMode, bool markToConvert, bool bc7format, float bc7quality);
    bool convertGameTexture(const QString &inputFile, QString &outputFile,
                            QList<TextureMapEntry> &textures, bool markToConvert, float bc7quality);
    bool convertGameImage(MeType gameId, QString &inputFile, QString &outputFile, bool markToConvert, float bc7quality);
    bool convertGameImages(MeType gameId, QString &inputDir, QString &outputDir, bool markToConvert, float bc7quality);
    bool convertImage(QString &inputFile, QString &outputFile, QString &format, int dxt1Threshold, float bc7qualityValue);
    bool extractMEM(MeType gameId, QString &inputDir, QString &outputDir);
    bool ApplyLODAndGfxSettings(MeType gameId);
    bool PrintLODSettings(MeType gameId);
    bool CheckGameDataAndMods(MeType gameId);
    bool CheckForMarkers(MeType gameId);
    bool DetectBadMods(MeType gameId);
    bool DetectMods(MeType gameId);
    void AddMarkers();
    bool InstallMods(MeType gameId, QString &inputDir,
                     bool alotInstaller, bool skipMarkers, bool verify, int cacheAmount);
    bool extractAllTextures(MeType gameId, QString &outputDir, QString &inputFile,
                            bool png, bool pccOnly, bool tfcOnly, bool mapCrc,
                            QString &textureTfcFilter, bool clearAlpha = false);
    bool extractAllMovieTextures(MeType gameId, QString &outputDir, QString &inputFile,
                                    bool pccOnly, bool tfcOnly, bool mapCrc,
                                    QString &textureTfcFilter);
    bool CheckTextures(MeType gameId);
    bool checkGameFilesAfter(MeType gameType);
    bool detectsMismatchPackagesAfter(MeType gameType);
};

#endif
