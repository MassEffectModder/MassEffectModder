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

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QSysInfo>
#include <QStringList>

#include "Exceptions/SignalHandler.h"
#include "Helpers/Logs.h"
#include "Helpers/Misc.h"

#include "CmdLineParams.h"
#include "Version.h"

int runQtApplication(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);

    QCoreApplication::setOrganizationName(APP_NAME);
    QCoreApplication::setApplicationName(APP_NAME);

    ConsoleWrite(QString("\nMassEffectModder (MEM) v%1 command line version\n"
                         "Copyright (C) 2014-2018 Pawel Kolodziejski\n"
                         "This is free software; see the source for copying conditions.\n"
                         "There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
                         ).arg(MEM_VERSION));

    return ProcessArguments();
}

int main(int argc, char *argv[])
{
    InstallSignalsHandler();

    CreateLogs();
    g_logs->printMsg("\n----------------------------------------------------");
    g_logs->printMsg("Log started at: " + QDateTime::currentDateTime().toString());
    g_logs->printMsg(QString(APP_NAME) + " v" + QString(MEM_VERSION));
    g_logs->printMsg("OS: " + QSysInfo::productType() + " " + QSysInfo::productVersion());
    g_logs->printMsg("RAM: " + QString::number(DetectAmountMemoryGB()) + " GB\n");

    int status = runQtApplication(argc, argv);

    ReleaseLogs();

    return status;
}
