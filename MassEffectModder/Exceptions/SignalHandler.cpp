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

using namespace std;

static void SignalsHandler(int signal)
{
    string output;

    switch (signal)
    {
        case SIGSEGV:
            output += "\nSIGSEGV: Invalid memory access (segmentation fault)\n";
            break;
        case SIGABRT:
            output += "\nSIGABRT: Abnormal termination an abort() or assert()\n";
            break;
        case SIGILL:
            output += "\nSIGILL: Illegal instruction\n";
            break;
        case SIGFPE:
            output += "\nSIGFPE: Fatal arithmetic error\n";
            break;
        case SIGINT:
            output += "\nSIGINT: Received interrupt, probably a ctrl+c\n";
            exit(signal);
            break;
        case SIGTERM:
        default:
            output += "\nSIGTERM: Program termination request received\n";
            exit(signal);
            break;
    }

    //GetBackTrace(output, true);

    cerr << output;

    //if (g_log)
    //    g_log->printf("%s", output.c_str);

    //if (g_qexpectiongui)
    //    g_qexpectiongui->display();

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
