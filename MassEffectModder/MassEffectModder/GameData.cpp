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

MeType GameData::gameType = UNKNOWN_TYPE;

void GameData::ScanGameFiles(bool force)
{
    if (force)
        gameFiles.clear();

    if (gameFiles.count() == 0)
    {
        QDirIterator iterator(MainData(), QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
        int pathLen = _path.length();
        while (iterator.hasNext())
        {
            iterator.next();
            gameFiles.push_back(iterator.filePath().mid(pathLen));
        };

        if (QDir(DLCData()).exists())
        {
            QStringList DLCs = QDir(DLCData(), "DLC_*", QDir::NoSort, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryList();
            foreach (QString DLCDir, DLCs)
            {
                QStringList packages;
                QDirIterator iterator(DLCData() + "/" + DLCDir, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
                bool isValid = false;
                while (iterator.hasNext())
                {
                    iterator.next();
                    if (gameType == MeType::ME3_TYPE && iterator.filePath().contains("Default.sfar", Qt::CaseInsensitive))
                        isValid = true;
                    packages.push_back(iterator.filePath().mid(pathLen));
                }
                if (gameType == MeType::ME3_TYPE)
                {
                    if (isValid)
                        gameFiles += packages;
                }
                else
                {
                    gameFiles += packages;
                }
            }
        }

        gameFiles.sort(Qt::CaseInsensitive);
    }
}

void GameData::Init(MeType type, ConfigIni *configIni)
{
    InternalInit(type, configIni, false);
}

void GameData::Init(MeType type, ConfigIni *configIni, bool force = false)
{
    InternalInit(type, configIni, force);
}

void GameData::InternalInit(MeType type, ConfigIni *configIni, bool force = false)
{
    gameType = type;
    _configIni = configIni;

    QString key = QString("ME%1").arg(static_cast<int>(gameType));
    QString path = configIni->Read(key, "GameDataPath");
    if (path != "" && !force)
    {
        _path = QDir::cleanPath(path);
        if (QFile(GameExePath()).exists())
        {
            ScanGameFiles(force);
            return;
        }
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
        if (QFile(GameExePath()).exists())
        {
            configIni->Write(key,
                       _path.replace(QChar('/'), QChar('\\'), Qt::CaseInsensitive), "GameDataPath");
            ScanGameFiles(force);
            return;
        }
        _path = "";
    }
#endif

    if (_path != "")
#if defined(_WIN32)
        configIni->Write(key,
                   _path.replace(QChar('/'), QChar('\\'), Qt::CaseInsensitive), "GameDataPath");
#else
        _configIni->Write(key, _path, "GameDataPath");
#endif

    ScanGameFiles(force);
}

const QString GameData::bioGamePath()
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

const QString GameData::MainData()
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

const QString GameData::DLCData()
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

const QString GameData::GameExePath()
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

const QString GameData::GameUserPath()
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

const QString GameData::ConfigIniPath()
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

const QString GameData::EngineConfigIniPath()
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

const QString GameData::RelativeGameData(QString &path)
{
    if (_path == "")
        CRASH_MSG("Game path not set!");

    if (!path.contains(_path.toLower(), Qt::CaseInsensitive))
        CRASH_MSG("The path not found in game path!");

    return path.left(_path.length());
}

void GameData::getTfcTextures()
{
    if (tfcFiles.count() != 0)
        return;

    if (gameFiles.count() == 0)
        ScanGameFiles(false);

    tfcFiles = gameFiles.filter(QRegExp("*.tfc", Qt::CaseInsensitive, QRegExp::Wildcard));
}

void GameData::getPackages()
{
    FullScanME1Game = false;
    int index;

    if (gameFiles.count() == 0)
        ScanGameFiles(false);

    if (gameType == MeType::ME1_TYPE)
    {
        // TODO: figure out why *.u match also *.upk
        //packageFiles = gameFiles.filter(QRegExp("*.upk", Qt::CaseInsensitive, QRegExp::Wildcard));
        packageFiles = gameFiles.filter(QRegExp("*.u", Qt::CaseInsensitive, QRegExp::Wildcard));
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
                iterator.next();
                if (iterator.fileName().compare("niebieska_pl.bik") == 0)
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
        do
        {
            index = packageFiles.indexOf(QRegExp("*guidcache*", Qt::CaseInsensitive, QRegExp::Wildcard));
            if (index != -1)
                packageFiles.removeAt(index);
        } while (index != -1);
    }
}

void GameData::ClosePackagesList()
{
    gameFiles.clear();
    packageFiles.clear();
    tfcFiles.clear();
}

GameData *g_GameData;

bool CreateGameData()
{
    g_GameData = new GameData();
    if (g_GameData == nullptr)
    {
        std::fputs("CreateLogs: Failed create instance\n", stderr);
        return false;
    }
    return true;
}

void ReleaseGameData()
{
    delete g_GameData;
    g_GameData = nullptr;
}
