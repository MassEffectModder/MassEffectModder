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

#include "GameData.h"

#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QDirIterator>

QString GameData::_path;
ConfigIni *GameData::_configIni = nullptr;
MeType GameData::gameType = UNKNOWN_TYPE;
QStringList GameData::gameFiles;
QStringList GameData::packageFiles;
QStringList GameData::tfcFiles;
bool GameData::FullScanME1Game = false;

void GameData::ScanGameFiles(bool force)
{
    if (force)
        gameFiles.clear();

    if (gameFiles.count() == 0)
    {
        QDirIterator iterator(MainData(), QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
        while (iterator.hasNext())
        {
            gameFiles.push_back(iterator.path().left(_path.length()));
        }

        if (QDir(DLCData()).exists())
        {
            QStringList DLCs = QDir(DLCData(), "DLC_*", QDir::NoSort, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryList();
            foreach (QString DLCDir, DLCs)
            {
                QStringList packages;
                QDirIterator iterator(DLCDir, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
                bool isValid = false;
                while (iterator.hasNext())
                {
                    if (gameType == MeType::ME3_TYPE && DLCDir.contains("Default.sfar", Qt::CaseInsensitive))
                        isValid = true;
                    packages.push_back(iterator.path().left(_path.length()));
                }
                if (gameType == MeType::ME3_TYPE && isValid)
                {
                    gameFiles += packages;
                }
            }
        }

        gameFiles.sort(Qt::CaseInsensitive);
    }
}

GameData::GameData(MeType type, ConfigIni *configIni, bool force = false)
{
    gameType = type;
    _configIni = configIni;

    QString key = QString("ME") + static_cast<char>(gameType);
    QString path = configIni->Read(key, "GameDataPath");
    if (path != "" && !force)
    {
        _path = QDir::cleanPath(path);
        if (QDir(GameExePath()).exists())
        {
            ScanGameFiles(force);
            return;
        }
        else
            _path = "";
    }

#if defined(_WIN32)
    QString registryKey = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\BioWare\\Mass Effect";
    QString entry = "Path";

    if (type == MeType::ME2_TYPE)
        registryKey += " 2";
    else if (type == MeType::ME3_TYPE)
    {
        registryKey += " 3";
        entry = "Install Dir";
    }

    QSettings settings(registryKey, QSettings::NativeFormat);
    path = settings.value(entry, "").toString();
    if (path != "" && !force)
    {
        _path = QDir::cleanPath(path);
        if (QDir(GameExePath()).exists())
        {
            configIni->Write(key, _path, "GameDataPath");
            ScanGameFiles(force);
            return;
        }
        else
            _path = "";
    }
#endif

    if (_path != "")
        _configIni->Write(key, _path, "GameDataPath");

    ScanGameFiles(force);
}

QString GameData::bioGamePath()
{
    if (_path == "")
        CRASH_MSG("Game path not set!");

    switch (gameType)
    {
        case MeType::ME1_TYPE:
        case MeType::ME2_TYPE:
            return _path + "/BioGame";
        case MeType::ME3_TYPE:
            return _path + "/BIOGame";
        case MeType::UNKNOWN_TYPE:
            CRASH();
    }

    CRASH();
}

QString GameData::MainData()
{
    switch (gameType)
    {
        case MeType::ME1_TYPE:
        case MeType::ME2_TYPE:
            return bioGamePath() + "/CookedPC";
        case MeType::ME3_TYPE:
            return bioGamePath() + "/CookedPCConsole";
        case MeType::UNKNOWN_TYPE:
            CRASH();
    }

    CRASH();
}

QString GameData::DLCData()
{
    if (_path == "")
        CRASH_MSG("Game path not set!");

    switch (gameType)
    {
        case MeType::ME1_TYPE:
            return _path + "/DLC";
        case MeType::ME2_TYPE:
        case MeType::ME3_TYPE:
            return bioGamePath() + "/DLC";
        case MeType::UNKNOWN_TYPE:
            CRASH();
    }

    CRASH();
}

QString GameData::GameExePath()
{
    if (_path == "")
        CRASH_MSG("Game path not set!");

    switch (gameType)
    {
        case MeType::ME1_TYPE:
            return _path + "/Binaries/MassEffect.exe";
        case MeType::ME2_TYPE:
            return _path + "/Binaries/MassEffect2.exe";
        case MeType::ME3_TYPE:
            return _path + "/Binaries/Win32/MassEffect3.exe";
        case MeType::UNKNOWN_TYPE:
            CRASH();
    }

    CRASH();
}

QString GameData::GameUserPath()
{
    if (_path == "")
        CRASH_MSG("Game path not set!");

    QString path = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first() + "/BioWare/Mass Effect";
    if (gameType == MeType::ME2_TYPE)
        path += " 2";
    else if (gameType == MeType::ME3_TYPE)
        path += " 3";

    return path;
}

QString GameData::ConfigIniPath()
{
    switch (gameType)
    {
        case MeType::ME1_TYPE:
            return GameUserPath() + "/Config";
        case MeType::ME2_TYPE:
        case MeType::ME3_TYPE:
            return GameUserPath() + "/BioGame/Config";
        case MeType::UNKNOWN_TYPE:
            CRASH();
    }

    CRASH();
}

QString GameData::EngineConfigIniPath()
{
    switch (gameType)
    {
        case MeType::ME1_TYPE:
            return ConfigIniPath() + "/BIOEngine.ini";
        case MeType::ME2_TYPE:
        case MeType::ME3_TYPE:
            return ConfigIniPath() + "/GamerSettings.ini";
        case MeType::UNKNOWN_TYPE:
            CRASH();
    }

    CRASH();
}

QString GameData::RelativeGameData(QString path)
{
    if (_path == "")
        CRASH_MSG("Game path not set!");

    if (!path.toLower().contains(_path.toLower()))
        CRASH_MSG("The path not found in game path!");

    return path.left(_path.length());
}

void GameData::getTfcTextures()
{
    if (tfcFiles.count() != 0)
        return;

    if (gameFiles.count() == 0)
        ScanGameFiles(false);

    QRegExp regex("*.tfc", Qt::CaseInsensitive, QRegExp::Wildcard);
    tfcFiles = gameFiles.filter(regex);
}

void GameData::getPackages()
{
    FullScanME1Game = false;
    int index;

    if (gameFiles.count() == 0)
        ScanGameFiles(false);

    if (gameType == MeType::ME1_TYPE)
    {
        packageFiles = gameFiles.filter(QRegExp("*.upk", Qt::CaseInsensitive, QRegExp::Wildcard));
        packageFiles += gameFiles.filter(QRegExp("*.u", Qt::CaseInsensitive, QRegExp::Wildcard));
        packageFiles += gameFiles.filter(QRegExp("*.sfm", Qt::CaseInsensitive, QRegExp::Wildcard));
        index = packageFiles.indexOf(QRegExp("*localshadercache-pc-d3d-sm3.upk", Qt::CaseInsensitive, QRegExp::Wildcard));
        if (index != -1)
            packageFiles.removeAt(index);
        index = packageFiles.indexOf(QRegExp("*refshadercache-pc-d3d-sm3.upk", Qt::CaseInsensitive, QRegExp::Wildcard));
        if (index != -1)
            packageFiles.removeAt(index);

        if (packageFiles.indexOf(QRegExp("*_PLPC.*", Qt::CaseInsensitive, QRegExp::Wildcard)) != -1)
            FullScanME1Game = true;
        else if (packageFiles.indexOf(QRegExp("*_CS.*", Qt::CaseInsensitive, QRegExp::Wildcard)) != -1)
            FullScanME1Game = true;
        else if (packageFiles.indexOf(QRegExp("*_HU.*", Qt::CaseInsensitive, QRegExp::Wildcard)) != -1)
            FullScanME1Game = true;
        else if (packageFiles.indexOf(QRegExp("*_RU.*", Qt::CaseInsensitive, QRegExp::Wildcard)) != -1)
            FullScanME1Game = true;
        else if (packageFiles.indexOf(QRegExp("*_RA.*", Qt::CaseInsensitive, QRegExp::Wildcard)) != -1)
            FullScanME1Game = true;
        else
        {
            QDirIterator iterator(MainData() + "/Movies", QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
            while (iterator.hasNext())
            {
                if (iterator.path().contains("niebieska_pl.bik"))
                {
                    FullScanME1Game = true;
                    break;
                }
            }
        }
    }
    else if (gameType == MeType::ME2_TYPE)
    {
        packageFiles = gameFiles.filter(QRegExp("*.pcc", Qt::CaseInsensitive, QRegExp::Wildcard));
    }
    else if (gameType == MeType::ME3_TYPE)
    {
        packageFiles = gameFiles.filter(QRegExp("*.pcc", Qt::CaseInsensitive, QRegExp::Wildcard));
        index = packageFiles.indexOf(QRegExp("*guidcache*", Qt::CaseInsensitive, QRegExp::Wildcard));
        if (index != -1)
            packageFiles.removeAt(index);
    }
}

void GameData::ClosePackagesList()
{
    gameFiles.clear();
    packageFiles.clear();
    tfcFiles.clear();
}
