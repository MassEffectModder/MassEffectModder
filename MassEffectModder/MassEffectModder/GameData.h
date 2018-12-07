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

#ifndef GAME_DATA_H
#define GAME_DATA_H

#include "MemTypes.h"
#include "ConfigIni.h"

bool compareByAscii(const QString &e1, const QString &e2);

class GameData
{
private:
    QString _path;

    void InternalInit(MeType type, ConfigIni &configIni, bool force);
    void ScanGameFiles(bool force);

public:
    static MeType gameType;
    QStringList packageFiles;
    QStringList packageMainFiles;
    QStringList packageDLCFiles;
    QStringList sfarFiles;
    QStringList tfcFiles;
    QStringList packageUpperNames;
    QMap<QString, int> mapPackageUpperNames;
    bool FullScanME1Game;
    bool DLCDataCacheDone = false;

    void Init(MeType type, ConfigIni &configIni);
    void Init(MeType type, ConfigIni &configIni, bool force);
    QString GamePath() { return _path; }
    const QString MainData();
    const QString bioGamePath();
    const QString DLCData();
    const QString DLCDataSuffix();
    const QString RelativeGameData(QString &path);
    const QString GameExePath();
    const QString GameUserPath();
    const QString ConfigIniPath();
    const QString EngineConfigIniPath();
    void ClosePackagesList();
};

extern GameData *g_GameData;

bool CreateGameData();
void ReleaseGameData();

#endif
