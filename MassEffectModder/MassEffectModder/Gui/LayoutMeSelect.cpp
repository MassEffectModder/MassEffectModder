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

#include "Gui/MainWindow.h"
#include "Gui/LayoutMeSelect.h"
#include "Gui/LayoutModules.h"
#include "Helpers/MiscHelpers.h"
#include "MemTypes.h"

LayoutMeSelect::LayoutMeSelect(MainWindow *window)
    : mainWindow(window)
{
    if (window == nullptr)
        CRASH();

    layoutId = MainWindow::kLayoutMeSelect;

    auto ButtonME1 = new QPushButton("Mass Effect");
    ButtonME1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonME1->setMinimumWidth(kButtonMinWidth);
    ButtonME1->setMinimumHeight(kButtonMinHeight);
    QFont ButtonFont = ButtonME1->font();
    ButtonFont.setPointSize(kFontSize);
    ButtonME1->setFont(ButtonFont);
    connect(ButtonME1, &QPushButton::clicked, this, &LayoutMeSelect::ME1Selected);

    auto ButtonME2 = new QPushButton("Mass Effect 2");
    ButtonME2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonME2->setMinimumWidth(kButtonMinWidth);
    ButtonME2->setMinimumHeight(kButtonMinHeight);
    ButtonME2->setFont(ButtonFont);
    connect(ButtonME2, &QPushButton::clicked, this, &LayoutMeSelect::ME2Selected);

    auto ButtonME3 = new QPushButton("Mass Effect 3");
    ButtonME3->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonME3->setMinimumWidth(kButtonMinWidth);
    ButtonME3->setMinimumHeight(kButtonMinHeight);
    ButtonME3->setFont(ButtonFont);
    connect(ButtonME3, &QPushButton::clicked, this, &LayoutMeSelect::ME3Selected);

    auto ButtonExit = new QPushButton("Exit");
    ButtonExit->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonExit->setMinimumWidth(kButtonMinWidth);
    ButtonExit->setMinimumHeight(kButtonMinHeight / 2);
    ButtonExit->setFont(ButtonFont);
    connect(ButtonExit, &QPushButton::clicked, this, &LayoutMeSelect::ExitSelected);

    auto *horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 40));
    auto *verticalLayout = new QVBoxLayout();
    verticalLayout->setAlignment(Qt::AlignVCenter);
    verticalLayout->addWidget(ButtonME1, 1);
    verticalLayout->addWidget(ButtonME2, 1);
    verticalLayout->addWidget(ButtonME3, 1);
    verticalLayout->addSpacing(20);
    verticalLayout->addWidget(ButtonExit, 1);
    horizontalLayout->addLayout(verticalLayout);
    horizontalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 40));
}

void LayoutMeSelect::ME1Selected()
{
    mainWindow->gameType = MeType::ME1_TYPE;
    QString title = QString("Mass Effect Modder v%1 - ME1").arg(MEM_VERSION);
    if (DetectAdminRights())
        title += " (run as Administrator)";
    mainWindow->setWindowTitle(title);
    mainWindow->GetLayout()->addWidget(new LayoutModules(mainWindow));
    mainWindow->SwitchLayoutById(MainWindow::kLayoutModules);
}

void LayoutMeSelect::ME2Selected()
{
    mainWindow->gameType = MeType::ME2_TYPE;
    QString title = QString("Mass Effect Modder v%1 - ME2").arg(MEM_VERSION);
    if (DetectAdminRights())
        title += " (run as Administrator)";
    mainWindow->setWindowTitle(title);
    mainWindow->GetLayout()->addWidget(new LayoutModules(mainWindow));
    mainWindow->SwitchLayoutById(MainWindow::kLayoutModules);
}

void LayoutMeSelect::ME3Selected()
{
    mainWindow->gameType = MeType::ME3_TYPE;
    QString title = QString("Mass Effect Modder v%1 - ME3").arg(MEM_VERSION);
    if (DetectAdminRights())
        title += " (run as Administrator)";
    mainWindow->setWindowTitle(title);
    mainWindow->GetLayout()->addWidget(new LayoutModules(mainWindow));
    mainWindow->SwitchLayoutById(MainWindow::kLayoutModules);
}

void LayoutMeSelect::ExitSelected()
{
    QApplication::closeAllWindows();
}
