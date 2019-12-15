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
