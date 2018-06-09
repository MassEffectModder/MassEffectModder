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
#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QSysInfo>

#include "Exceptions/SignalHandler.h"
#include "Logs/Logs.h"
#include "Helpers/Misc.h"

#include "ConfigIni.h"
#include "Version.h"

#define APP_NAME "MEM"

int runQtApplication(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);

    QCoreApplication::setOrganizationName(APP_NAME);
    QCoreApplication::setApplicationName(APP_NAME);
    QCoreApplication::setApplicationVersion(MEM_VERSION);

    QCommandLineParser cmdLineParser;
    cmdLineParser.setApplicationDescription(QCoreApplication::applicationName());
    cmdLineParser.process(application);

    ConfigIni conf;

    if (!cmdLineParser.positionalArguments().isEmpty())
    {
        // TODO: cmdline mode
        return -1;
    }
    else
    {
        // TODO: cmdline mode
        return -1;
    }
}

int main(int argc, char *argv[])
{
    InstallSignalsHandler();

    CreateLogs();
    g_logs->print("\n----------------------------------------------------\n");
    g_logs->printf("Log started at: %s\n", QDateTime::currentDateTime().toString().toStdString().c_str());

    g_logs->print(APP_NAME " v" MEM_VERSION "\n");
    g_logs->printf("OS: %s %s\n", QSysInfo::productType().toStdString().c_str(),
                  QSysInfo::productVersion().toStdString().c_str());
    g_logs->printf("RAM: %d GB\n\n", DetectAmountMemoryGB());

    int status = runQtApplication(argc, argv);

    ReleaseLogs();

    return status;
}
