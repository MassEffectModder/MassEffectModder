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
#include "Gui/LayoutInstallModsManager.h"
#include "Gui/MainWindow.h"

LayoutInstallModsManager::LayoutInstallModsManager(MainWindow *window)
    : mainWindow(window)
{
    if (window == nullptr)
        CRASH();

    layoutId = MainWindow::kLayoutInstallModsManager;

    auto ListViewMods = new QListView();
    ListViewMods->setMinimumWidth(kButtonMinWidth);
    ListViewMods->setMinimumHeight(kButtonMinHeight / 2);

    auto LabelListMods = new QLabel("List of loaded mods:");
    LabelListMods->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
    QFont LabelListModsFont = LabelListMods->font();
    LabelListModsFont.setPointSize(kFontSize);
    LabelListMods->setFont(LabelListModsFont);

    auto *verticalLayoutList = new QVBoxLayout();
    verticalLayoutList->addWidget(LabelListMods, 1);
    verticalLayoutList->addWidget(ListViewMods, 1);

    auto ButtonAdd = new QPushButton("Add to list");
    ButtonAdd->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonAdd->setMinimumWidth(kButtonMinWidth);
    ButtonAdd->setMinimumHeight(kButtonMinHeight / 2);
    QFont ButtonFont = ButtonAdd->font();
    ButtonFont.setPointSize(kFontSize);
    ButtonAdd->setFont(ButtonFont);
    connect(ButtonAdd, &QPushButton::clicked, this, &LayoutInstallModsManager::AddSelected);

    auto ButtonRemove = new QPushButton("Remove from list");
    ButtonRemove->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonRemove->setMinimumWidth(kButtonMinWidth);
    ButtonRemove->setMinimumHeight(kButtonMinHeight / 2);
    ButtonFont = ButtonRemove->font();
    ButtonFont.setPointSize(kFontSize);
    ButtonRemove->setFont(ButtonFont);
    connect(ButtonRemove, &QPushButton::clicked, this, &LayoutInstallModsManager::RemoveSelected);

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

    auto ButtonReturn = new QPushButton("Return");
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
}

void LayoutInstallModsManager::AddSelected()
{
}

void LayoutInstallModsManager::RemoveSelected()
{
}

void LayoutInstallModsManager::InstallSelected()
{
}

void LayoutInstallModsManager::InstallAllSelected()
{
}

void LayoutInstallModsManager::ReturnSelected()
{
    mainWindow->SwitchLayoutById(MainWindow::kLayoutModsManager);
    mainWindow->GetLayout()->removeWidget(this);
}
