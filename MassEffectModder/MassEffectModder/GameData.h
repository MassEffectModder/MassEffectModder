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

#include "MeType.h"
#include "ConfigIni.h"

#include <QString>
#include <QStringList>

class GameData
{
private:
    static QString _path;
    static ConfigIni *_configIni;

    void ScanGameFiles(bool force);

public:
    static MeType gameType;
    static QStringList gameFiles;
    static QStringList packageFiles;
    static QStringList tfcFiles;
    static bool FullScanME1Game;
    bool DLCDataCacheDone = false;

    explicit GameData();
    GameData(MeType type, ConfigIni *configIni, bool force);
    QString GamePath() { return _path; }
    QString MainData();
    QString bioGamePath();
    QString RelativeGameData(QString path);
    QString DLCData();
    QString GameExePath();
    QString GameUserPath();
    QString ConfigIniPath();
    QString EngineConfigIniPath();
    void getTfcTextures();
    void getPackages();
    void ClosePackagesList();
};

#endif
