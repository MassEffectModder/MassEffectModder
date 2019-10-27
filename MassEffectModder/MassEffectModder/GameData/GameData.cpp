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

#include <GameData/GameData.h>
#include <Helpers/MiscHelpers.h>

MeType GameData::gameType = UNKNOWN_TYPE;

bool comparePath(const QString &e1, const QString &e2)
{
    return e1.compare(e2, Qt::CaseInsensitive) < 0;
}

void GameData::ScanGameFiles(bool force, const QString &filterPath)
{
    if (force)
        ClosePackagesList();

    if (packageFiles.count() == 0)
    {
        QDirIterator iterator(MainData(), QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
        int pathLen = _path.length();
        while (iterator.hasNext())
        {
#ifdef GUI
            QApplication::processEvents();
#endif
            iterator.next();
            QString path = iterator.filePath().mid(pathLen);
            if (gameType == MeType::ME1_TYPE)
            {
                if (path.endsWith("localshadercache-pc-d3d-sm3.upk", Qt::CaseInsensitive))
                    continue;
                if (path.endsWith("refshadercache-pc-d3d-sm3.upk", Qt::CaseInsensitive))
                    continue;
                if (path.compare("niebieska_pl.bik") == 0)
                    FullScanGame = true;
                if (path.endsWith(".bik", Qt::CaseInsensitive))
                    bikFiles.push_back(path);
            }
            else if (gameType == MeType::ME2_TYPE)
            {
                if (path.endsWith(".afc", Qt::CaseInsensitive))
                    afcFiles.push_back(path);
                else if (path.endsWith(".tfc", Qt::CaseInsensitive))
                    tfcFiles.push_back(path);
            }
            else if (gameType == MeType::ME3_TYPE)
            {
                if (path.contains("guidcache", Qt::CaseInsensitive))
                    continue;
                if (path.endsWith("Coalesced.bin", Qt::CaseInsensitive))
                    coalescedFiles.push_back(path);
                else if (path.endsWith(".afc", Qt::CaseInsensitive))
                    afcFiles.push_back(path);
                else if (path.endsWith(".tfc", Qt::CaseInsensitive))
                    tfcFiles.push_back(path);
            }
            mainFiles.push_back(path);
        };

        if (gameType == MeType::ME2_TYPE)
        {
            QString iniPath = bioGamePath() + "/Config/PC/Cooked/Coalesced.ini";
            QString path = iniPath.mid(pathLen);
            if (QFile::exists(iniPath))
            {
                coalescedFiles.push_back(path);
            }
        }
        if (gameType == MeType::ME3_TYPE)
        {
            QString patchPath = bioGamePath() + "/Patches/PCConsole/Patch_001.sfar";
            QString path = patchPath.mid(pathLen);
            if (QFile::exists(patchPath))
            {
                sfarFiles.push_back(path);
            }
        }
        if (gameType == MeType::ME2_TYPE ||
            gameType == MeType::ME3_TYPE)
        {
            QDirIterator iterator(bioGamePath() + "/Movies", QDir::Files | QDir::NoSymLinks);
            while (iterator.hasNext())
            {
                iterator.next();
                QString path = iterator.filePath().mid(pathLen);
                if (path.endsWith(".bik", Qt::CaseInsensitive))
                {
                    bikFiles.push_back(path);
                }
            }
        }

        if (QDir(DLCData()).exists())
        {
            QStringList DLCs = QDir(DLCData(), "DLC_*", QDir::NoSort, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryList();
            foreach (QString DLCDir, DLCs)
            {
                QStringList files;
                QDirIterator iterator(DLCData() + "/" + DLCDir + DLCDataSuffix(), QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
                bool isValid = true;
                if (gameType == MeType::ME3_TYPE)
                    isValid = false;
                while (iterator.hasNext())
                {
#ifdef GUI
                    QApplication::processEvents();
#endif
                    iterator.next();
                    QString path = iterator.filePath().mid(pathLen);
                    if (filterPath != "" && !path.contains(filterPath, Qt::CaseInsensitive))
                        continue;
                    if (gameType == MeType::ME3_TYPE)
                    {
                        if (path.contains("guidcache", Qt::CaseInsensitive))
                            continue;
                        if (path.endsWith(".sfar", Qt::CaseInsensitive))
                            isValid = true;
                    }
                    files.push_back(path);
                }

                if (isValid)
                {
                    QDirIterator iterator2(DLCData() + "/" + DLCDir + "/Movies", QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
                    while (iterator2.hasNext())
                    {
#ifdef GUI
                        QApplication::processEvents();
#endif
                        iterator2.next();
                        QString path = iterator2.filePath().mid(pathLen);
                        if (path.endsWith(".bik", Qt::CaseInsensitive))
                        {
                            bikFiles.push_back(path);
                        }
                    }
                    DLCFiles += files;
                }
            }
        }

        if (gameType == MeType::ME2_TYPE)
        {
            for (int i = 0; i < DLCFiles.count(); i++)
            {
                if (DLCFiles[i].endsWith(".tfc", Qt::CaseInsensitive))
                    tfcFiles.push_back(DLCFiles[i]);
                else if (DLCFiles[i].endsWith(".afc", Qt::CaseInsensitive))
                    afcFiles.push_back(DLCFiles[i]);
            }
        }
        else if (gameType == MeType::ME3_TYPE)
        {
            for (int i = 0; i < DLCFiles.count(); i++)
            {
                if (DLCFiles[i].endsWith(".tfc", Qt::CaseInsensitive))
                    tfcFiles.push_back(DLCFiles[i]);
                else if (DLCFiles[i].endsWith(".afc", Qt::CaseInsensitive))
                    afcFiles.push_back(DLCFiles[i]);
                else if (DLCFiles[i].endsWith(".sfar", Qt::CaseInsensitive))
                    sfarFiles.push_back(DLCFiles[i]);
            }
        }
        std::sort(tfcFiles.begin(), tfcFiles.end(), comparePath);

        if (gameType == MeType::ME1_TYPE)
        {
            for (int i = 0; i < mainFiles.count(); i++)
            {
                if (filterPath != "" && !mainFiles[i].contains(filterPath, Qt::CaseInsensitive))
                    continue;
                if (mainFiles[i].endsWith(".u", Qt::CaseInsensitive) ||
                    mainFiles[i].endsWith(".upk", Qt::CaseInsensitive) ||
                    mainFiles[i].endsWith(".sfm", Qt::CaseInsensitive))
                {
                    packageFiles += mainFiles[i];
                }
            }
            for (int i = 0; i < DLCFiles.count(); i++)
            {
                if (filterPath != "" && !DLCFiles[i].contains(filterPath, Qt::CaseInsensitive))
                    continue;
                if (DLCFiles[i].endsWith(".u", Qt::CaseInsensitive) ||
                    DLCFiles[i].endsWith(".upk", Qt::CaseInsensitive) ||
                    DLCFiles[i].endsWith(".sfm", Qt::CaseInsensitive))
                {
                    packageFiles += DLCFiles[i];
                }
            }
        }
        else
        {
            for (int i = 0; i < mainFiles.count(); i++)
            {
                if (filterPath != "" && !mainFiles[i].contains(filterPath, Qt::CaseInsensitive))
                    continue;
                if (mainFiles[i].endsWith(".pcc", Qt::CaseInsensitive))
                {
                    packageFiles += mainFiles[i];
                }
            }
            for (int i = 0; i < DLCFiles.count(); i++)
            {
                if (filterPath != "" && !DLCFiles[i].contains(filterPath, Qt::CaseInsensitive))
                    continue;
                if (DLCFiles[i].endsWith(".pcc", Qt::CaseInsensitive))
                {
                    packageFiles += DLCFiles[i];
                }
            }
        }

        std::sort(packageFiles.begin(), packageFiles.end(), comparePath);

        if (gameType == MeType::ME1_TYPE)
        {
            for (int i = 0; i < packageFiles.count(); i++)
            {
                mapME1PackageUpperNames.insert(BaseNameWithoutExt(packageFiles[i]).toUpper(), i);
                packageME1UpperNames += BaseNameWithoutExt(packageFiles[i]).toUpper();
            }
            std::sort(packageME1UpperNames.begin(), packageME1UpperNames.end(), comparePath);
        }

        if (gameType == MeType::ME1_TYPE && !FullScanGame)
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
                    FullScanGame = true;
                    break;
                }
            }
            for (int i = 0; i < mainFiles.count(); i++)
            {
                QString path = mainFiles[i];
                if (path.contains("_DE.", Qt::CaseInsensitive) ||
                    path.contains("_FR.", Qt::CaseInsensitive) ||
                    path.contains("_IT.", Qt::CaseInsensitive) ||
                    path.contains("_ES.", Qt::CaseInsensitive))
                {
                    FullScanGame = true;
                    break;
                }
            }
        }
    }
}

void GameData::Init(MeType type)
{
    gameType = type;
}

void GameData::Init(MeType type, ConfigIni &configIni)
{
    InternalInit(type, configIni, false);
    ScanGameFiles(false, "");
}

void GameData::Init(MeType type, ConfigIni &configIni, const QString &filterPath)
{
    InternalInit(type, configIni, false);
    ScanGameFiles(false, filterPath);
}

void GameData::Init(MeType type, ConfigIni &configIni, bool force = false)
{
    InternalInit(type, configIni, force);
    ScanGameFiles(force, "");
}

void GameData::InternalInit(MeType type, ConfigIni &configIni, bool force)
{
    gameType = type;

    QString key = QString("ME%1").arg(static_cast<int>(gameType));
    QString path = configIni.Read(key, "GameDataPath");
    if (path.length() != 0 && !force)
    {
        _path = QDir::cleanPath(path);
        if (QFile(GameExePath()).exists())
        {
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
    if (path.length() != 0 && !force)
    {
        _path = QDir::cleanPath(path);
        if (QFile(GameExePath()).exists())
        {
            configIni.Write(key, _path.replace(QChar('/'), QChar('\\'), Qt::CaseInsensitive), "GameDataPath");
            return;
        }
        _path = "";
    }
#endif

    _path = QDir::cleanPath(path);
    if (_path.length() != 0 && QFile(GameExePath()).exists())
    {
#if defined(_WIN32)
        configIni.Write(key, _path.replace(QChar('/'), QChar('\\'), Qt::CaseInsensitive), "GameDataPath");
#else
        configIni.Write(key, _path, "GameDataPath");
#endif
    }
}

const QString GameData::bioGamePath()
{
    if (_path.length() == 0)
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
    if (_path.length() == 0)
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
    if (_path.length() == 0)
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

const QString GameData::GameUserPath(MeType type)
{
#if defined(_WIN32)
    QString path = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first() + "/BioWare/Mass Effect";
    if (type == MeType::ME2_TYPE)
        path += " 2";
    else if (type == MeType::ME3_TYPE)
        path += " 3";
#else
    ConfigIni configIni;
    QString key = QString("ME%1").arg(static_cast<int>(type));
    QString path = configIni.Read(key, "GameUserPath");
    if (!QDir(path).exists())
        path = "";
#endif

    return path;
}

const QString GameData::ConfigIniPath(MeType type)
{
    QString path = GameUserPath(type);
    if (path == "")
        return path;
    switch (type)
    {
        case MeType::ME1_TYPE:
            return path + "/Config";
        case MeType::ME2_TYPE:
        case MeType::ME3_TYPE:
            return path + "/BioGame/Config";
        case MeType::UNKNOWN_TYPE:
            CRASH();
    }

    CRASH();
}

const QString GameData::EngineConfigIniPath(MeType type)
{
    QString path = ConfigIniPath(type);
    if (path == "")
        return path;
    switch (type)
    {
        case MeType::ME1_TYPE:
            return path + "/BIOEngine.ini";
        case MeType::ME2_TYPE:
        case MeType::ME3_TYPE:
            return path + "/GamerSettings.ini";
        case MeType::UNKNOWN_TYPE:
            CRASH();
    }

    CRASH();
}

const QString GameData::RelativeGameData(const QString &path)
{
    if (_path.length() == 0)
        CRASH_MSG("Game path not set!");

    if (!path.contains(_path.toLower(), Qt::CaseInsensitive))
        return path;

    return path.mid(_path.length());
}

void GameData::ClosePackagesList()
{
    packageFiles.clear();
    mainFiles.clear();
    DLCFiles.clear();
    tfcFiles.clear();
    coalescedFiles.clear();
    afcFiles.clear();
    bikFiles.clear();
    mapME1PackageUpperNames.clear();
    packageME1UpperNames.clear();
}

GameData *g_GameData;

bool CreateGameData()
{
    g_GameData = new GameData();
    if (g_GameData == nullptr)
    {
        std::fputs("CreateLogs: Failed create instance.\n", stderr);
        return false;
    }
    return true;
}

void ReleaseGameData()
{
    delete g_GameData;
    g_GameData = nullptr;
}
