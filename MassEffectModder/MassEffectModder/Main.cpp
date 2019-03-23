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
#ifdef GUI
#include "Gui/MainWindow.h"
#endif
#include "GameData.h"

#include "CmdLineParams.h"

bool g_ipc = false;

int runQtApplication(int argc, char *argv[])
{
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

#ifdef GUI
    QApplication application(argc, argv);

    QApplication::setOrganizationName(APP_NAME);
    QApplication::setApplicationName(APP_NAME);
    MainWindow window;
    window.show();

    return QApplication::exec();
#else
    QCoreApplication application(argc, argv);

    QCoreApplication::setOrganizationName(APP_NAME);
    QCoreApplication::setApplicationName(APP_NAME);
    PINFO(QString("\nMassEffectModder (MEM) v%1 command line version\n"
                  "Copyright (C) 2014-%2 Pawel Kolodziejski\n"
                  "This is free software; see the source for copying conditions.  There is NO.\n"
                  "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\n"
                  ).arg(MEM_VERSION).arg(MEM_YEAR));

    return ProcessArguments();
#endif
}

int main(int argc, char *argv[])
{
    InstallSignalsHandler();
    CreateGameData();
    CreateLogs();

#ifdef GUI
    g_logs->EnableOutputFile(true);
#else
    g_logs->EnableOutputConsole(true);
#endif
    g_logs->ChangeLogLevel(LOG_INFO);

#ifdef GUI
    PINFO("\n----------------------------------------------------\n");
    PINFO(QString("Log started at: ") + QDateTime::currentDateTime().toString() + "\n");
    PINFO(QString(APP_NAME) + " v" + QString(MEM_VERSION) + "\n");
    PINFO(QString("OS: ") + QSysInfo::productType() + " " + QSysInfo::productVersion() + "\n");
    PINFO(QString("RAM: ") + QString::number(DetectAmountMemoryGB()) + " GB\n\n");
#endif

    int status = runQtApplication(argc, argv);

    ReleaseLogs();
    ReleaseGameData();

    return status;
}
