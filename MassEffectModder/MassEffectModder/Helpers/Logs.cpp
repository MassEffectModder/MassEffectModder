/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2019 Pawel Kolodziejski <aquadran at users.sourceforge.net>
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

#include "Logs.h"

Logs::Logs() :
        _startedTimestamp(0), _timeStampEnabled(false), _printToConsoleEnabled(false)
{
    _startedTimestamp = QDateTime::currentMSecsSinceEpoch();
}

void Logs::print(const QString &message)
{
    qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    QString timestampStr;

    _lock.lock();

    if (_timeStampEnabled)
        timestampStr = QString("[") + (_startedTimestamp - timestamp) + "] ";

    if (_printToConsoleEnabled)
        std::puts((timestampStr + message).toStdString().c_str());

    FILE *file = fopen("Log.txt", "a");
    if (file)
    {
        std::fputs((timestampStr + message + "\n").toStdString().c_str(), file);
        fclose(file);
    }

    _lock.unlock();
}

void Logs::printStdMsg(const std::string &message)
{
    printMsg(QString(message.c_str()));
}

void Logs::printMsgTimeStamp(const QString &message)
{
    bool oldState = _timeStampEnabled;
    _timeStampEnabled = true;
    this->print(message);
    _timeStampEnabled = oldState;
}

void Logs::printMsg(const QString &message)
{
    bool oldState = _timeStampEnabled;
    _timeStampEnabled = false;
    this->print(message);
    _timeStampEnabled = oldState;
}

Logs *g_logs;

bool CreateLogs()
{
    g_logs = new Logs();
    if (g_logs == nullptr)
    {
        std::fputs("CreateLogs: Failed create instance\n", stderr);
        return false;
    }
    return true;
}

void ReleaseLogs()
{
    delete g_logs;
    g_logs = nullptr;
}
