/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2019 Pawel Kolodziejski
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
        startedTimestamp(0),
        logLevel(LOG_NONE),
        errorsString(""),
        timeStampEnabled(false),
        consoleEnabled(false),
        fileEnabled(false),
        errorBufferEnabled(false)
{
    startedTimestamp = QDateTime::currentMSecsSinceEpoch();
}

void Logs::ChangeLogLevel(LOG_LEVEL level)
{
    logLevel = level;
}

void Logs::ClearErrors()
{
    errorsString = "";
}

void Logs::EnableOutputConsole(bool enable)
{
    consoleEnabled = enable;
}

void Logs::EnableOutputFile(bool enable)
{
    fileEnabled = enable;
}

void Logs::EnableTimeStamp(bool enable)
{
    timeStampEnabled = enable;
}

void Logs::EnableErrorsBuffer(bool enable)
{
    errorBufferEnabled = enable;
}

void Logs::Print(int level, const QString &message, int flags)
{
    if (logLevel < level)
        return;

    qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    QString timestampStr;

    lock.lock();

    if (errorBufferEnabled && flags & LOG_ERROR_BUFFER)
        errorsString += message;

    if (timeStampEnabled)
        timestampStr = QString("[") + (startedTimestamp - timestamp) + "] ";

    if (consoleEnabled&& flags & LOG_CONSOLE)
    {
#if defined(_WIN32)
        ::_fputws((timestampStr + message).toStdWString().c_str(), stdout);
#else
        std::fputs((timestampStr + message).toStdString().c_str(), stdout);
#endif
    }

    if (fileEnabled && flags & LOG_FILE)
    {
        FILE *file = fopen("Log.txt", "a");
        if (file)
        {
#if defined(_WIN32)
            std::fputws((timestampStr + message).toStdWString().c_str(), file);
#else
            std::fputs((timestampStr + message).toStdString().c_str(), file);
#endif
            fclose(file);
        }
    }

    lock.unlock();
}

void Logs::PrintCrash(const std::string &message)
{
    Print(LOG_MAX, QString(message.c_str()), LOG_ALL_OUTPUTS);
}

void Logs::PrintError(const QString &message)
{
    Print(LOG_ERROR, message, LOG_ALL_OUTPUTS);
}

void Logs::PrintInfo(const QString &message)
{
    Print(LOG_INFO, message, LOG_ALL_OUTPUTS);
}

void Logs::PrintDebug(const QString &message)
{
    Print(LOG_DEBUG, message, LOG_ALL_OUTPUTS);
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
