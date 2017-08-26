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

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFile>

#include "Exceptions/SignalHandler.h"
#include "Gui/MainWindow.h"
#include "Logs/Logs.h"

#include "ConfigIni.h"

#define MEM_VERSION "200"
#define APP_NAME "MEM"

int runQtApplication(int argc, char *argv[])
{
    QApplication application(argc, argv);

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
        QString iniPath = QCoreApplication::applicationDirPath() + QDir::separator() + "installer.ini";
        if (QFile::exists(iniPath))
        {
            // TODO: installer mode
            return 0;
        }
        else
        {
            MainWindow window;
            window.show();

            return application.exec();
        }
    }
}

int main(int argc, char *argv[])
{
    InstallSignalsHandler();

    CreateLogs();

    int status = runQtApplication(argc, argv);

    ReleaseLogs();

    return status;
}
