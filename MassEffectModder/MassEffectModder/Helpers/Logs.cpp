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

#include "Logs.h"

#if defined(_WIN32)
#include <fcntl.h>
#endif

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

void Logs::BufferClearErrors()
{
    errorsString = "";
}

QString Logs::BufferGetErrors()
{
    return errorsString;
}

void Logs::EnableOutputConsole(bool enable)
{
    consoleEnabled = enable;
}

void Logs::EnableOutputFile(const QString &path, bool enable)
{
    logPath = path;
    if (!enable)
    {
        fileEnabled = enable;
        return;
    }
#if defined(_WIN32)
    FILE *file = _wfopen(logPath.toStdWString().c_str(), L"w");
#else
    FILE *file = fopen(logPath.toUtf8().data(), "w");
#endif
    if (file) {
#if defined(_WIN32)
        unsigned char bom[] = { 0xFF,0xFE };
        fwrite(bom, 1, sizeof(bom), file);
#endif
        /*unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
        fwrite(bom, 1, sizeof(bom), file);*/
        fclose(file);
        fileEnabled = enable;
    }
}

void Logs::EnableTimeStamp(bool enable)
{
    timeStampEnabled = enable;
}

void Logs::BufferEnableErrors(bool enable)
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

    if (errorBufferEnabled && (flags & LOG_ERROR_BUFFER) && level == LOG_ERROR)
        errorsString += message;

    if (timeStampEnabled)
        timestampStr = QString("[") + (startedTimestamp - timestamp) + "] ";

    if (consoleEnabled && (flags & LOG_CONSOLE))
    {
#if defined(_WIN32)
        std::fputws((timestampStr + message).toStdWString().c_str(), stdout);
#else
        std::fputs((timestampStr + message).toUtf8().data(), stdout);
#endif
    }

    if (fileEnabled && (flags & LOG_FILE))
    {
#if defined(_WIN32)
        FILE *file = _wfopen(logPath.toStdWString().c_str(), L"a");
#else
        FILE *file = fopen(logPath.toUtf8().data(), "a");
#endif
        if (file)
        {
#if defined(_WIN32)
#define _O_U16TEXT  0x20000
            _setmode(_fileno(file), _O_U16TEXT);
            std::fputws((timestampStr + message).toStdWString().c_str(), file);
#else
            std::fputs((timestampStr + message).toUtf8().data(), file);
#endif
            fclose(file);
        }
    }

    lock.unlock();
}

void Logs::PrintCrash(const std::string &message)
{
    Print(LOG_NONE, QString(message.c_str()), LOG_ALL_OUTPUTS);
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
#if defined(_WIN32)
        std::fputws(L"CreateLogs: Failed create instance\n", stderr);
#else
        std::fputs("CreateLogs: Failed create instance\n", stderr);
#endif
        return false;
    }
    return true;
}

void ReleaseLogs()
{
    delete g_logs;
    g_logs = nullptr;
}
