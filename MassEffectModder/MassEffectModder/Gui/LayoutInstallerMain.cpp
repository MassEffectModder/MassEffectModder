/*
 * MassEffectModder
 *
 * Copyright (C) 2020-2021 Pawel Kolodziejski
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

#include <Gui/LayoutInstallerMain.h>
#include <Gui/InstallerWindow.h>
#include <Gui/MessageWindow.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/FileStream.h>
#include <Helpers/Logs.h>
#include <Misc/Misc.h>
#include <GameData/GameData.h>

LayoutInstallerMain::LayoutInstallerMain(MeType gameType, InstallerWindow *window)
    : installerWindow(window), gameId(gameType)
{
    layoutId = InstallerWindow::kLayoutInstallerMain;

    QPixmap Image(QString(":/me%1_bg.jpg").arg((int)gameId));
    Image = Image.scaled(window->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QPalette palette;
    palette.setBrush(QPalette::Window, Image);
    window->setPalette(palette);

    currentStatusLabel = new QLabel();
    QFont StatusFont = currentStatusLabel->font();
    StatusFont = currentStatusLabel->font();
    StatusFont.setPointSize(30);
    StatusFont.setBold(true);
    currentStatusLabel->setFont(StatusFont);
    currentStatusLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

    buttonStart = new QPushButton("S T A R T");
    buttonStart->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonStart->setMinimumWidth(kButtonMinWidth);
    buttonStart->setMinimumHeight(kButtonMinHeight / 2);
    QFont ButtonFont = buttonStart->font();
    ButtonFont.setPointSize(20);
    ButtonFont.setBold(true);
    buttonStart->setFont(ButtonFont);
    connect(buttonStart, &QPushButton::clicked, this, &LayoutInstallerMain::StartSelected);

    auto verticalLayoutMain = new QVBoxLayout(this);
    verticalLayoutMain->setAlignment(Qt::AlignCenter);
    verticalLayoutMain->addWidget(currentStatusLabel);
    verticalLayoutMain->addWidget(buttonStart);

    window->SetTitle(gameType, "Installer");
}

void LayoutInstallerMain::LockGui(bool lock)
{
    foreach (QWidget *widget, this->findChildren<QWidget*>())
    {
        widget->setEnabled(!lock);
    }
    installerWindow->LockClose(lock);
}

void LayoutInstallerMain::InstallCallback(void *handle, int progress, const QString &stage)
{
    auto *win = static_cast<LayoutInstallerMain *>(handle);
    QString progressMsg;
    if (progress != -1)
        progressMsg = " " + QString::number(progress) + "%";
    win->currentStatusLabel->setText(stage + progressMsg);
    QApplication::processEvents();
}

void LayoutInstallerMain::StartSelected()
{
    buttonStart->hide();

    Resources resources;
    resources.loadMD5Tables();

    ConfigIni configIni = ConfigIni();
    g_GameData->Init(gameId, configIni);
    if (!Misc::CheckGamePath())
    {
        QMessageBox::critical(this, "Installing MEM mods", "Game data not found.");
        buttonStart->show();
        return;
    }

    if (!Misc::checkWriteAccessDir(g_GameData->MainData()))
    {
        QMessageBox::critical(this, "Installing MEM mods",
                              QString("Installer has not write access to game folder.") +
              "\n\nCorrect access to game directory." +
              "\n\nThen start Installer again.");
        buttonStart->show();
        return;
    }

    QDirIterator iterator(QCoreApplication::applicationDirPath()
#if defined(__APPLE__)
                      + "/../../../"
#endif
                      , QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    QStringList modFiles;
    QString destPath = g_GameData->GamePath();
    quint64 diskFreeSpace = Misc::getDiskFreeSpace(destPath);
    quint64 diskUsage = 0;
    while (iterator.hasNext())
    {
        iterator.next();
        if (!iterator.fileName().endsWith(".mem", Qt::CaseSensitivity::CaseInsensitive))
            continue;
        auto file = iterator.fileInfo();
        auto info = QFileInfo(file);
        diskUsage += info.size();
        modFiles.push_back(file.absoluteFilePath());
    }
    modFiles.sort(Qt::CaseSensitivity::CaseInsensitive);

    diskUsage = (quint64)(diskUsage * 2.5);
    if (diskUsage >= diskFreeSpace)
    {
        QMessageBox::critical(this, "Installing MEM file(s)",
                              "You have not enough disk space remaining. You need about " +
                              Misc::getBytesFormat(diskUsage) + " free disk space.");
        buttonStart->show();
        return;
    }

    installerWindow->LockClose(false);
    currentStatusLabel->setText("");

    g_logs->BufferClearErrors();
    g_logs->BufferEnableErrors(true);

    if (!Misc::InstallMods(gameId, resources, modFiles, false, false, false, false, -1,
                           &LayoutInstallerMain::InstallCallback, this))
    {
        QMessageBox::critical(this, "Installing MEM mods", "Installation failed!");
        currentStatusLabel->setText("Installation failed!");
    }
    else
    {
        currentStatusLabel->setText("Installation completed.");
    }

    g_logs->BufferEnableErrors(false);
    if (g_logs->BufferGetErrors() != "")
    {
        MessageWindow msg;
        msg.Show(installerWindow, "Errors while installing MEM mods", g_logs->BufferGetErrors());
    }

    installerWindow->LockClose(false);
}
