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

#include <string>
#include <iostream>
#include <csignal>

#include <Helpers/Logs.h>
#include <Exceptions/Backtrace.h>

using namespace std;

void LogCrash(string output, string &message)
{
    output = message + output;

    if (g_logs)
    {
        g_logs->consoleEnabled(true);
        g_logs->printStdMsg(output);
        g_logs->consoleEnabled(false);
    }
    else
        cerr << output;
}

static void getFilename(char *dst, const char *src)
{
    long offset = 0;
    for (auto *ptr = src; *ptr != 0; ptr++)
    {
        if (*ptr == '/' || *ptr == '\\')
            offset = ptr - src + 1;
    }
    strncpy(dst, src + offset, 1024 - 1);
}

#ifdef NDEBUG
[[ noreturn ]]
#endif
void Exception(const char *file, const char *func, int line, const char *msg)
{
    char str[1024];
    getFilename(static_cast<char *>(str), file);

    string message = "\nException occured: ";
    if (msg)
    {
        message += "\"" + std::string(msg) + "\"";
    }
    message += "\n" + std::string(func) + " at " + std::string(static_cast<char *>(str)) + ": line " + std::to_string(line) + "\n";

    string output = "Backtrace:\n";
    GetBackTrace(output);

    LogCrash(output, message);

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
        GetBackTrace(output);
        LogCrash(output, message);
    }

    exit(signal);
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
