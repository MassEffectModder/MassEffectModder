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

#include <Gui/MainWindow.h>
#include <Gui/LayoutMeSelect.h>
#include <Gui/LayoutModules.h>
#include <Helpers/Exception.h>
#include <Helpers/MiscHelpers.h>
#include <Types/MemTypes.h>

LayoutMeSelect::LayoutMeSelect(MainWindow *window)
    : mainWindow(window)
{
    layoutId = MainWindow::kLayoutMeSelect;

    QIcon iconME1(":/logo_me1.png");
    auto ButtonME1 = new QPushButton(iconME1, "");
    ButtonME1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonME1->setIconSize(QSize(kButtonMinWidth, kButtonMinHeight));
    connect(ButtonME1, &QPushButton::clicked, this, &LayoutMeSelect::ME1Selected);

    QIcon iconME2(":/logo_me2.png");
    auto ButtonME2 = new QPushButton(iconME2, "");
    ButtonME2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonME2->setIconSize(QSize(kButtonMinWidth, kButtonMinHeight));
    connect(ButtonME2, &QPushButton::clicked, this, &LayoutMeSelect::ME2Selected);

    QIcon iconME3(":/logo_me3.png");
    auto ButtonME3 = new QPushButton(iconME3, "");
    ButtonME3->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonME3->setIconSize(QSize(kButtonMinWidth, kButtonMinHeight));
    connect(ButtonME3, &QPushButton::clicked, this, &LayoutMeSelect::ME3Selected);

    auto ButtonExit = new QPushButton("Exit");
    QFont ButtonFont = ButtonME1->font();
    ButtonFont.setPointSize(kFontSize);
    ButtonExit->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    ButtonExit->setMinimumHeight(22);
    ButtonExit->setFont(ButtonFont);
    connect(ButtonExit, &QPushButton::clicked, this, &LayoutMeSelect::ExitSelected);

    auto horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 40));
    auto verticalLayout = new QVBoxLayout();
    verticalLayout->setAlignment(Qt::AlignVCenter);
    verticalLayout->addWidget(ButtonME1, 1);
    verticalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 1));
    verticalLayout->addWidget(ButtonME2, 1);
    verticalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 1));
    verticalLayout->addWidget(ButtonME3, 1);
    verticalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 1));
    verticalLayout->addWidget(ButtonExit, 1);

    auto GroupBoxView = new QGroupBox;
    GroupBoxView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    GroupBoxView->setAlignment(Qt::AlignBottom);
    GroupBoxView->setLayout(verticalLayout);

    horizontalLayout->addWidget(GroupBoxView);
    horizontalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 40));

    QString title = QString("Mass Effect Modder v%1 - ME Game Selection").arg(MEM_VERSION);
    if (DetectAdminRights())
        title += " (run as Administrator)";
    mainWindow->setWindowTitle(title);
}

void LayoutMeSelect::MESelected()
{
    mainWindow->GetLayout()->addWidget(new LayoutModules(mainWindow));
    mainWindow->SwitchLayoutById(MainWindow::kLayoutModules);
}

void LayoutMeSelect::ME1Selected()
{
    mainWindow->gameType = MeType::ME1_TYPE;
    MESelected();
}

void LayoutMeSelect::ME2Selected()
{
    mainWindow->gameType = MeType::ME2_TYPE;
    MESelected();
}

void LayoutMeSelect::ME3Selected()
{
    mainWindow->gameType = MeType::ME3_TYPE;
    MESelected();
}

void LayoutMeSelect::ExitSelected()
{
    QApplication::closeAllWindows();
}
