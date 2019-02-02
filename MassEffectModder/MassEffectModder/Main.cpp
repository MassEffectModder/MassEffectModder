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

#include "Exceptions/SignalHandler.h"
#include "Helpers/Logs.h"
#include "Helpers/MiscHelpers.h"
#include "GameData.h"

#include "CmdLineParams.h"

int runQtApplication(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);

    QCoreApplication::setOrganizationName(APP_NAME);
    QCoreApplication::setApplicationName(APP_NAME);

    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    PINFO(QString("\nMassEffectModder (MEM) v%1 command line version\n"
                  "Copyright (C) 2014-2019 Pawel Kolodziejski\n"
                  "This is free software; see the source for copying conditions.  There is NO.\n"
                  "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\n"
                  ).arg(MEM_VERSION));

    return ProcessArguments();
}

int main(int argc, char *argv[])
{
    InstallSignalsHandler();
    CreateGameData();
    CreateLogs();

    g_logs->EnableOutputConsole(true);
    g_logs->ChangeLogLevel(LOG_INFO);

    PINFO("\n----------------------------------------------------");
    PINFO(QString("Log started at: ") + QDateTime::currentDateTime().toString());
    PINFO(QString(APP_NAME) + " v" + QString(MEM_VERSION));
    PINFO(QString("OS: ") + QSysInfo::productType() + " " + QSysInfo::productVersion());
    PINFO(QString("RAM: ") + QString::number(DetectAmountMemoryGB()) + " GB\n");

    int status = runQtApplication(argc, argv);

    ReleaseLogs();
    ReleaseGameData();

    return status;
}
