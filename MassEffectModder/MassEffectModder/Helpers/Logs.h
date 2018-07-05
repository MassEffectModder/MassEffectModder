/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2018 Pawel Kolodziejski <aquadran at users.sourceforge.net>
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

#ifndef LOGS_H
#define LOGS_H

#include <mutex>
#include <cstring>

class QString;

class Logs
{
private:

    std::mutex      _lock;
    qint64          _startedTimestamp;
    bool            _timeStampEnabled;
    bool            _printToConsoleEnabled;

    void print(const QString &message);

public:

    Logs();
    void printStdMsg(const std::string &message);
    void printMsg(const QString &message);
    void printMsgTimeStamp(const QString &message);
    void enableTimeStamp(bool enable) { _timeStampEnabled = enable; }
};

extern Logs *g_logs;

bool CreateLogs();
void ReleaseLogs();

#endif
