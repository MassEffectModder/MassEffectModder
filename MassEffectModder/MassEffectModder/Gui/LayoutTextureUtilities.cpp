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
#include <Gui/LayoutTextureUtilities.h>
#include <Gui/MainWindow.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <Program/ConfigIni.h>
#include <GameData/GameData.h>
#include <GameData/LODSettings.h>
#include <Misc/Misc.h>

LayoutTextureUtilities::LayoutTextureUtilities(MainWindow *window)
    : mainWindow(window)
{
    layoutId = MainWindow::kLayoutTextureUtilities;

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

    QPushButton *ButtonApply2kLODs = nullptr;
    if (mainWindow->gameType == MeType::ME1_TYPE)
    {
        ButtonApply2kLODs = new QPushButton("Apply 2k LODs Settings");
        ButtonApply2kLODs->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
        ButtonApply2kLODs->setMinimumWidth(kButtonMinWidth);
        ButtonApply2kLODs->setMinimumHeight(kButtonMinHeight);
        ButtonApply2kLODs->setFont(ButtonFont);
        connect(ButtonApply2kLODs, &QPushButton::clicked, this, &LayoutTextureUtilities::Apply2kLODsSelected);
    }

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

    QPushButton *ButtonApplyHQGfxSoftShadows = nullptr;
    if (mainWindow->gameType == MeType::ME1_TYPE)
    {
        ButtonApplyHQGfxSoftShadows = new QPushButton("Apply HQ GFX Settings\n(Soft Shadows)");
        ButtonApplyHQGfxSoftShadows->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
        ButtonApplyHQGfxSoftShadows->setMinimumWidth(kButtonMinWidth);
        ButtonApplyHQGfxSoftShadows->setMinimumHeight(kButtonMinHeight);
        ButtonApplyHQGfxSoftShadows->setFont(ButtonFont);
        connect(ButtonApplyHQGfxSoftShadows, &QPushButton::clicked, this, &LayoutTextureUtilities::ApplyHQGfxSoftShadowsSelected);
    }

    auto ButtonReturn = new QPushButton("Return");
    ButtonReturn->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonReturn->setMinimumWidth(kButtonMinWidth);
    ButtonReturn->setMinimumHeight(kButtonMinHeight / 2);
    ButtonReturn->setFont(ButtonFont);
    connect(ButtonReturn, &QPushButton::clicked, this, &LayoutTextureUtilities::ReturnSelected);

    auto horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 40));
    auto verticalLayout = new QVBoxLayout();
    verticalLayout->setAlignment(Qt::AlignVCenter);
    verticalLayout->addWidget(ButtonRemoveScanFile, 1);
    verticalLayout->addWidget(ButtonApplyHQLODs, 1);
    if (mainWindow->gameType == MeType::ME1_TYPE)
    {
        verticalLayout->addWidget(ButtonApply2kLODs, 1);
    }
    verticalLayout->addWidget(ButtonApplyVanillaLODs, 1);
    verticalLayout->addWidget(ButtonApplyHQGfx, 1);
    if (mainWindow->gameType == MeType::ME1_TYPE)
    {
        verticalLayout->addWidget(ButtonApplyHQGfxSoftShadows, 1);
    }
    verticalLayout->addSpacing(20);
    verticalLayout->addWidget(ButtonReturn, 1);
    horizontalLayout->addLayout(verticalLayout);
    horizontalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 40));

    mainWindow->SetTitle("Texture Utilities");
}

void LayoutTextureUtilities::RemoveScanFileSelected()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Remove textures map of the game.");
    msgBox.setText("You are going to delete your current textures scan file.");
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
        QString filename = path + QString("/me%1map.bin").arg(static_cast<int>(mainWindow->gameType));
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

void LayoutTextureUtilities::ApplyLODs(bool lods2k)
{
    if (GameData::ConfigIniPath(mainWindow->gameType).length() == 0)
    {
        QMessageBox::critical(this, "Appling HQ LODs settings.",
                              "Game User path is not defined.");
        return;
    }
    QString path = GameData::EngineConfigIniPath(mainWindow->gameType);
    QDir().mkpath(DirName(path));
#if !defined(_WIN32)
    if (QFile(path).exists())
    {
        if (!Misc::ConvertEndLines(path, true))
            return;
    }
#endif
    ConfigIni engineConf = ConfigIni(path);
    LODSettings::updateLOD(mainWindow->gameType, engineConf, lods2k);
#if !defined(_WIN32)
    Misc::ConvertEndLines(path, false);
#endif

    QMessageBox::information(this, "Appling HQ LODs settings.",
            QString("Game configuration file at ") + path + " updated.");
}

void LayoutTextureUtilities::ApplyHQLODsSelected()
{
    ApplyLODs(false);
}

void LayoutTextureUtilities::Apply2kLODsSelected()
{
    ApplyLODs(true);
}

void LayoutTextureUtilities::ApplyVanillaLODsSelected()
{
    if (GameData::ConfigIniPath(mainWindow->gameType).length() == 0)
    {
        QMessageBox::critical(this, "Appling vanilla LODs settings.",
                              "Game User path is not defined.");
        return;
    }
    QString path = GameData::EngineConfigIniPath(mainWindow->gameType);
    QDir().mkpath(DirName(path));
#if !defined(_WIN32)
    if (QFile(path).exists())
    {
        if (!Misc::ConvertEndLines(path, true))
            return;
    }
#endif
    ConfigIni engineConf = ConfigIni(path);
    LODSettings::removeLOD(mainWindow->gameType, engineConf);
#if !defined(_WIN32)
    Misc::ConvertEndLines(path, false);
#endif

    QMessageBox::information(this, "Appling vanilla LODs settings.",
            QString("Game configuration file at ") + path + " updated.");
}

void LayoutTextureUtilities::ApplyHQGfx(bool softShadows)
{
    if (GameData::ConfigIniPath(mainWindow->gameType).length() == 0)
    {
        QMessageBox::critical(this, "Appling HQ gfx settings.",
                              "Game User path is not defined.");
        return;
    }
    QString path = GameData::EngineConfigIniPath(mainWindow->gameType);
    QDir().mkpath(DirName(path));
#if !defined(_WIN32)
    if (QFile(path).exists())
    {
        if (!Misc::ConvertEndLines(path, true))
            return;
    }
#endif
    ConfigIni engineConf = ConfigIni(path);
    LODSettings::updateGFXSettings(mainWindow->gameType, engineConf, softShadows, false);
#if !defined(_WIN32)
    Misc::ConvertEndLines(path, false);
#endif

    QMessageBox::information(this, "Appling HQ gfx settings.",
            QString("Game configuration file at ") + path + " updated.");
}

void LayoutTextureUtilities::ApplyHQGfxSelected()
{
    ApplyHQGfx(false);
}

void LayoutTextureUtilities::ApplyHQGfxSoftShadowsSelected()
{
    ApplyHQGfx(true);
}

void LayoutTextureUtilities::ReturnSelected()
{
    mainWindow->SwitchLayoutById(MainWindow::kLayoutModules);
    mainWindow->GetLayout()->removeWidget(this);
    mainWindow->SetTitle("Modules Selection");
}
