/*
 * MassEffectModder
 *
 * Copyright (C) 2017 Pawel Kolodziejski <aquadran at users.sourceforge.net>
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
#include <signal.h>

#include <Logs/Logs.h>
#include <Exceptions/Backtrace.h>
#include <errno.h>

using namespace std;

static void SignalsHandler(int signal)
{
    string output;
    bool crashed = false;

    switch (signal)
    {
    case SIGSEGV:
    case SIGABRT:
    case SIGILL:
    case SIGFPE:
        output += "Program crashed!\n";
        crashed = true;
        break;
    default:
        output += "Program stopped.\n";
        break;
    }

    if (crashed)
        GetBackTrace(output);

    if (g_logs)
        g_logs->printf("%s", output.c_str());
    else
        cerr << output;

    //if (crashed && g_qexpectiongui)
    //    g_qexpectiongui->display(output);

    exit(signal);
}

void InstallSignalsHandler()
{
    signal(SIGSEGV, SignalsHandler);
    signal(SIGABRT, SignalsHandler);
    signal(SIGILL,  SignalsHandler);
    signal(SIGFPE,  SignalsHandler);
    signal(SIGINT,  SignalsHandler);
    signal(SIGTERM, SignalsHandler);
}
