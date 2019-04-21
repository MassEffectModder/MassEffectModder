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
#include "Gui/LayoutGameUtilities.h"
#include "Gui/MainWindow.h"

LayoutGameUtilities::LayoutGameUtilities(MainWindow *window)
    : mainWindow(window)
{
    if (window == nullptr)
        CRASH();

    layoutId = MainWindow::kLayoutGameUtilities;

    auto ButtonCheckGameFiles = new QPushButton("Check Game Files");
    ButtonCheckGameFiles->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonCheckGameFiles->setMinimumWidth(kButtonMinWidth);
    ButtonCheckGameFiles->setMinimumHeight(kButtonMinHeight);
    QFont ButtonFont = ButtonCheckGameFiles->font();
    ButtonFont.setPointSize(kFontSize);
    ButtonCheckGameFiles->setFont(ButtonFont);
    connect(ButtonCheckGameFiles, &QPushButton::clicked, this, &LayoutGameUtilities::CheckGameFilesSelected);

    auto ButtonChangeGamePath = new QPushButton("Change Game Path");
    ButtonChangeGamePath->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonChangeGamePath->setMinimumWidth(kButtonMinWidth);
    ButtonChangeGamePath->setMinimumHeight(kButtonMinHeight);
    ButtonChangeGamePath->setFont(ButtonFont);
    connect(ButtonChangeGamePath, &QPushButton::clicked, this, &LayoutGameUtilities::ChangeGamePathSelected);

    auto ButtonRepackGameFiles = new QPushButton("Repack Game Files");
    ButtonRepackGameFiles->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonRepackGameFiles->setMinimumWidth(kButtonMinWidth);
    ButtonRepackGameFiles->setMinimumHeight(kButtonMinHeight);
    ButtonRepackGameFiles->setFont(ButtonFont);
    connect(ButtonRepackGameFiles, &QPushButton::clicked, this, &LayoutGameUtilities::RepackGameFilesSelected);

    auto ButtonUpdateTOCs = new QPushButton("Update TOCs");
    ButtonUpdateTOCs->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonUpdateTOCs->setMinimumWidth(kButtonMinWidth);
    ButtonUpdateTOCs->setMinimumHeight(kButtonMinHeight);
    ButtonUpdateTOCs->setFont(ButtonFont);
    connect(ButtonUpdateTOCs, &QPushButton::clicked, this, &LayoutGameUtilities::UpdateTOCsSelected);

    auto ButtonExtractDLCs = new QPushButton("Extract DLCs");
    ButtonExtractDLCs->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonExtractDLCs->setMinimumWidth(kButtonMinWidth);
    ButtonExtractDLCs->setMinimumHeight(kButtonMinHeight);
    ButtonExtractDLCs->setFont(ButtonFont);
    connect(ButtonExtractDLCs, &QPushButton::clicked, this, &LayoutGameUtilities::ExtractDLCsSelected);

    auto ButtonReturn = new QPushButton("Return");
    ButtonReturn->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonReturn->setMinimumWidth(kButtonMinWidth);
    ButtonReturn->setMinimumHeight(kButtonMinHeight / 2);
    ButtonReturn->setFont(ButtonFont);
    connect(ButtonReturn, &QPushButton::clicked, this, &LayoutGameUtilities::ReturnSelected);

    auto *horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 40));
    auto *verticalLayout = new QVBoxLayout();
    verticalLayout->setAlignment(Qt::AlignVCenter);
    verticalLayout->addWidget(ButtonCheckGameFiles, 1);
    verticalLayout->addWidget(ButtonChangeGamePath, 1);
    verticalLayout->addWidget(ButtonRepackGameFiles, 1);
    verticalLayout->addWidget(ButtonUpdateTOCs, 1);
    verticalLayout->addWidget(ButtonExtractDLCs, 1);
    verticalLayout->addSpacing(20);
    verticalLayout->addWidget(ButtonReturn, 1);
    horizontalLayout->addLayout(verticalLayout);
    horizontalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 40));
}

void LayoutGameUtilities::CheckGameFilesSelected()
{
}

void LayoutGameUtilities::ChangeGamePathSelected()
{
}

void LayoutGameUtilities::RepackGameFilesSelected()
{
}

void LayoutGameUtilities::UpdateTOCsSelected()
{
}

void LayoutGameUtilities::ExtractDLCsSelected()
{
}

void LayoutGameUtilities::ReturnSelected()
{
    mainWindow->SwitchLayoutById(MainWindow::kLayoutModules);
    mainWindow->GetLayout()->removeWidget(this);
}
