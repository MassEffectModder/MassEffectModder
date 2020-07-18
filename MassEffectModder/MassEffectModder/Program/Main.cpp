/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2020 Pawel Kolodziejski
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

#include <CmdLine/CmdLineParams.h>
#include <GameData/GameData.h>
#ifdef GUI
#include <Gui/MainWindow.h>
#include <Gui/InstallerWindow.h>
#endif
#include <Helpers/Logs.h>
#include <Helpers/MiscHelpers.h>
#include <Program/SignalHandler.h>
#ifdef GUI
#include <Program/Updater.h>
#endif

bool g_ipc = false;

int runQtApplication(int argc, char *argv[])
{
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    CreateGameData();
#ifdef GUI
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", QByteArray("1"));

    QApplication application(argc, argv);

    QApplication::setOrganizationName(APP_NAME);
    QApplication::setApplicationName(APP_NAME);
    QApplication::setWindowIcon(QIcon(":/MEM.png"));

#ifndef __APPLE__
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    QPalette darkModePalette;

    darkModePalette.setColor(QPalette::AlternateBase, QColor(50, 50, 50));
    darkModePalette.setColor(QPalette::Base, QColor(30, 30, 30));
    darkModePalette.setColor(QPalette::BrightText, Qt::white);
    darkModePalette.setColor(QPalette::Button, QColor(0x57, 0x57, 0x57));
    darkModePalette.setColor(QPalette::Disabled, QPalette::Button, QColor(64, 64, 64));
    darkModePalette.setColor(QPalette::ButtonText, Qt::white);
    darkModePalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));

    darkModePalette.setColor(QPalette::PlaceholderText, QColor(50, 50, 50));
    darkModePalette.setColor(QPalette::Text, Qt::white);
    darkModePalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));

    darkModePalette.setColor(QPalette::ToolTipBase, QColor(30, 30, 30));
    darkModePalette.setColor(QPalette::ToolTipText, Qt::white);

    darkModePalette.setColor(QPalette::Window, QColor(50, 50, 50));
    darkModePalette.setColor(QPalette::WindowText, Qt::white);
    darkModePalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));

    darkModePalette.setColor(QPalette::Highlight, QColor(0, 75, 218));
    darkModePalette.setColor(QPalette::Inactive, QPalette::Highlight, QColor(81, 81, 81));
    darkModePalette.setColor(QPalette::HighlightedText, Qt::white);

    darkModePalette.setColor(QPalette::Link, QColor(0, 75, 218));
    darkModePalette.setColor(QPalette::LinkVisited, QColor(0, 75, 218));

    QApplication::setPalette(darkModePalette);
    application.setStyleSheet("QToolTip { color: #ffffff; background-color: #505050; border: 1px solid white; }");
#endif

    QString path = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation).first() +
            "/MassEffectModder/Logs";
    QDir().mkpath(path);
    auto dateTime = QDateTime::currentDateTime();
    QString logFile = QString().asprintf("/MEMLog-%04d%02d%02d_%08d.txt", dateTime.date().year(),
            dateTime.date().month(), dateTime.date().day(), dateTime.time().msecsSinceStartOfDay());
    g_logs->EnableOutputFile(path + "/" + logFile, true);
    g_logs->ChangeLogLevel(LOG_INFO);

    PINFO(QString("\n----------------------------------------------------\n"
                  "Log started at: %1\n"
                  "%2 v%3\n"
                  "OS: %4 %5\n"
                  "RAM: %6 GB\n\n"
                  ).arg(
                   QDateTime::currentDateTime().toString(),
                   APP_NAME,
                   QString::number(MEM_VERSION),
                   QSysInfo::productType(),
                   QSysInfo::productVersion(),
                   QString::number(DetectAmountMemoryGB())));

    MainWindow window;
    InstallerWindow installer;

    window.show();

#ifdef NDEBUG
    Updater updater(&window);
    QTimer::singleShot(1000, &updater, SLOT(processUpdate()));
#endif

    int status = QApplication::exec();
#else
    QCoreApplication application(argc, argv);

    g_logs->EnableOutputConsole(true);
    g_logs->ChangeLogLevel(LOG_INFO);

    QCoreApplication::setOrganizationName(APP_NAME);
    QCoreApplication::setApplicationName(APP_NAME);
    PINFO(QString("\nMassEffectModder (MEM) v%1 command line version\n"
                  "Copyright (C) 2014-%2 Pawel Kolodziejski\n"
                  "This is free software; see the source for copying conditions.  There is NO.\n"
                  "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\n"
                  ).arg(MEM_VERSION).arg(MEM_YEAR));

    int status = ProcessArguments();
#endif
    ReleaseGameData();
    return status;
}

int main(int argc, char *argv[])
{
    InstallSignalsHandler();
    CreateLogs();

    int status = runQtApplication(argc, argv);

    ReleaseLogs();

    return status;
}
