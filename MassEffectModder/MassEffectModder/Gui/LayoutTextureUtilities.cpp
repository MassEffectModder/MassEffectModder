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
#include "Gui/LayoutTextureUtilities.h"
#include "Gui/MainWindow.h"

LayoutTextureUtilities::LayoutTextureUtilities(QWidget *parent, QStackedLayout *layout, MainWindow *window)
    : QWidget(parent)
{
    if (layout == nullptr || window == nullptr)
        CRASH();
    stackedLayout = layout;
    mainWindow = window;

    auto ButtonRemoveScanFile = new QPushButton("Delete Textures Scan File");
    ButtonRemoveScanFile->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonRemoveScanFile->setMinimumWidth(kButtonMinWidth);
    ButtonRemoveScanFile->setMinimumHeight(kButtonMinHeight);
    QFont ButtonFont = ButtonRemoveScanFile->font();
    ButtonFont.setPointSize(kFontSize);
    ButtonRemoveScanFile->setFont(ButtonFont);
    connect(ButtonRemoveScanFile, &QPushButton::clicked, this, &LayoutTextureUtilities::RemoveScanFileSelected);

    auto ButtonApplyHQLODs = new QPushButton("Apply HQ LODs Settings");
    ButtonApplyHQLODs->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonApplyHQLODs->setMinimumWidth(kButtonMinWidth);
    ButtonApplyHQLODs->setMinimumHeight(kButtonMinHeight);
    ButtonApplyHQLODs->setFont(ButtonFont);
    connect(ButtonApplyHQLODs, &QPushButton::clicked, this, &LayoutTextureUtilities::ApplyHQLODsSelected);

    auto ButtonApplyVanillaLODs = new QPushButton("Apply Vanilla LODs Settings");
    ButtonApplyVanillaLODs->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonApplyVanillaLODs->setMinimumWidth(kButtonMinWidth);
    ButtonApplyVanillaLODs->setMinimumHeight(kButtonMinHeight);
    ButtonApplyVanillaLODs->setFont(ButtonFont);
    connect(ButtonApplyVanillaLODs, &QPushButton::clicked, this, &LayoutTextureUtilities::ApplyVanillaLODsSelected);

    auto ButtonApplyHQGfx = new QPushButton("Apply HQ GFX Settings");
    ButtonApplyHQGfx->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonApplyHQGfx->setMinimumWidth(kButtonMinWidth);
    ButtonApplyHQGfx->setMinimumHeight(kButtonMinHeight);
    ButtonApplyHQGfx->setFont(ButtonFont);
    connect(ButtonApplyHQGfx, &QPushButton::clicked, this, &LayoutTextureUtilities::ApplyHQGfxSelected);

    auto ButtonReturn = new QPushButton("Return");
    ButtonReturn->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonReturn->setMinimumWidth(kButtonMinWidth);
    ButtonReturn->setMinimumHeight(kButtonMinHeight / 2);
    ButtonReturn->setFont(ButtonFont);
    connect(ButtonReturn, &QPushButton::clicked, this, &LayoutTextureUtilities::ReturnSelected);

    auto *horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 40));
    auto *verticalLayout = new QVBoxLayout();
    verticalLayout->setAlignment(Qt::AlignVCenter);
    verticalLayout->addWidget(ButtonRemoveScanFile, 1);
    verticalLayout->addWidget(ButtonApplyHQLODs, 1);
    verticalLayout->addWidget(ButtonApplyVanillaLODs, 1);
    verticalLayout->addWidget(ButtonApplyHQGfx, 1);
    verticalLayout->addSpacing(20);
    verticalLayout->addWidget(ButtonReturn, 1);
    horizontalLayout->addLayout(verticalLayout);
    horizontalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 40));

    layout->addWidget(this);
}

void LayoutTextureUtilities::RemoveScanFileSelected()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Remove textures map of the game.");
    msgBox.setText("WARNING: you are going to delete your current textures scan file.");
    msgBox.setInformativeText(QString("After that, and before scanning your game again, ") +
                              "you need to restore game to vanilla state and reinstall vanilla DLCs and DLC mods." +
                              "\n\nAre you sure you want to proceed?");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Abort);
    msgBox.setDefaultButton(QMessageBox::Abort);
    int result = msgBox.exec();
    if (result == QMessageBox::Yes)
    {
        QString path = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation).first() +
                "/MassEffectModder";
        QString filename = path + QString("/me%1map.bin").arg((int)mainWindow->gameType);
        if (QFile(filename).exists())
        {
            QFile(filename).remove();
            QMessageBox::information(this, "Remove textures map of the game.",
                    QString("File at ") + filename + " deleted.");
        }
        else
        {
            QMessageBox::information(this, "Remove textures map of the game.",
                    QString("File at ") + filename + " not found.");
        }
    }
}

void LayoutTextureUtilities::ApplyHQLODsSelected()
{
}

void LayoutTextureUtilities::ApplyVanillaLODsSelected()
{
}

void LayoutTextureUtilities::ApplyHQGfxSelected()
{
}

void LayoutTextureUtilities::ReturnSelected()
{
    stackedLayout->setCurrentIndex(MainWindow::kLayoutModules);
}
