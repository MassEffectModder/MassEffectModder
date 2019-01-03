/*
 * MassEffectModder
 *
 * Copyright (C) 2018 Pawel Kolodziejski <aquadran at users.sourceforge.net>
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

#include "MemTypes.h"
#include "Image.h"
#include "TreeScan.h"
#include "MipMaps.h"

class CmdLineTools
{

private:

    QStringList pkgsToRepack;
    QStringList pkgsToMarker;
    QList<ModEntry> modsToReplace;

    bool detectMod(MeType gameId);
    bool ScanTextures(MeType gameId, Resources &resources, QList<FoundTexture> &texture, bool ipc);
    bool RemoveMipmaps(MipMaps &mipMaps, QList<FoundTexture> &textures,
                       QStringList &pkgsToMarker, QStringList &pkgsToRepack,
                       bool ipc, bool repack, bool appendMarker);
    void RepackME23(MeType gameId, bool ipc, bool appendMarker);

public:

    int scanTextures(MeType gameId, bool ipc);
    int removeEmptyMips(MeType gameId, bool ipc);
    void Repack(MeType gameId, bool ipc);
    bool applyModTag(MeType gameId, int MeuitmV, int AlotV);
    bool ConvertToMEM(MeType gameId, QString &inputDir, QString &memFile, bool markToConvert, bool ipc);
    bool convertGameTexture(const QString &inputFile, QString &outputFile, QList<FoundTexture> *textures,
                            bool markToConvert);
    bool convertGameImage(MeType gameId, QString &inputFile, QString &outputFile, bool markToConvert);
    bool convertGameImages(MeType gameId, QString &inputDir, QString &outputDir, bool markToConvert);
    bool convertImage(QString &inputFile, QString &outputFile, QString &format, int dxt1Threshold);
    bool extractTPF(QString &inputDir, QString &outputDir, bool ipc);
    bool extractMOD(MeType gameId, QString &inputDir, QString &outputDir, bool ipc);
    bool extractMEM(MeType gameId, QString &inputDir, QString &outputDir, bool ipc);
    bool ApplyME1LAAPatch();
    bool ApplyLODAndGfxSettings(MeType gameId, bool softShadowsME1, bool meuitmMode, bool limit2k);
    bool RemoveLODSettings(MeType gameId);
    bool PrintLODSettings(MeType gameId, bool ipc);
    bool CheckGameData(MeType gameId, bool ipc);
    bool CheckForMarkers(MeType gameId, bool ipc);
    bool DetectBadMods(MeType gameId, bool ipc);
    bool DetectMods(MeType gameId, bool ipc);
    void AddMarkers(bool ipc);
    bool InstallMods(MeType gameId, QString &inputDir, bool ipc, bool repack, bool guiInstaller, bool
#if defined(_WIN32)
                     limit2k, bool
#endif
                     verify);
    bool applyMEMSpecialModME3(MeType gameId, QString &memFile, QString &tfcName, QByteArray &guid, bool verify);
    bool applyMods(QStringList &files, QList<FoundTexture> &textures, MipMaps &mipMaps, bool repack,
                   bool modded, bool ipc, QString &tfcName, QByteArray &guid, bool verify = false, bool special = false);
    void replaceTextureSpecialME3Mod(Image &image, QList<MatchedTexture> &list, QString &textureName,
                                     QString &tfcName, QByteArray &guid, bool verify);
    bool extractAllTextures(MeType gameId, QString &outputDir, bool png,
                            bool pccOnly, bool tfcOnly, QString &textureTfcFilter);
    bool CheckTextures(MeType gameId, bool ipc);
    bool checkGameFilesAfter(MeType gameType, bool ipc = false);
    bool detectsMismatchPackagesAfter(MeType gameType, bool ipc = false);
};

#endif
