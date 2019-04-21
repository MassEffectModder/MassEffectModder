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

void LayoutModsManager::InstallModsSelected()
{
}

void LayoutModsManager::ExtractModsSelected()
{
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
