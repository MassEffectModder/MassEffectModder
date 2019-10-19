/*
 * MassEffectModder
 *
 * Copyright (C) 2019 Pawel Kolodziejski
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

#include "Gui/LayoutMeSelect.h"
#include "Gui/LayoutModsManager.h"
#include "Gui/MainWindow.h"
#include "Gui/MessageWindow.h"
#include "Helpers/MiscHelpers.h"
#include "Helpers/Logs.h"
#include "GameData.h"
#include "Misc.h"
#include "MipMaps.h"

LayoutModsManager::LayoutModsManager(MainWindow *window)
    : mainWindow(window)
{
    if (window == nullptr)
        CRASH();

    layoutId = MainWindow::kLayoutModsManager;

    auto ButtonInstallMods = new QPushButton("Install Mods");
    ButtonInstallMods->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonInstallMods->setMinimumWidth(kButtonMinWidth);
    ButtonInstallMods->setMinimumHeight(kButtonMinHeight);
    QFont ButtonFont = ButtonInstallMods->font();
    ButtonFont.setPointSize(kFontSize);
    ButtonInstallMods->setFont(ButtonFont);
    connect(ButtonInstallMods, &QPushButton::clicked, this, &LayoutModsManager::InstallModsSelected);

    auto ButtonExtractMods = new QPushButton("Extract Mods");
    ButtonExtractMods->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonExtractMods->setMinimumWidth(kButtonMinWidth);
    ButtonExtractMods->setMinimumHeight(kButtonMinHeight);
    ButtonExtractMods->setFont(ButtonFont);
    connect(ButtonExtractMods, &QPushButton::clicked, this, &LayoutModsManager::ExtractModsSelected);

    auto ButtonCreateMod = new QPushButton("Create Mod");
    ButtonCreateMod->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonCreateMod->setMinimumWidth(kButtonMinWidth);
    ButtonCreateMod->setMinimumHeight(kButtonMinHeight);
    ButtonCreateMod->setFont(ButtonFont);
    connect(ButtonCreateMod, &QPushButton::clicked, this, &LayoutModsManager::CreateModSelected);

    auto ButtonCreateBinaryMod = new QPushButton("Create Binary Mod");
    ButtonCreateBinaryMod->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonCreateBinaryMod->setMinimumWidth(kButtonMinWidth);
    ButtonCreateBinaryMod->setMinimumHeight(kButtonMinHeight);
    ButtonCreateBinaryMod->setFont(ButtonFont);
    connect(ButtonCreateBinaryMod, &QPushButton::clicked, this, &LayoutModsManager::CreateBinaryModSelected);

    auto ButtonReturn = new QPushButton("Return");
    ButtonReturn->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonReturn->setMinimumWidth(kButtonMinWidth);
    ButtonReturn->setMinimumHeight(kButtonMinHeight / 2);
    ButtonReturn->setFont(ButtonFont);
    connect(ButtonReturn, &QPushButton::clicked, this, &LayoutModsManager::ReturnSelected);

    auto *horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 40));
    auto *verticalLayout = new QVBoxLayout();
    verticalLayout->setAlignment(Qt::AlignVCenter);
    verticalLayout->addWidget(ButtonInstallMods, 1);
    verticalLayout->addWidget(ButtonExtractMods, 1);
    verticalLayout->addWidget(ButtonCreateMod, 1);
    verticalLayout->addWidget(ButtonCreateBinaryMod, 1);
    verticalLayout->addSpacing(20);
    verticalLayout->addWidget(ButtonReturn, 1);
    horizontalLayout->addLayout(verticalLayout);
    horizontalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 40));
}

void LayoutModsManager::LockGui(bool enable)
{
    foreach (QWidget *widget, this->findChildren<QWidget*>())
    {
        widget->setEnabled(!enable);
    }
    mainWindow->LockClose(enable);
}

void LayoutModsManager::InstallModsSelected()
{
}

void LayoutModsManager::ExtractMEMCallback(void *handle, int progress)
{
    auto *win = static_cast<MainWindow *>(handle);
    win->statusBar()->showMessage(QString("Extracting MEM files... Progress: ") + QString::number(progress) + "%");
    QApplication::processEvents();
}

void LayoutModsManager::ExtractModsSelected()
{
    LockGui(true);
    QStringList files = QFileDialog::getOpenFileNames(this,
            "Please select Mod file", "", "MEM mod file (*.mem)");
    if (files.count() == 0)
    {
        LockGui(false);
        return;
    }
    QString outDir = QFileDialog::getExistingDirectory(this,
            "Please select destination directory for MEM extraction");
    if (outDir == "")
    {
        LockGui(false);
        return;
    }
    outDir = QDir::cleanPath(outDir);
    QFileInfoList list;
    long diskFreeSpace = Misc::getDiskFreeSpace(outDir);
    long diskUsage = 0;
    foreach (QString file, files)
    {
        auto info = QFileInfo(file);
        diskUsage += info.size();
        list.push_back(info);
    }
    diskUsage = (long)(diskUsage * 2.5);
    if (diskUsage >= diskFreeSpace)
    {
        QMessageBox::critical(this, "Extracting MEM file(s)",
                              "You have not enough disk space remaining. You need about " +
                              Misc::getBytesFormat(diskUsage) + " free disk space.");
        LockGui(false);
        return;
    }

    g_logs->BufferClearErrors();
    g_logs->BufferEnableErrors(true);
    mainWindow->statusBar()->clearMessage();
    Misc::extractMEM(mainWindow->gameType, list, outDir,
                     &LayoutModsManager::ExtractMEMCallback, mainWindow);
    mainWindow->statusBar()->clearMessage();
    g_logs->BufferEnableErrors(false);
    if (g_logs->BufferGetErrors() != "")
    {
        MessageWindow msg;
        msg.Show("Extracting MEM file(s)", g_logs->BufferGetErrors());
    }
    else
    {
        QMessageBox::information(this, "Checking game", "All game files checked.");
    }

    LockGui(false);
}

void LayoutModsManager::CreateModSelected()
{
}

void LayoutModsManager::CreateBinaryModSelected()
{
}

void LayoutModsManager::ReturnSelected()
{
    mainWindow->SwitchLayoutById(MainWindow::kLayoutModules);
    mainWindow->GetLayout()->removeWidget(this);
}
