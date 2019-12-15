/* The MIT License
 *
 * Copyright (c) 2017-2019 Pawel Kolodziejski
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
#if defined(_WIN32)
    FILE *file = _wfopen(logPath.toStdWString().c_str(), L"w");
#else
    FILE *file = fopen(logPath.toStdString().c_str(), "w");
#endif
    if (file)
        fclose(file);
    fileEnabled = enable;
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
        std::fputs((timestampStr + message).toStdString().c_str(), stdout);
#endif
    }

    if (fileEnabled && (flags & LOG_FILE))
    {
#if defined(_WIN32)
        FILE *file = _wfopen(logPath.toStdWString().c_str(), L"a");
#else
        FILE *file = fopen(logPath.toStdString().c_str(), "a");
#endif
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
