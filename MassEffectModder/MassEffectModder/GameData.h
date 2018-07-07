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

#include <QString>
#include <QStringList>

class GameData
{
private:
    static QString _path;
    static ConfigIni *_configIni;

    void Init(MeType type, ConfigIni *configIni, bool force);
    void ScanGameFiles(bool force);

public:
    static MeType gameType;
    static QStringList gameFiles;
    static QStringList packageFiles;
    static QStringList tfcFiles;
    static bool FullScanME1Game;
    bool DLCDataCacheDone = false;

    GameData(MeType type, ConfigIni *configIni);
    GameData(MeType type, ConfigIni *configIni, bool force);
    static QString GamePath() { return _path; }
    static const QString MainData();
    static const QString bioGamePath();
    static const QString DLCData();
    static const QString RelativeGameData(QString &path);
    const QString GameExePath();
    const QString GameUserPath();
    const QString ConfigIniPath();
    const QString EngineConfigIniPath();
    void getTfcTextures();
    void getPackages();
    void ClosePackagesList();
};

#endif
