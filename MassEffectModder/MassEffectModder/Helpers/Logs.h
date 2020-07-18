/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2020 Pawel Kolodziejski
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

enum LOG_LEVEL {
    LOG_NONE,
    LOG_ERROR,
    LOG_INFO,
    LOG_DEBUG,
    LOG_MAX
};

#define LOG_CONSOLE       0x01
#define LOG_FILE          0x02
#define LOG_ERROR_BUFFER  0x04
#define LOG_ALL_OUTPUTS   (LOG_CONSOLE | LOG_FILE | LOG_ERROR_BUFFER)

class Logs
{
private:

    std::mutex      lock;
    qint64          startedTimestamp;
    int             logLevel;
    QString         logPath;
    QString         errorsString;

    bool            timeStampEnabled;
    bool            consoleEnabled;
    bool            fileEnabled;
    bool            errorBufferEnabled;

    void Print(int level, const QString &message, int flags);

public:

    Logs();
    void PrintCrash(const std::string &message);

    void PrintInfo(const QString &message);
    void PrintError(const QString &message);
    void PrintDebug(const QString &message);

    void ChangeLogLevel(LOG_LEVEL level);
    void BufferClearErrors();
    QString BufferGetErrors();
    void BufferEnableErrors(bool enable);
    void EnableOutputConsole(bool enable);
    void EnableOutputFile(const QString &path, bool enable);
    void EnableTimeStamp(bool enable);
    QString GetLogPath() { return logPath; }
};

extern Logs *g_logs;

bool CreateLogs();
void ReleaseLogs();

#define PINFO g_logs->PrintInfo
#define PERROR g_logs->PrintError
#define PDEBUG g_logs->PrintDebug

#endif
