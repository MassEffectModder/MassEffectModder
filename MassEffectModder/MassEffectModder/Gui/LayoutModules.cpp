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

#include "Gui/LayoutModules.h"
#include "Gui/LayoutTexturesManager.h"
#include "Gui/LayoutTextureUtilities.h"
#include "Gui/LayoutGameUtilities.h"
#include "Gui/LayoutModsManager.h"
#include "Gui/MainWindow.h"
#include "Helpers/MiscHelpers.h"
#include "MemTypes.h"

LayoutModules::LayoutModules(MainWindow *window)
    : mainWindow(window)
{
    if (window == nullptr)
        CRASH();

    layoutId = MainWindow::kLayoutModules;

    auto ButtonTexturesManager = new QPushButton("Textures Manager");
    ButtonTexturesManager->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonTexturesManager->setMinimumWidth(kButtonMinWidth);
    ButtonTexturesManager->setMinimumHeight(kButtonMinHeight);
    QFont ButtonFont = ButtonTexturesManager->font();
    ButtonFont.setPointSize(kFontSize);
    ButtonTexturesManager->setFont(ButtonFont);
    connect(ButtonTexturesManager, &QPushButton::clicked, this, &LayoutModules::TexturesManagerSelected);

    auto ButtonTextureUtilities = new QPushButton("Texture Utilities");
    ButtonTextureUtilities->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonTextureUtilities->setMinimumWidth(kButtonMinWidth);
    ButtonTextureUtilities->setMinimumHeight(kButtonMinHeight);
    ButtonTextureUtilities->setFont(ButtonFont);
    connect(ButtonTextureUtilities, &QPushButton::clicked, this, &LayoutModules::TextureUtilitiesSelected);

    auto ButtonGameUtilities = new QPushButton("Game Utilities", this);
    ButtonGameUtilities->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonGameUtilities->setMinimumWidth(kButtonMinWidth);
    ButtonGameUtilities->setMinimumHeight(kButtonMinHeight);
    ButtonGameUtilities->setFont(ButtonFont);
    connect(ButtonGameUtilities, &QPushButton::clicked, this, &LayoutModules::GameUtilitiesSelected);

    auto ButtonModsManager = new QPushButton("Mods Manager", this);
    ButtonModsManager->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonModsManager->setMinimumWidth(kButtonMinWidth);
    ButtonModsManager->setMinimumHeight(kButtonMinHeight);
    ButtonModsManager->setFont(ButtonFont);
    connect(ButtonModsManager, &QPushButton::clicked, this, &LayoutModules::ModsManagerSelected);

    auto ButtonReturn = new QPushButton("Return", this);
    ButtonReturn->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonReturn->setMinimumWidth(kButtonMinWidth);
    ButtonReturn->setMinimumHeight(kButtonMinHeight / 2);
    ButtonReturn->setFont(ButtonFont);
    connect(ButtonReturn, &QPushButton::clicked, this, &LayoutModules::ReturnSelected);

    auto *horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 40));
    auto *verticalLayout = new QVBoxLayout();
    verticalLayout->setAlignment(Qt::AlignVCenter);
    verticalLayout->addWidget(ButtonTexturesManager, 1);
    verticalLayout->addWidget(ButtonTextureUtilities, 1);
    verticalLayout->addWidget(ButtonGameUtilities, 1);
    verticalLayout->addWidget(ButtonModsManager, 1);
    verticalLayout->addSpacing(20);
    verticalLayout->addWidget(ButtonReturn, 1);
    horizontalLayout->addLayout(verticalLayout);
    horizontalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 40));
}

void LayoutModules::TexturesManagerSelected()
{
    mainWindow->GetLayout()->addWidget(new LayoutTexturesManager(mainWindow));
    mainWindow->SwitchLayoutById(MainWindow::kLayoutTexturesManager);
}

void LayoutModules::TextureUtilitiesSelected()
{
    mainWindow->GetLayout()->addWidget(new LayoutTextureUtilities(mainWindow));
    mainWindow->SwitchLayoutById(MainWindow::kLayoutTextureUtilities);
}

void LayoutModules::GameUtilitiesSelected()
{
    mainWindow->GetLayout()->addWidget(new LayoutGameUtilities(mainWindow));
    mainWindow->SwitchLayoutById(MainWindow::kLayoutGameUtilities);
}

void LayoutModules::ModsManagerSelected()
{
    mainWindow->GetLayout()->addWidget(new LayoutModsManager(mainWindow));
    mainWindow->SwitchLayoutById(MainWindow::kLayoutModsManager);
}

void LayoutModules::ReturnSelected()
{
    QString title = QString("Mass Effect Modder v%1").arg(MEM_VERSION);
    if (DetectAdminRights())
        title += " (run as Administrator)";
    mainWindow->setWindowTitle(title);
    mainWindow->gameType = MeType::UNKNOWN_TYPE;
    mainWindow->SwitchLayoutById(MainWindow::kLayoutMeSelect);
    mainWindow->GetLayout()->removeWidget(this);
}
