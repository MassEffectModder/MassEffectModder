/*
 * MassEffectModder
 *
 * Copyright (C) 2018-2021 Pawel Kolodziejski
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
#include <Helpers/Exception.h>
#include <Helpers/MiscHelpers.h>

MeType GameData::gameType = UNKNOWN_TYPE;

bool comparePath(const QString &e1, const QString &e2)
{
    return AsciiStringCompareCaseIgnore(e1, e2) < 0;
}

void GameData::ScanGameFiles(bool force, const QString &filterPath)
{
    if (force)
        ClosePackagesList();

    if (_path == "")
    {
        ClosePackagesList();
        return;
    }

    if (packageFiles.count() == 0)
    {
#ifdef GUI
        QElapsedTimer timer;
        timer.start();
#endif
        QStringList files;
        QDirIterator iterator(MainData(), QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
        int pathLen = _path.length();
        while (iterator.hasNext())
        {
#ifdef GUI
            if (timer.elapsed() > 100)
            {
                QApplication::processEvents();
                timer.restart();
            }
#endif
            iterator.next();
            files.append(iterator.filePath().mid(pathLen));
        }

        for (int f = 0; f < files.count(); f++)
        {
#ifdef GUI
            if (timer.elapsed() > 100)
            {
                QApplication::processEvents();
                timer.restart();
            }
#endif
            const QString &path = files[f];
            if (AsciiStringEndsWith(files[f], EXTENSION_TFC, EXTENSION_TFC_LEN))
            {
                tfcFiles.push_back(path);
                continue;
            }
            if (AsciiStringEndsWith(files[f], GLOBALPERIST, GLOBALPERIST_LEN))
                continue;
            if (AsciiStringEndsWith(files[f], GUIDCACHE_PCC, GUIDCACHE_PCC_LEN))
                continue;
            if (AsciiBaseNameStringStartsWith(files[f], GUIDCACHE, GUIDCACHE_LEN))
                continue;
            if (AsciiBaseNameStringStartsWith(files[f], COALESCED, COALESCED_LEN))
            {
                othersFiles.push_back(path);
                continue;
            }

            if (gameType == MeType::ME2_TYPE ||
                gameType == MeType::ME3_TYPE)
            {
                if (AsciiStringEndsWith(files[f], EXTENSION_AFC, EXTENSION_AFC_LEN))
                {
                    othersFiles.push_back(path);
                    continue;
                }
                if (AsciiStringEndsWith(files[f], EXTENSION_TLK, EXTENSION_TLK_LEN))
                {
                    othersFiles.push_back(path);
                    continue;
                }
            }
            mainFiles.push_back(path);
        };

        QString splashPath = bioGamePath() + "/Splash/PC/Splash.bmp";
        QString path = splashPath.mid(pathLen);
        if (QFile::exists(splashPath))
        {
            othersFiles.push_back(path);
        }

        QDirIterator iterator2(_path + "/Game/ME" + QString::number((int)gameType) + "/Engine/Shaders", QDir::Files | QDir::NoSymLinks);
        while (iterator2.hasNext())
        {
            iterator2.next();
            QString path = iterator2.filePath().mid(pathLen);
            if (AsciiStringEndsWith(path, EXTENSION_USF, EXTENSION_USF_LEN))
            {
                othersFiles.push_back(path);
            }
        }

        QDirIterator iterator3(bioGamePath() + "/Movies", QDir::Files | QDir::NoSymLinks);
        while (iterator3.hasNext())
        {
            iterator3.next();
            QString path = iterator3.filePath().mid(pathLen);
            if (AsciiStringEndsWith(path, EXTENSION_BIK, EXTENSION_BIK_LEN))
            {
                othersFiles.push_back(path);
            }
        }

        QDirIterator iterator4(bioGamePath() + "/Content/Packages/ISACT", QDir::Files | QDir::NoSymLinks);
        while (iterator4.hasNext())
        {
            iterator4.next();
            QString path = iterator4.filePath().mid(pathLen);
            if (AsciiStringEndsWith(path, EXTENSION_ISB, EXTENSION_ISB_LEN))
            {
                othersFiles.push_back(path);
            }
        }

        if (QDir(DLCData()).exists())
        {
            QStringList DLCs = QDir(DLCData(), "DLC_*", QDir::NoSort, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryList();
            foreach (QString DLCDir, DLCs)
            {
                QStringList files;
                QDirIterator iterator(DLCData() + "/" + DLCDir + DLCDataSuffix(), QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
                bool isValid = false;
                while (iterator.hasNext())
                {
#ifdef GUI
                    if (timer.elapsed() > 100)
                    {
                        QApplication::processEvents();
                        timer.restart();
                    }
#endif
                    iterator.next();
                    if (iterator.filePath().endsWith("Mount.dlc", Qt::CaseInsensitive))
                    {
                        isValid = true;
                    }
                    QString path = iterator.filePath().mid(pathLen);
                    if (filterPath.length() != 0 && !path.contains(filterPath, Qt::CaseInsensitive))
                        continue;
                    if (AsciiBaseNameStringStartsWith(path, GUIDCACHE, GUIDCACHE_LEN))
                        continue;
                    files.push_back(path);
                }

                if (isValid)
                {
                    QDirIterator iterator2(DLCData() + "/" + DLCDir + "/Movies", QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
                    while (iterator2.hasNext())
                    {
                        iterator2.next();
                        QString path = iterator2.filePath().mid(pathLen);
                        if (AsciiStringEndsWith(path, EXTENSION_BIK, EXTENSION_BIK_LEN))
                        {
                            othersFiles.push_back(path);
                        }
                    }
                    DLCFiles += files;
                }
            }

            if (gameType == MeType::ME2_TYPE)
            {
                for (int i = 0; i < DLCFiles.count(); i++)
                {
                    if (AsciiStringEndsWith(DLCFiles[i], EXTENSION_TFC, EXTENSION_TFC_LEN))
                        tfcFiles.push_back(DLCFiles[i]);
                    else if (AsciiStringEndsWith(DLCFiles[i], EXTENSION_AFC, EXTENSION_AFC_LEN))
                        othersFiles.push_back(DLCFiles[i]);
                    else if (AsciiStringEndsWith(DLCFiles[i], EXTENSION_TLK, EXTENSION_TLK_LEN))
                        othersFiles.push_back(DLCFiles[i]);
                    else if (AsciiStringEndsWith(DLCFiles[i], EXTENSION_INI, EXTENSION_INI_LEN))
                        othersFiles.push_back(DLCFiles[i]);
                }
            }
            else if (gameType == MeType::ME3_TYPE)
            {
                for (int i = 0; i < DLCFiles.count(); i++)
                {
                    if (AsciiStringEndsWith(DLCFiles[i], EXTENSION_TFC, EXTENSION_TFC_LEN))
                        tfcFiles.push_back(DLCFiles[i]);
                    else if (AsciiStringEndsWith(DLCFiles[i], EXTENSION_AFC, EXTENSION_AFC_LEN))
                        othersFiles.push_back(DLCFiles[i]);
                    else if (AsciiStringEndsWith(DLCFiles[i], EXTENSION_TLK, EXTENSION_TLK_LEN))
                        othersFiles.push_back(DLCFiles[i]);
                }
            }
        }

        QDirIterator iterator5(_path + "/Game/Launcher", QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
        while (iterator5.hasNext())
        {
            iterator5.next();
            QString path = iterator5.filePath().mid(pathLen);
            if (AsciiStringEndsWith(path, EXTENSION_EXE, EXTENSION_EXE_LEN))
                continue;
            if (AsciiStringEndsWith(path, EXTENSION_DLL, EXTENSION_DLL_LEN))
                continue;
            othersFiles.push_back(path);
        }

        std::sort(tfcFiles.begin(), tfcFiles.end(), comparePath);

        for (int i = 0; i < mainFiles.count(); i++)
        {
            if (filterPath.length() != 0 && !mainFiles[i].contains(filterPath, Qt::CaseInsensitive))
                continue;
            if (AsciiStringEndsWith(mainFiles[i], EXTENSION_PCC, EXTENSION_PCC_LEN))
            {
                packageFiles += mainFiles[i];
            }
        }
        for (int i = 0; i < DLCFiles.count(); i++)
        {
            if (filterPath.length() != 0 && !DLCFiles[i].contains(filterPath, Qt::CaseInsensitive))
                continue;
            if (AsciiStringEndsWith(DLCFiles[i], EXTENSION_PCC, EXTENSION_PCC_LEN))
            {
                packageFiles += DLCFiles[i];
            }
        }

        std::sort(packageFiles.begin(), packageFiles.end(), comparePath);
    }
    g_GameData->FullScanGame = true;
}

void GameData::Init(MeType type)
{
    gameType = type;
}

void GameData::Init(MeType type, ConfigIni &configIni)
{
    InternalInit(type, configIni);
    ScanGameFiles(false, "");
}

void GameData::Init(MeType type, ConfigIni &configIni, const QString &filterPath)
{
    InternalInit(type, configIni);
    ScanGameFiles(false, filterPath);
}

void GameData::Init(MeType type, ConfigIni &configIni, bool force = false)
{
    InternalInit(type, configIni);
    ScanGameFiles(force, "");
}

void GameData::InternalInit(MeType type, ConfigIni &configIni)
{
    gameType = type;

    QString path = configIni.Read("MELE", "GameDataPath");
    if (path.length() != 0)
    {
        _path = QDir::cleanPath(path);
        if (QFile(GameExePath()).exists())
        {
            return;
        }
        _path = "";
    }

#if defined(_WIN32)
    QString registryKey = R"(HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\BioWare\Mass Effect Legendary Edition)";
    QString entry = "Install Dir";

    QSettings settings(registryKey, QSettings::NativeFormat);
    path = settings.value(entry, "").toString();
    if (path.length() != 0)
    {
        _path = QDir::cleanPath(path);
        if (QFile(GameExePath()).exists())
        {
            auto exeVersion = getVersionString(GameExePath());
            bool properVersion = false;
            switch (type)
            {
            case MeType::ME1_TYPE:
                if (exeVersion == "2.0.0.47902")
                    properVersion = true;
                break;
            case MeType::ME2_TYPE:
                if (exeVersion == "2.0.0.47902")
                    properVersion = true;
                break;
            case MeType::ME3_TYPE:
                if (exeVersion == "2.0.0.47902")
                    properVersion = true;
                break;
            case MeType::UNKNOWN_TYPE:
                break;
            }
            if (properVersion)
            {
                configIni.Write(key, _path.replace(QChar('/'), QChar('\\'), Qt::CaseInsensitive), "GameDataPath");
                return;
            }
        }
        _path = "";
    }
#endif

    _path = QDir::cleanPath(path);
    if (_path.length() != 0 && QFile(GameExePath()).exists())
    {
#if defined(_WIN32)
        configIni.Write("MELE", _path.replace(QChar('/'), QChar('\\'), Qt::CaseInsensitive), "GameDataPath");
#else
        configIni.Write("MELE", _path, "GameDataPath");
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
        case MeType::ME3_TYPE:
            return _path + "/Game/ME" + QString::number((int)gameType) + "/BioGame";
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
        case MeType::ME2_TYPE:
        case MeType::ME3_TYPE:
        return _path + "/Game/ME" + QString::number((int)gameType) +
                "/Binaries/Win64/MassEffect" + QString::number((int)gameType) + ".exe";
        case MeType::UNKNOWN_TYPE:
            CRASH();
    }

    CRASH();
}

const QString GameData::GameUserPath()
{
#if defined(_WIN32)
    QString path = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first() + "/BioWare/Mass Effect Legendary Edition";
#else
    ConfigIni configIni;
    QString path = configIni.Read("MELE", "GameUserPath");
    if (!QDir(path).exists())
        path = "";
#endif

    return path;
}

const QString GameData::ConfigIniPath(MeType type)
{
    if (_path.length() == 0)
        CRASH_MSG("Game path not set!");

    switch (type)
    {
        case MeType::ME1_TYPE:
        case MeType::ME2_TYPE:
        case MeType::ME3_TYPE:
            return bioGamePath() + "/Config";
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
    othersFiles.clear();
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
