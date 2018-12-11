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
#include "Helpers/MiscHelpers.h"

MeType GameData::gameType = UNKNOWN_TYPE;

bool compareByAscii(const QString &e1, const QString &e2)
{
    return e1.compare(e2, Qt::CaseInsensitive) < 0;
}

void GameData::ScanGameFiles(bool force)
{
    if (force)
        packageFiles.clear();

    if (packageFiles.count() == 0)
    {
        QDirIterator iterator(MainData(), QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
        int pathLen = _path.length();
        while (iterator.hasNext())
        {
            iterator.next();
            QString path = iterator.filePath().mid(pathLen);
            if (gameType == MeType::ME1_TYPE)
            {
                if (path.endsWith("localshadercache-pc-d3d-sm3.upk", Qt::CaseInsensitive))
                    continue;
                if (path.endsWith("refshadercache-pc-d3d-sm3.upk", Qt::CaseInsensitive))
                    continue;
            }
            else if (gameType == MeType::ME3_TYPE)
            {
                if (path.contains("guidcache", Qt::CaseInsensitive))
                    continue;
            }
            packageMainFiles.push_back(path);
        };

        if (gameType == MeType::ME1_TYPE)
        {
            QDirIterator iterator(GamePath() + "/Movies", QDir::Files | QDir::NoSymLinks);
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

        if (QDir(DLCData()).exists())
        {
            QStringList DLCs = QDir(DLCData(), "DLC_*", QDir::NoSort, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryList();
            foreach (QString DLCDir, DLCs)
            {
                QStringList packages;
                QDirIterator iterator(DLCData() + "/" + DLCDir + DLCDataSuffix(), QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
                bool isValid = false;
                while (iterator.hasNext())
                {
                    iterator.next();
                    QString path = iterator.filePath().mid(pathLen);
                    if (gameType == MeType::ME2_TYPE)
                    {
                        if (path.endsWith(".tfc", Qt::CaseInsensitive))
                            tfcFiles.push_back(path);
                    }
                    else if (gameType == MeType::ME3_TYPE)
                    {
                        if (path.contains("guidcache", Qt::CaseInsensitive))
                            continue;
                        if (path.endsWith(".tfc", Qt::CaseInsensitive))
                            tfcFiles.push_back(path);
                        else if (path.endsWith(".sfar", Qt::CaseInsensitive))
                        {
                            sfarFiles.push_back(path);
                            isValid = true;
                        }
                    }
                    packages.push_back(path);
                }
                if (gameType == MeType::ME3_TYPE)
                {
                    if (isValid)
                        packageDLCFiles += packages;
                }
                else
                {
                    packageDLCFiles += packages;
                }
            }

            std::sort(tfcFiles.begin(), tfcFiles.end(), compareByAscii);
        }


        if (gameType == MeType::ME1_TYPE)
        {
            for (int i = 0; i < packageMainFiles.count(); i++)
            {
                if (packageMainFiles[i].endsWith(".u", Qt::CaseInsensitive) ||
                    packageMainFiles[i].endsWith(".upk", Qt::CaseInsensitive) ||
                    packageMainFiles[i].endsWith(".sfm", Qt::CaseInsensitive))
                {
                    packageFiles += packageMainFiles[i];
                }
            }
            for (int i = 0; i < packageDLCFiles.count(); i++)
            {
                if (packageDLCFiles[i].endsWith(".u", Qt::CaseInsensitive) ||
                    packageDLCFiles[i].endsWith(".upk", Qt::CaseInsensitive) ||
                    packageDLCFiles[i].endsWith(".sfm", Qt::CaseInsensitive))
                {
                    packageFiles += packageDLCFiles[i];
                }
            }
        }
        else
        {
            for (int i = 0; i < packageMainFiles.count(); i++)
            {
                if (packageMainFiles[i].endsWith(".pcc", Qt::CaseInsensitive))
                {
                    packageFiles += packageMainFiles[i];
                }
            }
            for (int i = 0; i < packageDLCFiles.count(); i++)
            {
                if (packageDLCFiles[i].endsWith(".pcc", Qt::CaseInsensitive))
                {
                    packageFiles += packageDLCFiles[i];
                }
            }
        }

        std::sort(packageFiles.begin(), packageFiles.end(), compareByAscii);

        if (gameType == MeType::ME1_TYPE)
        {
            for (int i = 0; i < packageFiles.count(); i++)
            {
                mapPackageUpperNames.insert(BaseNameWithoutExt(packageFiles[i]).toUpper(), i);
                packageUpperNames += BaseNameWithoutExt(packageFiles[i]).toUpper();
            }
        }

        if (gameType == MeType::ME1_TYPE && !FullScanME1Game)
        {
            for (int i = 0; i < packageFiles.count(); i++)
            {
                QString path = packageFiles[i];
                if (path.contains("_PLPC.", Qt::CaseInsensitive) ||
                    path.contains("_CS.", Qt::CaseInsensitive) ||
                    path.contains("_HU.", Qt::CaseInsensitive) ||
                    path.contains("_RU.", Qt::CaseInsensitive) ||
                    path.contains("_RA.", Qt::CaseInsensitive))
                {
                    FullScanME1Game = true;
                    break;
                }
            }
        }
    }
}

void GameData::Init(MeType type, ConfigIni &configIni)
{
    InternalInit(type, configIni, false);
}

void GameData::Init(MeType type, ConfigIni &configIni, bool force = false)
{
    InternalInit(type, configIni, force);
}

void GameData::InternalInit(MeType type, ConfigIni &configIni, bool force = false)
{
    gameType = type;

    QString key = QString("ME%1").arg(static_cast<int>(gameType));
    QString path = configIni.Read(key, "GameDataPath");
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
    QString registryKey = R"(HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\BioWare\Mass Effect)";
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
            configIni.Write(key,
                       _path.replace(QChar('/'), QChar('\\'), Qt::CaseInsensitive), "GameDataPath");
            ScanGameFiles(force);
            return;
        }
        _path = "";
    }
#endif

    if (_path != "")
#if defined(_WIN32)
        configIni.Write(key,
                   _path.replace(QChar('/'), QChar('\\'), Qt::CaseInsensitive), "GameDataPath");
#else
        configIni.Write(key, _path, "GameDataPath");
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

const QString GameData::DLCDataSuffix()
{
    switch (gameType)
    {
        case MeType::ME1_TYPE:
        case MeType::ME2_TYPE:
            return "/CookedPC";
        case MeType::ME3_TYPE:
            return "/CookedPCConsole";
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

const QString GameData::RelativeGameData(const QString &path)
{
    if (_path == "")
        CRASH_MSG("Game path not set!");

    if (!path.contains(_path.toLower(), Qt::CaseInsensitive))
        CRASH_MSG("The path not found in game path!");

    return path.mid(_path.length());
}

void GameData::ClosePackagesList()
{
    packageFiles.clear();
    packageMainFiles.clear();
    packageDLCFiles.clear();
    tfcFiles.clear();
    packageUpperNames.clear();
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
