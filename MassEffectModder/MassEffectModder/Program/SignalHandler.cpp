/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2021 Pawel Kolodziejski
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

#include <string>
#include <iostream>
#include <csignal>

#include <Helpers/Logs.h>
#include <Helpers/MiscHelpers.h>
#include <Wrappers.h>

using namespace std;

#define MAX_FILE_PATH 1024

void LogCrash(string output, string &message)
{
#ifdef GUI
    QString error = QString::fromStdString(message).replace("\n", "<br>") + "<br>" +
            "Callstack for the crash provided after press 'Show Details'.<br><br>";
    if (g_logs != nullptr)
        error += "Program log provided in the file: <br>'" + g_logs->GetLogPath() + "'";
    QMessageBox msgBox;
    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText(error);
    msgBox.setStandardButtons(QMessageBox::Close);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setDetailedText(QString::fromStdString(output));
    msgBox.setWindowModality(Qt::ApplicationModal);
    auto *spacer = new QSpacerItem(800, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    auto *layout = dynamic_cast<QGridLayout *>(msgBox.layout());
    layout->addItem(spacer, layout->rowCount(), 0, 1, layout->columnCount());
    msgBox.exec();
#endif

    output = "\n" + message + "\n" + output;

    if (g_logs)
    {
        g_logs->PrintCrash(output);
    }
    else
        cerr << output;
}

static void getFilename(char *dst, const char *src)
{
    long offset = 0;
    for (auto *ptr = src; *ptr != 0 && (ptr - src < MAX_FILE_PATH); ptr++)
    {
        if (*ptr == '/' || *ptr == '\\')
            offset = ptr - src + 1;
    }
    strncpy(dst, src + offset, MAX_FILE_PATH - 1);
}

#ifdef NDEBUG
[[ noreturn ]]
#endif
void Exception(const char *file, const char *func, int line, const char *msg)
{
    char str[MAX_FILE_PATH];
    getFilename(str, file);

    if (g_ipc)
    {
        string messageIpc = "[IPC]EXCEPTION_OCCURRED ";
        if (msg)
            messageIpc += "\"" + std::string(msg) + "\" ";
        messageIpc += std::string(func) + " at " + std::string(str) + ": line " + std::to_string(line);
        ConsoleWrite(messageIpc.c_str());
        ConsoleSync();
    }

    string messageStd = "Exception occurred!\n";
    if (msg)
        messageStd += "\n\"" + std::string(msg) + "\"\n";
    messageStd += "\n" + std::string(func) + " at " + std::string(str) + ": line " + std::to_string(line) + "\n";

    string output = "Callstack:\n";
    GetBackTrace(output, true, false);

    LogCrash(output, messageStd);

#ifdef NDEBUG
    exit(1);
#endif
}

#ifdef NDEBUG

[[ noreturn ]] static void SignalsHandler(int signal)
{
    string output, message;
    bool crashed = false;

    switch (signal)
    {
    case SIGSEGV:
    case SIGABRT:
    case SIGILL:
    case SIGFPE:
        message = "Program crashed!\n";
        crashed = true;
        break;
    default:
        message = "Program stopped.\n";
        break;
    }

    if (crashed)
    {
        string output = "Backtrace:\n";
        GetBackTrace(output, false, true);
        LogCrash(output, message);
    }

    exit(1);
}

#endif

void InstallSignalsHandler()
{
#ifdef NDEBUG
    signal(SIGSEGV, SignalsHandler);
    signal(SIGABRT, SignalsHandler);
    signal(SIGILL,  SignalsHandler);
    signal(SIGFPE,  SignalsHandler);
    signal(SIGINT,  SignalsHandler);
    signal(SIGTERM, SignalsHandler);
#endif
}
