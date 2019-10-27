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

#include <Gui/LayoutMeSelect.h>
#include <Gui/LayoutModsManager.h>
#include <Gui/LayoutInstallModsManager.h>
#include <Gui/MainWindow.h>
#include <Gui/MessageWindow.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/FileStream.h>
#include <Helpers/Logs.h>
#include <Misc/Misc.h>
#include <GameData/GameData.h>

LayoutInstallModsManager::LayoutInstallModsManager(MainWindow *window)
    : mainWindow(window)
{
    if (window == nullptr)
        CRASH();

    layoutId = MainWindow::kLayoutInstallModsManager;

    ListMods = new QListWidget();
    ListMods->setMinimumWidth(kButtonMinWidth);
    ListMods->setMinimumHeight(kButtonMinHeight / 2);
    ListMods->setSelectionMode(QAbstractItemView::ExtendedSelection);

    auto LabelListMods = new QLabel("List of loaded mods:");
    LabelListMods->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
    QFont LabelListModsFont = LabelListMods->font();
    LabelListModsFont.setPointSize(kFontSize);
    LabelListMods->setFont(LabelListModsFont);

    auto *verticalLayoutList = new QVBoxLayout();
    verticalLayoutList->addWidget(LabelListMods, 1);
    verticalLayoutList->addWidget(ListMods, 1);

    auto ButtonAdd = new QPushButton("Add to list");
    ButtonAdd->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonAdd->setMinimumWidth(kButtonMinWidth);
    ButtonAdd->setMinimumHeight(kButtonMinHeight / 2);
    QFont ButtonFont = ButtonAdd->font();
    ButtonFont.setPointSize(kFontSize);
    ButtonAdd->setFont(ButtonFont);
    connect(ButtonAdd, &QPushButton::clicked, this, &LayoutInstallModsManager::AddSelected);

    auto ButtonRemove = new QPushButton("Remove selected from list");
    ButtonRemove->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonRemove->setMinimumWidth(kButtonMinWidth);
    ButtonRemove->setMinimumHeight(kButtonMinHeight / 2);
    ButtonFont = ButtonRemove->font();
    ButtonFont.setPointSize(kFontSize);
    ButtonRemove->setFont(ButtonFont);
    connect(ButtonRemove, &QPushButton::clicked, this, &LayoutInstallModsManager::RemoveSelected);

    auto ButtonClear = new QPushButton("Clear list");
    ButtonClear->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonClear->setMinimumWidth(kButtonMinWidth);
    ButtonClear->setMinimumHeight(kButtonMinHeight / 2);
    ButtonFont = ButtonClear->font();
    ButtonFont.setPointSize(kFontSize);
    ButtonClear->setFont(ButtonFont);
    connect(ButtonClear, &QPushButton::clicked, this, &LayoutInstallModsManager::ClearSelected);

    auto ButtonInstall = new QPushButton("Install selected mods");
    ButtonInstall->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonInstall->setMinimumWidth(kButtonMinWidth);
    ButtonInstall->setMinimumHeight(kButtonMinHeight / 2);
    ButtonFont = ButtonRemove->font();
    ButtonFont.setPointSize(kFontSize);
    ButtonInstall->setFont(ButtonFont);
    connect(ButtonInstall, &QPushButton::clicked, this, &LayoutInstallModsManager::InstallSelected);

    auto ButtonInstallAll = new QPushButton("Install all mods");
    ButtonInstallAll->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonInstallAll->setMinimumWidth(kButtonMinWidth);
    ButtonInstallAll->setMinimumHeight(kButtonMinHeight / 2);
    ButtonFont = ButtonRemove->font();
    ButtonFont.setPointSize(kFontSize);
    ButtonInstallAll->setFont(ButtonFont);
    connect(ButtonInstallAll, &QPushButton::clicked, this, &LayoutInstallModsManager::InstallAllSelected);

    auto ButtonReturn = new QPushButton("Exit Mods Installer");
    ButtonReturn->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonReturn->setMinimumWidth(kButtonMinWidth);
    ButtonReturn->setMinimumHeight(kButtonMinHeight / 2);
    ButtonFont = ButtonReturn->font();
    ButtonFont.setPointSize(kFontSize);
    ButtonReturn->setFont(ButtonFont);
    connect(ButtonReturn, &QPushButton::clicked, this, &LayoutInstallModsManager::ReturnSelected);

    auto *verticalLayout = new QVBoxLayout();
    verticalLayout->setAlignment(Qt::AlignVCenter);
    verticalLayout->addWidget(ButtonAdd, 1);
    verticalLayout->addWidget(ButtonRemove, 1);
    verticalLayout->addWidget(ButtonClear, 1);
    verticalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 3));
    verticalLayout->addWidget(ButtonInstall, 1);
    verticalLayout->addWidget(ButtonInstallAll, 1);
    verticalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 3));
    verticalLayout->addWidget(ButtonReturn, 1);

    auto *horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 20));
    horizontalLayout->addLayout(verticalLayoutList);
    horizontalLayout->addLayout(verticalLayout);
    horizontalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 20));

    mainWindow->SetTitle("Mods Installer");
}

void LayoutInstallModsManager::LockGui(bool enable)
{
    foreach (QWidget *widget, this->findChildren<QWidget*>())
    {
        widget->setEnabled(!enable);
    }
    mainWindow->LockClose(enable);
}

void LayoutInstallModsManager::AddSelected()
{
    QFileDialog dialog = QFileDialog(this, "Please select MEM mod file(s)",
                                     "", "MEM file (*.mem)");
    QStringList files;
    dialog.setFileMode(QFileDialog::ExistingFiles);
    if (dialog.exec())
    {
        files = dialog.selectedFiles();
    }
    if (files.count() == 0)
    {
        LockGui(false);
        return;
    }
    files.sort(Qt::CaseInsensitive);

    g_logs->BufferClearErrors();
    g_logs->BufferEnableErrors(true);
    foreach (QString file, files)
    {
        FileStream fs = FileStream(file, FileMode::Open, FileAccess::ReadOnly);
        if (!Misc::CheckMEMHeader(fs, file))
            continue;
        if (!Misc::CheckMEMGameVersion(fs, file, mainWindow->gameType))
            continue;

        QListWidgetItem *item = new QListWidgetItem(BaseNameWithoutExt(file));
        item->setData(Qt::UserRole, file);
        ListMods->addItem(item);
    }
    g_logs->BufferEnableErrors(false);
    if (g_logs->BufferGetErrors() != "")
    {
        MessageWindow msg;
        msg.Show(mainWindow, "Ading MEM file(s)", g_logs->BufferGetErrors());
    }

    LockGui(false);
}

void LayoutInstallModsManager::RemoveSelected()
{
    foreach (QListWidgetItem *item, ListMods->selectedItems())
    {
        ListMods->removeItemWidget(item);
        delete item;
    }
}

void LayoutInstallModsManager::ClearSelected()
{
    ListMods->clear();
}

void LayoutInstallModsManager::InstallModsCallback(void *handle, int progress, const QString &stage)
{
    auto *win = static_cast<MainWindow *>(handle);
    win->statusBar()->showMessage(QString("Installing MEM mods... Stage: ") + stage + " -  Progress: " + QString::number(progress) + "%");
    QApplication::processEvents();
}

void LayoutInstallModsManager::InstallMods(QStringList &mods)
{
    if (mods.count() == 0)
        return;
    ConfigIni configIni{};
    g_GameData->Init(mainWindow->gameType, configIni);
    if (g_GameData->GamePath().length() == 0 || !QDir(g_GameData->GamePath()).exists())
    {
        mainWindow->statusBar()->clearMessage();
        QMessageBox::critical(this, "Installing MEM mods", "Game data not found.");
        LockGui(false);
        return;
    }

    QList<FoundTexture> textures;
    Resources resources;
    resources.loadMD5Tables();

    TreeScan::loadTexturesMap(mainWindow->gameType, resources, textures);

    g_logs->BufferClearErrors();
    g_logs->BufferEnableErrors(true);

    if (!Misc::InstallMods(mainWindow->gameType, resources, mods, false, false, false, false, 0,
                      &LayoutInstallModsManager::InstallModsCallback, mainWindow))
    {
        QMessageBox::critical(this, "Installing MEM mods", "Installation failed!");
    }
    mainWindow->statusBar()->clearMessage();

    g_logs->BufferEnableErrors(false);
    if (g_logs->BufferGetErrors() != "")
    {
        MessageWindow msg;
        msg.Show(mainWindow, "Errors while installing MEM mods", g_logs->BufferGetErrors());
    }
    else
    {
        QMessageBox::information(this, "Installing MEM mods", "All MEM mods installed.");
    }
}

void LayoutInstallModsManager::InstallSelected()
{
    LockGui(true);
    QStringList mods;
    foreach (QListWidgetItem *item, ListMods->selectedItems())
    {
        auto file = item->data(Qt::UserRole).toString();
        mods.append(file);
    }

    InstallMods(mods);
    LockGui(false);
}

void LayoutInstallModsManager::InstallAllSelected()
{
    LockGui(true);
    QStringList mods;
    for (int i = 0; i < ListMods->count(); i++)
    {
        QListWidgetItem *item = ListMods->item(i);
        auto file = item->data(Qt::UserRole).toString();
        mods.append(file);
    }

    InstallMods(mods);
    LockGui(false);
}

void LayoutInstallModsManager::ReturnSelected()
{
    mainWindow->SwitchLayoutById(MainWindow::kLayoutModsManager);
    mainWindow->GetLayout()->removeWidget(this);
    mainWindow->SetTitle("Mods Manager");
}
