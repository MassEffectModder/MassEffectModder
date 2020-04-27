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

#ifndef GAME_DATA_H
#define GAME_DATA_H

#include <Program/ConfigIni.h>
#include <Types/MemTypes.h>

bool comparePath(const QString &e1, const QString &e2);

#define EXTENSION_AFC      ".afc"
#define EXTENSION_AFC_LEN  (sizeof(EXTENSION_AFC) - 1)

#define EXTENSION_TFC      ".tfc"
#define EXTENSION_TFC_LEN  (sizeof(EXTENSION_TFC) - 1)

#define EXTENSION_PCC      ".pcc"
#define EXTENSION_PCC_LEN  (sizeof(EXTENSION_PCC) - 1)

#define EXTENSION_U        ".u"
#define EXTENSION_U_LEN    (sizeof(EXTENSION_U) - 1)

#define EXTENSION_UPK      ".upk"
#define EXTENSION_UPK_LEN  (sizeof(EXTENSION_UPK) - 1)

#define EXTENSION_SFM      ".sfm"
#define EXTENSION_SFM_LEN  (sizeof(EXTENSION_SFM) - 1)

#define EXTENSION_BIK      ".bik"
#define EXTENSION_BIK_LEN  (sizeof(EXTENSION_BIK) - 1)

#define EXTENSION_SFAR     ".sfar"
#define EXTENSION_SFAR_LEN (sizeof(EXTENSION_SFAR) - 1)

#define EXTENSION_INI      ".ini"
#define EXTENSION_INI_LEN  (sizeof(EXTENSION_INI) - 1)

#define EXTENSION_USF      ".usf"
#define EXTENSION_USF_LEN  (sizeof(EXTENSION_USF) - 1)

#define GUIDCACHE_PCC      "guidcache.pcc"
#define GUIDCACHE_PCC_LEN  (sizeof(GUIDCACHE_PCC) - 1)

#define GUIDCACHE          "guidcache"
#define GUIDCACHE_LEN      (sizeof(GUIDCACHE) - 1)

#define SHADER1_PCC        "localshadercache-pc-d3d-sm3.upk"
#define SHADER1_PCC_LEN    (sizeof(SHADER1_PCC) - 1)

#define SHADER2_PCC        "refshadercache-pc-d3d-sm3.upk"
#define SHADER2_PCC_LEN    (sizeof(SHADER2_PCC) - 1)

#define LANG_PL            "_plpc."
#define LANG_PL_LEN        (sizeof(LANG_PL) - 1)

#define LANG_CS            "_cs."
#define LANG_CS_LEN        (sizeof(LANG_CS) - 1)

#define LANG_HU            "_hu."
#define LANG_HU_LEN        (sizeof(LANG_HU) - 1)

#define LANG_RU            "_ru."
#define LANG_RU_LEN        (sizeof(LANG_RU) - 1)

#define LANG_RA            "_ra."
#define LANG_RA_LEN        (sizeof(LANG_RA) - 1)

#define LANG_DE            "_de."
#define LANG_DE_LEN        (sizeof(LANG_DE) - 1)

#define LANG_FR            "_fr."
#define LANG_FR_LEN        (sizeof(LANG_FR) - 1)

#define LANG_IT            "_it."
#define LANG_IT_LEN        (sizeof(LANG_IT) - 1)

#define LANG_ES            "_es."
#define LANG_ES_LEN        (sizeof(LANG_ES) - 1)

class GameData
{
private:
    QString _path;

    void InternalInit(MeType type, ConfigIni &configIni);
    void ScanGameFiles(bool force, const QString &filterPath);

public:
    static MeType gameType;
    QStringList packageFiles;
    QStringList mainFiles;
    QStringList DLCFiles;
    QStringList sfarFiles;
    QStringList tfcFiles;
    QStringList coalescedFiles;
    QStringList afcFiles;
    QStringList bikFiles;
    QStringList packageME1UpperNames;
    QMap<QString, int> mapME1PackageUpperNames;
    bool FullScanGame;
    bool DLCDataCacheDone = false;

    void Init(MeType type);
    void Init(MeType type, ConfigIni &configIni);
    void Init(MeType type, ConfigIni &configIni, const QString &filterPath);
    void Init(MeType type, ConfigIni &configIni, bool force);
    QString GamePath() { return _path; }
    const QString MainData();
    const QString bioGamePath();
    const QString DLCData();
    const QString DLCDataSuffix();
    const QString RelativeGameData(const QString &path);
    const QString GameExePath();
    static const QString GameUserPath(MeType type);
    static const QString ConfigIniPath(MeType type);
    static const QString EngineConfigIniPath(MeType type);
    void ClosePackagesList();
};

extern GameData *g_GameData;

bool CreateGameData();
void ReleaseGameData();

#endif
