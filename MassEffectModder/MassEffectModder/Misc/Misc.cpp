/*
 * MassEffectModder
 *
 * Copyright (C) 2018-2020 Pawel Kolodziejski
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

#include <Misc/Misc.h>
#include <GameData/GameData.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>

bool Misc::SetGameDataPath(MeType gameId, const QString &path)
{
    ConfigIni configIni;

    QString key = QString("ME%1").arg(static_cast<int>(gameId));
#if defined(_WIN32)
    configIni.Write(key, QString(path).replace(QChar('/'), QChar('\\'), Qt::CaseInsensitive), "GameDataPath");
#else
    configIni.Write(key, path, "GameDataPath");
#endif
    g_GameData->Init(gameId, configIni, true);
    if (!QFile(g_GameData->GameExePath()).exists())
    {
        PERROR("Error: Could not found the game!\n");
        return false;
    }

    PINFO("Game data found.\n");
    return true;
}

bool Misc::SetGameUserPath(MeType gameId, const QString &path)
{
    ConfigIni configIni;

    QString key = QString("ME%1").arg(static_cast<int>(gameId));
#if defined(_WIN32)
    configIni.Write(key, QString(path).replace(QChar('/'), QChar('\\'), Qt::CaseInsensitive), "GameUserPath");
#else
    configIni.Write(key, path, "GameUserPath");
#endif

    QString newPath = GameData::GameUserPath(gameId);
    if (newPath.length() == 0 || !QDir(newPath).exists())
    {
        PERROR("Error: Could not found game user config path!\n");
        return false;
    }

    PINFO("Game user config path changed.\n");
    return true;
}

bool Misc::CheckGamePath()
{
    if (g_GameData->GamePath().length() == 0 || !QDir(g_GameData->GamePath()).exists())
    {
        PERROR("Error: Could not found the game!\n");
        return false;
    }

    return true;
}

bool Misc::compareFileInfoPath(const QFileInfo &e1, const QFileInfo &e2)
{
    return e1.absoluteFilePath().compare(e2.absoluteFilePath(), Qt::CaseInsensitive) < 0;
}

bool Misc::ConvertEndLines(const QString &path, bool unixMode)
{
    auto inputFile = new QFile(path);
    if (!inputFile->open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    auto text = inputFile->readAll();
    delete inputFile;
    QTextStream streamIn(text);

    if (QFile(path).open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        auto fs = FileStream(path, FileMode::Create);
        while (!streamIn.atEnd())
        {
            auto line = streamIn.readLine();
            fs.WriteStringASCII(line);
            if (unixMode)
                fs.WriteStringASCII("\n");
            else
                fs.WriteStringASCII("\r\n");
        }
        return true;
    }
    return false;
}

bool Misc::checkWriteAccessDir(const QString &path)
{
    QFile file(path + "/test-mem-writefile");
    bool status = file.open(QIODevice::ReadWrite);
    if (status)
    {
        file.close();
        QFile(path + "/test-mem-writefile").remove();
    }
    return status;
}

bool Misc::checkWriteAccessFile(QString &path)
{
    QFile file(path);
    bool status = file.open(QIODevice::ReadWrite);
    return status;
}

bool Misc::isRunAsAdministrator()
{
    return DetectAdminRights();
}

bool Misc::CheckAndCorrectAccessToGame()
{
    if (!checkWriteAccessDir(g_GameData->MainData()))
    {
        PERROR(QString("MEM has not write access to game folders:\n") +
                     g_GameData->MainData() + "\n");
        return false;
    }

    return true;
}

quint64 Misc::getDiskFreeSpace(QString &path)
{
    return QStorageInfo(path).bytesFree();
}

quint64 Misc::getDirectorySize(QString &dir)
{
    return QFileInfo(dir).size();
}

QString Misc::getBytesFormat(quint64 size)
{
    if (size / 1024 == 0)
        return QString::number(size, 'f', 2) + " Bytes";
    if (size / 1024 / 1024 == 0)
        return QString::number(size / 1024.0, 'f', 2) + " KB";
    if (size / 1024 / 1024 / 1024 == 0)
        return  QString::number(size / 1024.0 / 1024.0, 'f', 2) + " MB";
    return  QString::number(size / 1024.0 / 1024 / 1024.0, 'f', 2) + " GB";
}

static QElapsedTimer timer;
void Misc::startTimer()
{
    timer.start();
}

long Misc::elapsedTime()
{
    return timer.elapsed();
}

QString Misc::getTimerFormat(long time)
{
    if (time / 1000 == 0)
        return QString("%1 milliseconds").arg(time);
    if (time / 1000 / 60 == 0)
        return QString("%1 seconds").arg(time / 1000);
    if (time / 1000 / 60 / 60 == 0)
        return QString("%1 min - %2 sec").arg(time / 1000 / 60).arg(time / 1000 % 60);

    long hours = time / 1000 / 60 / 60;
    long minutes = (time - (hours * 1000 * 60 * 60)) / 1000 / 60;
    long seconds = (time - (hours * 1000 * 60 * 60) - (minutes * 1000 * 60)) / 1000 / 60;
    return QString("%1 hours - %2 min - %3 sec").arg(hours).arg(minutes).arg(seconds);
}
