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
#include <Gui/LayoutMain.h>
#include <Gui/LayoutModules.h>
#include <Helpers/Exception.h>
#include <Helpers/MiscHelpers.h>
#include <Types/MemTypes.h>

LayoutMain::LayoutMain(MainWindow *window)
    : mainWindow(window)
{
    layoutId = MainWindow::kLayoutMain;

    QPixmap pixmapME1(QString(":/logo_me1.png"));
    pixmapME1 = pixmapME1.scaled(300, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    auto iconME1Logo = new QLabel;
    iconME1Logo->setPixmap(pixmapME1);
    iconME1Logo->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    QPixmap pixmapME2(QString(":/logo_me2.png"));
    pixmapME2 = pixmapME2.scaled(300, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    auto iconME2Logo = new QLabel;
    iconME2Logo->setPixmap(pixmapME2);
    iconME2Logo->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    QPixmap pixmapME3(QString(":/logo_me3.png"));
    pixmapME3 = pixmapME3.scaled(300, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    auto iconME3Logo = new QLabel;
    iconME3Logo->setPixmap(pixmapME3);
    iconME3Logo->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    auto ButtonTexturesManagerME1 = new QPushButton("Textures Manager");
    ButtonTexturesManagerME1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonTexturesManagerME1->setMinimumWidth(kButtonMinWidth);
    ButtonTexturesManagerME1->setMinimumHeight(kButtonMinHeight);
    QFont ButtonFont = ButtonTexturesManagerME1->font();
    ButtonFont.setPointSize(kFontSize);
    ButtonTexturesManagerME1->setFont(ButtonFont);
    //connect(ButtonTexturesManagerME1, &QPushButton::clicked, this, &LayoutModules::TexturesManagerSelected);

    auto ButtonTexturesManagerME2 = new QPushButton("Textures Manager");
    ButtonTexturesManagerME2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonTexturesManagerME2->setMinimumWidth(kButtonMinWidth);
    ButtonTexturesManagerME2->setMinimumHeight(kButtonMinHeight);
    ButtonTexturesManagerME2->setFont(ButtonFont);
    //connect(ButtonTexturesManagerME2, &QPushButton::clicked, this, &LayoutModules::TexturesManagerSelected);

    auto ButtonTexturesManagerME3 = new QPushButton("Textures Manager");
    ButtonTexturesManagerME3->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonTexturesManagerME3->setMinimumWidth(kButtonMinWidth);
    ButtonTexturesManagerME3->setMinimumHeight(kButtonMinHeight);
    ButtonTexturesManagerME3->setFont(ButtonFont);
    //connect(ButtonTexturesManagerME3, &QPushButton::clicked, this, &LayoutModules::TexturesManagerSelected);

    buttonTextureUtilitiesME1 = new QPushButton("Texture Utilities");
    buttonTextureUtilitiesME1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonTextureUtilitiesME1->setMinimumWidth(kButtonMinWidth);
    buttonTextureUtilitiesME1->setMinimumHeight(kButtonMinHeight);
    buttonTextureUtilitiesME1->setFont(ButtonFont);
    buttonTextureUtilitiesME1->setCheckable(true);
    connect(buttonTextureUtilitiesME1, &QPushButton::clicked, this, &LayoutMain::ButtonTextureUtilitiesME1Selected);

    buttonTextureUtilitiesME2 = new QPushButton("Texture Utilities");
    buttonTextureUtilitiesME2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonTextureUtilitiesME2->setMinimumWidth(kButtonMinWidth);
    buttonTextureUtilitiesME2->setMinimumHeight(kButtonMinHeight);
    buttonTextureUtilitiesME2->setFont(ButtonFont);
    buttonTextureUtilitiesME2->setCheckable(true);
    connect(buttonTextureUtilitiesME2, &QPushButton::clicked, this, &LayoutMain::ButtonTextureUtilitiesME2Selected);

    buttonTextureUtilitiesME3 = new QPushButton("Texture Utilities");
    buttonTextureUtilitiesME3->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonTextureUtilitiesME3->setMinimumWidth(kButtonMinWidth);
    buttonTextureUtilitiesME3->setMinimumHeight(kButtonMinHeight);
    buttonTextureUtilitiesME3->setFont(ButtonFont);
    buttonTextureUtilitiesME3->setCheckable(true);
    connect(buttonTextureUtilitiesME3, &QPushButton::clicked, this, &LayoutMain::ButtonTextureUtilitiesME3Selected);

    buttonGameUtilitiesME1 = new QPushButton("Game Utilities");
    buttonGameUtilitiesME1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonGameUtilitiesME1->setMinimumWidth(kButtonMinWidth);
    buttonGameUtilitiesME1->setMinimumHeight(kButtonMinHeight);
    buttonGameUtilitiesME1->setFont(ButtonFont);
    buttonGameUtilitiesME1->setCheckable(true);
    connect(buttonGameUtilitiesME1, &QPushButton::clicked, this, &LayoutMain::ButtonGameUtilitiesME1Selected);

    buttonGameUtilitiesME2 = new QPushButton("Game Utilities");
    buttonGameUtilitiesME2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonGameUtilitiesME2->setMinimumWidth(kButtonMinWidth);
    buttonGameUtilitiesME2->setMinimumHeight(kButtonMinHeight);
    buttonGameUtilitiesME2->setFont(ButtonFont);
    buttonGameUtilitiesME2->setCheckable(true);
    connect(buttonGameUtilitiesME2, &QPushButton::clicked, this, &LayoutMain::ButtonGameUtilitiesME2Selected);

    buttonGameUtilitiesME3 = new QPushButton("Game Utilities");
    buttonGameUtilitiesME3->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonGameUtilitiesME3->setMinimumWidth(kButtonMinWidth);
    buttonGameUtilitiesME3->setMinimumHeight(kButtonMinHeight);
    buttonGameUtilitiesME3->setFont(ButtonFont);
    buttonGameUtilitiesME3->setCheckable(true);
    connect(buttonGameUtilitiesME3, &QPushButton::clicked, this, &LayoutMain::ButtonGameUtilitiesME3Selected);

    buttonModsManagerME1 = new QPushButton("Mods Manager");
    buttonModsManagerME1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonModsManagerME1->setMinimumWidth(kButtonMinWidth);
    buttonModsManagerME1->setMinimumHeight(kButtonMinHeight);
    buttonModsManagerME1->setFont(ButtonFont);
    buttonModsManagerME1->setCheckable(true);
    connect(buttonModsManagerME1, &QPushButton::clicked, this, &LayoutMain::ButtonModsManagerME1Selected);

    buttonModsManagerME2 = new QPushButton("Mods Manager");
    buttonModsManagerME2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonModsManagerME2->setMinimumWidth(kButtonMinWidth);
    buttonModsManagerME2->setMinimumHeight(kButtonMinHeight);
    buttonModsManagerME2->setFont(ButtonFont);
    buttonModsManagerME2->setCheckable(true);
    connect(buttonModsManagerME2, &QPushButton::clicked, this, &LayoutMain::ButtonModsManagerME2Selected);

    buttonModsManagerME3 = new QPushButton("Mods Manager");
    buttonModsManagerME3->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonModsManagerME3->setMinimumWidth(kButtonMinWidth);
    buttonModsManagerME3->setMinimumHeight(kButtonMinHeight);
    buttonModsManagerME3->setFont(ButtonFont);
    buttonModsManagerME3->setCheckable(true);
    connect(buttonModsManagerME3, &QPushButton::clicked, this, &LayoutMain::ButtonModsManagerME3Selected);


    buttonRemoveScanFileME1 = new QPushButton("Delete Textures Scan File");
    buttonRemoveScanFileME1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonRemoveScanFileME1->setMinimumWidth(kButtonMinWidth);
    buttonRemoveScanFileME1->setMinimumHeight(kButtonMinHeight);
    buttonRemoveScanFileME1->setFont(ButtonFont);
    //connect(buttonRemoveScanFileME1, &QPushButton::clicked, this, &LayoutTextureUtilities::RemoveScanFileSelected);

    buttonRemoveScanFileME2 = new QPushButton("Delete Textures Scan File");
    buttonRemoveScanFileME2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonRemoveScanFileME2->setMinimumWidth(kButtonMinWidth);
    buttonRemoveScanFileME2->setMinimumHeight(kButtonMinHeight);
    buttonRemoveScanFileME2->setFont(ButtonFont);
    //connect(buttonRemoveScanFileME2, &QPushButton::clicked, this, &LayoutTextureUtilities::RemoveScanFileSelected);

    buttonRemoveScanFileME3 = new QPushButton("Delete Textures Scan File");
    buttonRemoveScanFileME3->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonRemoveScanFileME3->setMinimumWidth(kButtonMinWidth);
    buttonRemoveScanFileME3->setMinimumHeight(kButtonMinHeight);
    buttonRemoveScanFileME3->setFont(ButtonFont);
    //connect(buttonRemoveScanFileME3, &QPushButton::clicked, this, &LayoutTextureUtilities::RemoveScanFileSelected);

    buttonApplyHQLODsME1 = new QPushButton("Apply HQ LODs Settings");
    buttonApplyHQLODsME1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonApplyHQLODsME1->setMinimumWidth(kButtonMinWidth);
    buttonApplyHQLODsME1->setMinimumHeight(kButtonMinHeight);
    buttonApplyHQLODsME1->setFont(ButtonFont);
    //connect(buttonApplyHQLODsME1, &QPushButton::clicked, this, &LayoutTextureUtilities::ApplyHQLODsSelected);

    buttonApplyHQLODsME2 = new QPushButton("Apply HQ LODs Settings");
    buttonApplyHQLODsME2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonApplyHQLODsME2->setMinimumWidth(kButtonMinWidth);
    buttonApplyHQLODsME2->setMinimumHeight(kButtonMinHeight);
    buttonApplyHQLODsME2->setFont(ButtonFont);
    //connect(buttonApplyHQLODsME2, &QPushButton::clicked, this, &LayoutTextureUtilities::ApplyHQLODsSelected);

    buttonApplyHQLODsME3 = new QPushButton("Apply HQ LODs Settings");
    buttonApplyHQLODsME3->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonApplyHQLODsME3->setMinimumWidth(kButtonMinWidth);
    buttonApplyHQLODsME3->setMinimumHeight(kButtonMinHeight);
    buttonApplyHQLODsME3->setFont(ButtonFont);
    //connect(buttonApplyHQLODsME3, &QPushButton::clicked, this, &LayoutTextureUtilities::ApplyHQLODsSelected);

    buttonApply2kLODsME1 = new QPushButton("Apply 2k LODs Settings");
    buttonApply2kLODsME1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonApply2kLODsME1->setMinimumWidth(kButtonMinWidth);
    buttonApply2kLODsME1->setMinimumHeight(kButtonMinHeight);
    buttonApply2kLODsME1->setFont(ButtonFont);
    //connect(buttonApply2kLODsME1, &QPushButton::clicked, this, &LayoutTextureUtilities::Apply2kLODsSelected);

    buttonApplyVanillaLODsME1 = new QPushButton("Apply Vanilla LODs Settings");
    buttonApplyVanillaLODsME1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonApplyVanillaLODsME1->setMinimumWidth(kButtonMinWidth);
    buttonApplyVanillaLODsME1->setMinimumHeight(kButtonMinHeight);
    buttonApplyVanillaLODsME1->setFont(ButtonFont);
    //connect(buttonApplyVanillaLODsME2, &QPushButton::clicked, this, &LayoutTextureUtilities::ApplyVanillaLODsSelected);

    buttonApplyVanillaLODsME2 = new QPushButton("Apply Vanilla LODs Settings");
    buttonApplyVanillaLODsME2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonApplyVanillaLODsME2->setMinimumWidth(kButtonMinWidth);
    buttonApplyVanillaLODsME2->setMinimumHeight(kButtonMinHeight);
    buttonApplyVanillaLODsME2->setFont(ButtonFont);
    //connect(buttonApplyVanillaLODsME2, &QPushButton::clicked, this, &LayoutTextureUtilities::ApplyVanillaLODsSelected);

    buttonApplyVanillaLODsME3 = new QPushButton("Apply Vanilla LODs Settings");
    buttonApplyVanillaLODsME3->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonApplyVanillaLODsME3->setMinimumWidth(kButtonMinWidth);
    buttonApplyVanillaLODsME3->setMinimumHeight(kButtonMinHeight);
    buttonApplyVanillaLODsME3->setFont(ButtonFont);
    //connect(buttonApplyVanillaLODsME3, &QPushButton::clicked, this, &LayoutTextureUtilities::ApplyVanillaLODsSelected);

    buttonApplyHQGfxME1 = new QPushButton("Apply HQ GFX Settings");
    buttonApplyHQGfxME1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonApplyHQGfxME1->setMinimumWidth(kButtonMinWidth);
    buttonApplyHQGfxME1->setMinimumHeight(kButtonMinHeight);
    buttonApplyHQGfxME1->setFont(ButtonFont);
    //connect(buttonApplyHQGfxME1, &QPushButton::clicked, this, &LayoutTextureUtilities::ApplyHQGfxSelected);

    buttonApplyHQGfxME2 = new QPushButton("Apply HQ GFX Settings");
    buttonApplyHQGfxME2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonApplyHQGfxME2->setMinimumWidth(kButtonMinWidth);
    buttonApplyHQGfxME2->setMinimumHeight(kButtonMinHeight);
    buttonApplyHQGfxME2->setFont(ButtonFont);
    //connect(buttonApplyHQGfxME2, &QPushButton::clicked, this, &LayoutTextureUtilities::ApplyHQGfxSelected);

    buttonApplyHQGfxME3 = new QPushButton("Apply HQ GFX Settings");
    buttonApplyHQGfxME3->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonApplyHQGfxME3->setMinimumWidth(kButtonMinWidth);
    buttonApplyHQGfxME3->setMinimumHeight(kButtonMinHeight);
    buttonApplyHQGfxME3->setFont(ButtonFont);
    //connect(buttonApplyHQGfxME3, &QPushButton::clicked, this, &LayoutTextureUtilities::ApplyHQGfxSelected);

    buttonApplyHQGfxSoftShadows = new QPushButton("Apply HQ GFX Settings (Soft)");
    buttonApplyHQGfxSoftShadows->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonApplyHQGfxSoftShadows->setMinimumWidth(kButtonMinWidth);
    buttonApplyHQGfxSoftShadows->setMinimumHeight(kButtonMinHeight);
    buttonApplyHQGfxSoftShadows->setFont(ButtonFont);
    //connect(buttonApplyHQGfxSoftShadows, &QPushButton::clicked, this, &LayoutTextureUtilities::ApplyHQGfxSoftShadowsSelected);

    buttonCheckGameFilesME1 = new QPushButton("Check Game Files");
    buttonCheckGameFilesME1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonCheckGameFilesME1->setMinimumWidth(kButtonMinWidth);
    buttonCheckGameFilesME1->setMinimumHeight(kButtonMinHeight);
    buttonCheckGameFilesME1->setFont(ButtonFont);
    //connect(buttonCheckGameFilesME1, &QPushButton::clicked, this, &LayoutGameUtilities::CheckGameFilesSelected);

    buttonCheckGameFilesME2 = new QPushButton("Check Game Files");
    buttonCheckGameFilesME2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonCheckGameFilesME2->setMinimumWidth(kButtonMinWidth);
    buttonCheckGameFilesME2->setMinimumHeight(kButtonMinHeight);
    buttonCheckGameFilesME2->setFont(ButtonFont);
    //connect(buttonCheckGameFilesME2, &QPushButton::clicked, this, &LayoutGameUtilities::CheckGameFilesSelected);

    buttonCheckGameFilesME3 = new QPushButton("Check Game Files");
    buttonCheckGameFilesME3->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonCheckGameFilesME3->setMinimumWidth(kButtonMinWidth);
    buttonCheckGameFilesME3->setMinimumHeight(kButtonMinHeight);
    ButtonFont.setPointSize(kFontSize);
    buttonCheckGameFilesME3->setFont(ButtonFont);
    //connect(buttonCheckGameFilesME3, &QPushButton::clicked, this, &LayoutGameUtilities::CheckGameFilesSelected);

    buttonRepackGameFilesME2 = new QPushButton("Repack Game Files");
    buttonRepackGameFilesME2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonRepackGameFilesME2->setMinimumWidth(kButtonMinWidth);
    buttonRepackGameFilesME2->setMinimumHeight(kButtonMinHeight);
    buttonRepackGameFilesME2->setFont(ButtonFont);
    //connect(buttonRepackGameFilesME2, &QPushButton::clicked, this, &LayoutGameUtilities::RepackGameFilesSelected);

    buttonRepackGameFilesME3 = new QPushButton("Repack Game Files");
    buttonRepackGameFilesME3->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonRepackGameFilesME3->setMinimumWidth(kButtonMinWidth);
    buttonRepackGameFilesME3->setMinimumHeight(kButtonMinHeight);
    buttonRepackGameFilesME3->setFont(ButtonFont);
    //connect(buttonRepackGameFilesME3, &QPushButton::clicked, this, &LayoutGameUtilities::RepackGameFilesSelected);

    buttonUpdateTOCs = new QPushButton("Update TOCs");
    buttonUpdateTOCs->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonUpdateTOCs->setMinimumWidth(kButtonMinWidth);
    buttonUpdateTOCs->setMinimumHeight(kButtonMinHeight);
    buttonUpdateTOCs->setFont(ButtonFont);
    //connect(buttonUpdateTOCs, &QPushButton::clicked, this, &LayoutGameUtilities::UpdateTOCsSelected);

    buttonExtractDLCs = new QPushButton("Unpack DLCs");
    buttonExtractDLCs->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonExtractDLCs->setMinimumWidth(kButtonMinWidth);
    buttonExtractDLCs->setMinimumHeight(kButtonMinHeight);
    buttonExtractDLCs->setFont(ButtonFont);
    //connect(buttonExtractDLCs, &QPushButton::clicked, this, &LayoutGameUtilities::ExtractDLCsSelected);

    buttonChangeGamePathME1 = new QPushButton("Change Game Path");
    buttonChangeGamePathME1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonChangeGamePathME1->setMinimumWidth(kButtonMinWidth);
    buttonChangeGamePathME1->setMinimumHeight(kButtonMinHeight);
    buttonChangeGamePathME1->setFont(ButtonFont);
    //connect(buttonChangeGamePathME1, &QPushButton::clicked, this, &LayoutGameUtilities::ChangeGamePathSelected);

    buttonChangeGamePathME2 = new QPushButton("Change Game Path");
    buttonChangeGamePathME2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonChangeGamePathME2->setMinimumWidth(kButtonMinWidth);
    buttonChangeGamePathME2->setMinimumHeight(kButtonMinHeight);
    buttonChangeGamePathME2->setFont(ButtonFont);
    //connect(buttonChangeGamePathME2, &QPushButton::clicked, this, &LayoutGameUtilities::ChangeGamePathSelected);

    buttonChangeGamePathME3 = new QPushButton("Change Game Path");
    buttonChangeGamePathME3->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonChangeGamePathME3->setMinimumWidth(kButtonMinWidth);
    buttonChangeGamePathME3->setMinimumHeight(kButtonMinHeight);
    buttonChangeGamePathME3->setFont(ButtonFont);
    //connect(buttonChangeGamePathME3, &QPushButton::clicked, this, &LayoutGameUtilities::ChangeGamePathSelected);

#if !defined(_WIN32)
    buttonChangeUserPathME1 = new QPushButton("Change User Path");
    buttonChangeUserPathME1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonChangeUserPathME1->setMinimumWidth(kButtonMinWidth);
    buttonChangeUserPathME1->setMinimumHeight(kButtonMinHeight);
    buttonChangeUserPathME1->setFont(ButtonFont);
    //connect(buttonChangeUserPathME1, &QPushButton::clicked, this, &LayoutGameUtilities::ChangeUserPathSelected);

    buttonChangeUserPathME2 = new QPushButton("Change User Path");
    buttonChangeUserPathME2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonChangeUserPathME2->setMinimumWidth(kButtonMinWidth);
    buttonChangeUserPathME2->setMinimumHeight(kButtonMinHeight);
    buttonChangeUserPathME2->setFont(ButtonFont);
    //connect(buttonChangeUserPathME2, &QPushButton::clicked, this, &LayoutGameUtilities::ChangeUserPathSelected);

    buttonChangeUserPathME3 = new QPushButton("Change User Path");
    buttonChangeUserPathME3->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonChangeUserPathME3->setMinimumWidth(kButtonMinWidth);
    buttonChangeUserPathME3->setMinimumHeight(kButtonMinHeight);
    buttonChangeUserPathME3->setFont(ButtonFont);
    //connect(buttonChangeUserPathME3, &QPushButton::clicked, this, &LayoutGameUtilities::ChangeUserPathSelected);
#endif

    buttonInstallModsME1 = new QPushButton("Mods Installer");
    buttonInstallModsME1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonInstallModsME1->setMinimumWidth(kButtonMinWidth);
    buttonInstallModsME1->setMinimumHeight(kButtonMinHeight);
    buttonInstallModsME1->setFont(ButtonFont);
    //connect(buttonInstallModsME1, &QPushButton::clicked, this, &LayoutModsManager::InstallModsSelected);

    buttonInstallModsME2 = new QPushButton("Mods Installer");
    buttonInstallModsME2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonInstallModsME2->setMinimumWidth(kButtonMinWidth);
    buttonInstallModsME2->setMinimumHeight(kButtonMinHeight);
    buttonInstallModsME2->setFont(ButtonFont);
    //connect(buttonInstallModsME2, &QPushButton::clicked, this, &LayoutModsManager::InstallModsSelected);

    buttonInstallModsME3 = new QPushButton("Mods Installer");
    buttonInstallModsME3->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonInstallModsME3->setMinimumWidth(kButtonMinWidth);
    buttonInstallModsME3->setMinimumHeight(kButtonMinHeight);
    buttonInstallModsME3->setFont(ButtonFont);
    //connect(buttonInstallModsME3, &QPushButton::clicked, this, &LayoutModsManager::InstallModsSelected);

    buttonExtractModsME1 = new QPushButton("Extract Mods");
    buttonExtractModsME1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonExtractModsME1->setMinimumWidth(kButtonMinWidth);
    buttonExtractModsME1->setMinimumHeight(kButtonMinHeight);
    buttonExtractModsME1->setFont(ButtonFont);
    //connect(buttonExtractModsME1, &QPushButton::clicked, this, &LayoutModsManager::ExtractModsSelected);

    buttonExtractModsME2 = new QPushButton("Extract Mods");
    buttonExtractModsME2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonExtractModsME2->setMinimumWidth(kButtonMinWidth);
    buttonExtractModsME2->setMinimumHeight(kButtonMinHeight);
    buttonExtractModsME2->setFont(ButtonFont);
    //connect(buttonExtractModsME2, &QPushButton::clicked, this, &LayoutModsManager::ExtractModsSelected);

    buttonExtractModsME3 = new QPushButton("Extract Mods");
    buttonExtractModsME3->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonExtractModsME3->setMinimumWidth(kButtonMinWidth);
    buttonExtractModsME3->setMinimumHeight(kButtonMinHeight);
    buttonExtractModsME3->setFont(ButtonFont);
    //connect(buttonExtractModsME3, &QPushButton::clicked, this, &LayoutModsManager::ExtractModsSelected);

    buttonConvertModME1 = new QPushButton("Convert Mod");
    buttonConvertModME1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonConvertModME1->setMinimumWidth(kButtonMinWidth);
    buttonConvertModME1->setMinimumHeight(kButtonMinHeight);
    buttonConvertModME1->setFont(ButtonFont);
    //connect(buttonConvertModME1, &QPushButton::clicked, this, &LayoutModsManager::ConvertModSelected);

    buttonConvertModME2 = new QPushButton("Convert Mod");
    buttonConvertModME2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonConvertModME2->setMinimumWidth(kButtonMinWidth);
    buttonConvertModME2->setMinimumHeight(kButtonMinHeight);
    buttonConvertModME2->setFont(ButtonFont);
    //connect(buttonConvertModME2, &QPushButton::clicked, this, &LayoutModsManager::ConvertModSelected);

    buttonConvertModME3 = new QPushButton("Convert Mod");
    buttonConvertModME3->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonConvertModME3->setMinimumWidth(kButtonMinWidth);
    buttonConvertModME3->setMinimumHeight(kButtonMinHeight);
    buttonConvertModME3->setFont(ButtonFont);
    //connect(buttonConvertModME3, &QPushButton::clicked, this, &LayoutModsManager::ConvertModSelected);

    buttonCreateModME1 = new QPushButton("Create Mod");
    buttonCreateModME1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonCreateModME1->setMinimumWidth(kButtonMinWidth);
    buttonCreateModME1->setMinimumHeight(kButtonMinHeight);
    buttonCreateModME1->setFont(ButtonFont);
    //connect(buttonCreateModME1, &QPushButton::clicked, this, &LayoutModsManager::CreateModSelected);

    buttonCreateModME2 = new QPushButton("Create Mod");
    buttonCreateModME2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonCreateModME2->setMinimumWidth(kButtonMinWidth);
    buttonCreateModME2->setMinimumHeight(kButtonMinHeight);
    buttonCreateModME2->setFont(ButtonFont);
    //connect(buttonCreateModME2, &QPushButton::clicked, this, &LayoutModsManager::CreateModSelected);

    buttonCreateModME3 = new QPushButton("Create Mod");
    buttonCreateModME3->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonCreateModME3->setMinimumWidth(kButtonMinWidth);
    buttonCreateModME3->setMinimumHeight(kButtonMinHeight);
    buttonCreateModME3->setFont(ButtonFont);
    //connect(buttonCreateModME3, &QPushButton::clicked, this, &LayoutModsManager::CreateModSelected);

    buttonCreateBinaryModME1 = new QPushButton("Create Binary Mod");
    buttonCreateBinaryModME1->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonCreateBinaryModME1->setMinimumWidth(kButtonMinWidth);
    buttonCreateBinaryModME1->setMinimumHeight(kButtonMinHeight);
    buttonCreateBinaryModME1->setFont(ButtonFont);
    //connect(buttonCreateBinaryModME1, &QPushButton::clicked, this, &LayoutModsManager::CreateBinaryModSelected);

    buttonCreateBinaryModME2 = new QPushButton("Create Binary Mod");
    buttonCreateBinaryModME2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonCreateBinaryModME2->setMinimumWidth(kButtonMinWidth);
    buttonCreateBinaryModME2->setMinimumHeight(kButtonMinHeight);
    buttonCreateBinaryModME2->setFont(ButtonFont);
    //connect(buttonCreateBinaryModME2, &QPushButton::clicked, this, &LayoutModsManager::CreateBinaryModSelected);

    buttonCreateBinaryModME3 = new QPushButton("Create Binary Mod");
    buttonCreateBinaryModME3->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    buttonCreateBinaryModME3->setMinimumWidth(kButtonMinWidth);
    buttonCreateBinaryModME3->setMinimumHeight(kButtonMinHeight);
    buttonCreateBinaryModME3->setFont(ButtonFont);
    //connect(buttonCreateBinaryModME3, &QPushButton::clicked, this, &LayoutModsManager::CreateBinaryModSelected);


    auto verticalLayoutMenuME1 = new QVBoxLayout();
    verticalLayoutMenuME1->setAlignment(Qt::AlignCenter);
    verticalLayoutMenuME1->addWidget(ButtonTexturesManagerME1, 1);

    auto verticalLayoutMenuME2 = new QVBoxLayout();
    verticalLayoutMenuME2->setAlignment(Qt::AlignCenter);
    verticalLayoutMenuME2->addWidget(ButtonTexturesManagerME2, 1);

    auto verticalLayoutMenuME3 = new QVBoxLayout();
    verticalLayoutMenuME3->setAlignment(Qt::AlignCenter);
    verticalLayoutMenuME3->addWidget(ButtonTexturesManagerME3, 1);

    verticalLayoutMenuME1->addSpacing(10);
    verticalLayoutMenuME1->addWidget(buttonTextureUtilitiesME1, 1);
    verticalLayoutMenuME1->addWidget(buttonRemoveScanFileME1, 1);
    verticalLayoutMenuME1->addWidget(buttonApplyHQLODsME1, 1);
    verticalLayoutMenuME1->addWidget(buttonApply2kLODsME1, 1);
    verticalLayoutMenuME1->addWidget(buttonApplyVanillaLODsME1, 1);
    verticalLayoutMenuME1->addWidget(buttonApplyHQGfxME1, 1);
    verticalLayoutMenuME1->addWidget(buttonApplyHQGfxSoftShadows, 1);
    spacerBottomTextureUtilitiesME1 = new QSpacerItem(0, 10);
    verticalLayoutMenuME1->addItem(spacerBottomTextureUtilitiesME1);

    verticalLayoutMenuME2->addSpacing(10);
    verticalLayoutMenuME2->addWidget(buttonTextureUtilitiesME2, 1);
    verticalLayoutMenuME2->addWidget(buttonRemoveScanFileME2, 1);
    verticalLayoutMenuME2->addWidget(buttonApplyHQLODsME2, 1);
    verticalLayoutMenuME2->addWidget(buttonApplyVanillaLODsME2, 1);
    verticalLayoutMenuME2->addWidget(buttonApplyHQGfxME2, 1);
    spacerBottomTextureUtilitiesME2 = new QSpacerItem(0, 10);
    verticalLayoutMenuME2->addItem(spacerBottomTextureUtilitiesME2);

    verticalLayoutMenuME3->addSpacing(10);
    verticalLayoutMenuME3->addWidget(buttonTextureUtilitiesME3, 1);
    verticalLayoutMenuME3->addWidget(buttonRemoveScanFileME3, 1);
    verticalLayoutMenuME3->addWidget(buttonApplyHQLODsME3, 1);
    verticalLayoutMenuME3->addWidget(buttonApplyVanillaLODsME3, 1);
    verticalLayoutMenuME3->addWidget(buttonApplyHQGfxME3, 1);
    spacerBottomTextureUtilitiesME3 = new QSpacerItem(0, 10);
    verticalLayoutMenuME3->addItem(spacerBottomTextureUtilitiesME3);

    verticalLayoutMenuME1->addSpacing(10);
    verticalLayoutMenuME1->addWidget(buttonGameUtilitiesME1, 1);
    verticalLayoutMenuME1->addWidget(buttonCheckGameFilesME1, 1);
    verticalLayoutMenuME1->addWidget(buttonChangeGamePathME1, 1);
#if !defined(_WIN32)
    verticalLayoutMenuME1->addWidget(buttonChangeUserPathME1, 1);
#endif
    spacerBottomGameUtilitiesME1 = new QSpacerItem(0, 10);
    verticalLayoutMenuME1->addItem(spacerBottomGameUtilitiesME1);

    verticalLayoutMenuME2->addSpacing(10);
    verticalLayoutMenuME2->addWidget(buttonGameUtilitiesME2, 1);
    verticalLayoutMenuME2->addWidget(buttonCheckGameFilesME2, 1);
    verticalLayoutMenuME2->addWidget(buttonRepackGameFilesME2, 1);
    verticalLayoutMenuME2->addWidget(buttonChangeGamePathME2, 1);
#if !defined(_WIN32)
    verticalLayoutMenuME2->addWidget(buttonChangeUserPathME2, 1);
#endif
    spacerBottomGameUtilitiesME2 = new QSpacerItem(0, 10);
    verticalLayoutMenuME2->addItem(spacerBottomGameUtilitiesME2);

    verticalLayoutMenuME3->addSpacing(10);
    verticalLayoutMenuME3->addWidget(buttonGameUtilitiesME3, 1);
    verticalLayoutMenuME3->addWidget(buttonCheckGameFilesME3, 1);
    verticalLayoutMenuME3->addWidget(buttonRepackGameFilesME3, 1);
    verticalLayoutMenuME3->addWidget(buttonUpdateTOCs, 1);
    verticalLayoutMenuME3->addWidget(buttonExtractDLCs, 1);
    verticalLayoutMenuME3->addWidget(buttonChangeGamePathME3, 1);
#if !defined(_WIN32)
    verticalLayoutMenuME3->addWidget(buttonChangeUserPathME3, 1);
#endif
    spacerBottomGameUtilitiesME3 = new QSpacerItem(0, 10);
    verticalLayoutMenuME3->addItem(spacerBottomGameUtilitiesME3);

    verticalLayoutMenuME1->addSpacing(10);
    verticalLayoutMenuME1->addWidget(buttonModsManagerME1, 1);
    verticalLayoutMenuME1->addWidget(buttonInstallModsME1, 1);
    verticalLayoutMenuME1->addWidget(buttonExtractModsME1, 1);
    verticalLayoutMenuME1->addWidget(buttonConvertModME1, 1);
    verticalLayoutMenuME1->addWidget(buttonCreateModME1, 1);
    verticalLayoutMenuME1->addWidget(buttonCreateBinaryModME1, 1);

    verticalLayoutMenuME2->addSpacing(10);
    verticalLayoutMenuME2->addWidget(buttonModsManagerME2, 1);
    verticalLayoutMenuME2->addWidget(buttonInstallModsME2, 1);
    verticalLayoutMenuME2->addWidget(buttonExtractModsME2, 1);
    verticalLayoutMenuME2->addWidget(buttonConvertModME2, 1);
    verticalLayoutMenuME2->addWidget(buttonCreateModME2, 1);
    verticalLayoutMenuME2->addWidget(buttonCreateBinaryModME2, 1);

    verticalLayoutMenuME3->addSpacing(10);
    verticalLayoutMenuME3->addWidget(buttonModsManagerME3, 1);
    verticalLayoutMenuME3->addWidget(buttonInstallModsME3, 1);
    verticalLayoutMenuME3->addWidget(buttonExtractModsME3, 1);
    verticalLayoutMenuME3->addWidget(buttonConvertModME3, 1);
    verticalLayoutMenuME3->addWidget(buttonCreateModME3, 1);
    verticalLayoutMenuME3->addWidget(buttonCreateBinaryModME3, 1);


    auto GroupBoxViewME1 = new QGroupBox;
    GroupBoxViewME1->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    GroupBoxViewME1->setLayout(verticalLayoutMenuME1);
    auto sizePolicy = GroupBoxViewME1->sizePolicy();
    sizePolicy.setRetainSizeWhenHidden(true);
    GroupBoxViewME1->setSizePolicy(sizePolicy);

    auto GroupBoxViewME2 = new QGroupBox;
    GroupBoxViewME2->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    GroupBoxViewME2->setLayout(verticalLayoutMenuME2);
    sizePolicy.setRetainSizeWhenHidden(true);
    GroupBoxViewME2->setSizePolicy(sizePolicy);

    auto GroupBoxViewME3 = new QGroupBox;
    GroupBoxViewME3->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    GroupBoxViewME3->setLayout(verticalLayoutMenuME3);
    sizePolicy.setRetainSizeWhenHidden(true);
    GroupBoxViewME3->setSizePolicy(sizePolicy);

    auto verticalLayoutMainME1 = new QVBoxLayout();
    verticalLayoutMainME1->setAlignment(Qt::AlignCenter);
    verticalLayoutMainME1->addWidget(GroupBoxViewME1);

    auto verticalLayoutMainME2 = new QVBoxLayout();
    verticalLayoutMainME2->setAlignment(Qt::AlignCenter);
    verticalLayoutMainME2->addWidget(GroupBoxViewME2);

    auto verticalLayoutMainME3 = new QVBoxLayout();
    verticalLayoutMainME3->setAlignment(Qt::AlignCenter);
    verticalLayoutMainME3->addWidget(GroupBoxViewME3);

    auto verticalLayoutME1 = new QVBoxLayout();
    verticalLayoutME1->setAlignment(Qt::AlignCenter | Qt::AlignTop);
    verticalLayoutME1->addWidget(iconME1Logo, 1);
    verticalLayoutME1->addWidget(GroupBoxViewME1, 1);
    auto verticalLayoutME2 = new QVBoxLayout();
    verticalLayoutME2->setAlignment(Qt::AlignCenter | Qt::AlignTop);
    verticalLayoutME2->addWidget(iconME2Logo, 1);
    verticalLayoutME2->addWidget(GroupBoxViewME2, 1);
    auto verticalLayoutME3 = new QVBoxLayout();
    verticalLayoutME3->setAlignment(Qt::AlignCenter | Qt::AlignTop);
    verticalLayoutME3->addWidget(iconME3Logo, 1);
    verticalLayoutME3->addWidget(GroupBoxViewME3, 1);

    auto horizontalLayoutMain = new QHBoxLayout(this);
    horizontalLayoutMain->addLayout(verticalLayoutME1);
    horizontalLayoutMain->addLayout(verticalLayoutME2);
    horizontalLayoutMain->addLayout(verticalLayoutME3);

    HideAllSubMenusME1();
    HideAllSubMenusME2();
    HideAllSubMenusME3();

    mainWindow->SetTitle("");
}

void LayoutMain::HideAllSubMenusME1()
{
    buttonTextureUtilitiesME1->setChecked(false);
    buttonRemoveScanFileME1->hide();
    buttonApplyHQLODsME1->hide();
    buttonApply2kLODsME1->hide();
    buttonApplyVanillaLODsME1->hide();
    buttonApplyHQGfxME1->hide();
    buttonApplyHQGfxSoftShadows->hide();
    spacerBottomTextureUtilitiesME1->changeSize(0, 0);
    buttonGameUtilitiesME1->setChecked(false);
    buttonCheckGameFilesME1->hide();
    buttonChangeGamePathME1->hide();
    buttonChangeUserPathME1->hide();
    spacerBottomGameUtilitiesME1->changeSize(0, 0);
    buttonModsManagerME1->setChecked(false);
    buttonInstallModsME1->hide();
    buttonExtractModsME1->hide();
    buttonConvertModME1->hide();
    buttonCreateModME1->hide();
    buttonCreateBinaryModME1->hide();
}

void LayoutMain::HideAllSubMenusME2()
{
    buttonTextureUtilitiesME2->setChecked(false);
    buttonRemoveScanFileME2->hide();
    buttonApplyHQLODsME2->hide();
    buttonApplyVanillaLODsME2->hide();
    buttonApplyHQGfxME2->hide();
    spacerBottomTextureUtilitiesME2->changeSize(0, 0);
    buttonGameUtilitiesME2->setChecked(false);
    buttonCheckGameFilesME2->hide();
    buttonRepackGameFilesME2->hide();
    buttonChangeGamePathME2->hide();
#if !defined(_WIN32)
    buttonChangeUserPathME2->hide();
#endif
    spacerBottomGameUtilitiesME2->changeSize(0, 0);
    buttonModsManagerME2->setChecked(false);
    buttonInstallModsME2->hide();
    buttonExtractModsME2->hide();
    buttonConvertModME2->hide();
    buttonCreateModME2->hide();
    buttonCreateBinaryModME2->hide();
}

void LayoutMain::HideAllSubMenusME3()
{
    buttonTextureUtilitiesME3->setChecked(false);
    buttonRemoveScanFileME3->hide();
    buttonApplyHQLODsME3->hide();
    buttonApplyVanillaLODsME3->hide();
    buttonApplyHQGfxME3->hide();
    spacerBottomTextureUtilitiesME3->changeSize(0, 0);
    buttonGameUtilitiesME3->setChecked(false);
    buttonCheckGameFilesME3->hide();
    buttonRepackGameFilesME3->hide();
    buttonUpdateTOCs->hide();
    buttonExtractDLCs->hide();
    buttonChangeGamePathME3->hide();
#if !defined(_WIN32)
    buttonChangeUserPathME3->hide();
#endif
    spacerBottomGameUtilitiesME3->changeSize(0, 0);
    buttonModsManagerME3->setChecked(false);
    buttonInstallModsME3->hide();
    buttonExtractModsME3->hide();
    buttonConvertModME3->hide();
    buttonCreateModME3->hide();
    buttonCreateBinaryModME3->hide();
}

void LayoutMain::ButtonTextureUtilitiesME1Selected()
{
    bool checked = buttonTextureUtilitiesME1->isChecked();
    HideAllSubMenusME1();
    HideAllSubMenusME2();
    HideAllSubMenusME3();
    buttonTextureUtilitiesME1->setChecked(checked);
    buttonGameUtilitiesME1->setChecked(false);
    buttonModsManagerME1->setChecked(false);
    buttonRemoveScanFileME1->setHidden(!checked);
    buttonApplyHQLODsME1->setHidden(!checked);
    buttonApply2kLODsME1->setHidden(!checked);
    buttonApplyVanillaLODsME1->setHidden(!checked);
    buttonApplyHQGfxME1->setHidden(!checked);
    buttonApplyHQGfxSoftShadows->setHidden(!checked);
    spacerBottomTextureUtilitiesME1->changeSize(0, checked ? 10 : 0);
}

void LayoutMain::ButtonTextureUtilitiesME2Selected()
{
    bool checked = buttonTextureUtilitiesME2->isChecked();
    HideAllSubMenusME1();
    HideAllSubMenusME2();
    HideAllSubMenusME3();
    buttonTextureUtilitiesME2->setChecked(checked);
    buttonGameUtilitiesME2->setChecked(false);
    buttonModsManagerME2->setChecked(false);
    buttonRemoveScanFileME2->setHidden(!checked);
    buttonApplyHQLODsME2->setHidden(!checked);
    buttonApplyVanillaLODsME2->setHidden(!checked);
    buttonApplyHQGfxME2->setHidden(!checked);
    spacerBottomTextureUtilitiesME2->changeSize(0, checked ? 10 : 0);
}

void LayoutMain::ButtonTextureUtilitiesME3Selected()
{
    bool checked = buttonTextureUtilitiesME3->isChecked();
    HideAllSubMenusME1();
    HideAllSubMenusME2();
    HideAllSubMenusME3();
    buttonTextureUtilitiesME3->setChecked(checked);
    buttonGameUtilitiesME3->setChecked(false);
    buttonModsManagerME3->setChecked(false);
    buttonRemoveScanFileME3->setHidden(!checked);
    buttonApplyHQLODsME3->setHidden(!checked);
    buttonApplyVanillaLODsME3->setHidden(!checked);
    buttonApplyHQGfxME3->setHidden(!checked);
    spacerBottomTextureUtilitiesME3->changeSize(0, checked ? 10 : 0);
}

void LayoutMain::ButtonGameUtilitiesME1Selected()
{
    bool checked = buttonGameUtilitiesME1->isChecked();
    HideAllSubMenusME1();
    HideAllSubMenusME2();
    HideAllSubMenusME3();
    buttonTextureUtilitiesME1->setChecked(false);
    buttonGameUtilitiesME1->setChecked(checked);
    buttonModsManagerME1->setChecked(false);
    buttonCheckGameFilesME1->setHidden(!checked);
    buttonChangeGamePathME1->setHidden(!checked);
#if !defined(_WIN32)
    buttonChangeUserPathME1->setHidden(!checked);
#endif
    spacerBottomGameUtilitiesME1->changeSize(0, checked ? 10 : 0);
}

void LayoutMain::ButtonGameUtilitiesME2Selected()
{
    bool checked = buttonGameUtilitiesME2->isChecked();
    HideAllSubMenusME1();
    HideAllSubMenusME2();
    HideAllSubMenusME3();
    buttonTextureUtilitiesME2->setChecked(false);
    buttonGameUtilitiesME2->setChecked(checked);
    buttonModsManagerME2->setChecked(false);
    buttonCheckGameFilesME2->setHidden(!checked);
    buttonRepackGameFilesME2->setHidden(!checked);
    buttonChangeGamePathME2->setHidden(!checked);
#if !defined(_WIN32)
    buttonChangeUserPathME2->setHidden(!checked);
#endif
    spacerBottomGameUtilitiesME2->changeSize(0, checked ? 10 : 0);
}

void LayoutMain::ButtonGameUtilitiesME3Selected()
{
    bool checked = buttonGameUtilitiesME3->isChecked();
    HideAllSubMenusME1();
    HideAllSubMenusME2();
    HideAllSubMenusME3();
    buttonTextureUtilitiesME3->setChecked(false);
    buttonGameUtilitiesME3->setChecked(checked);
    buttonModsManagerME3->setChecked(false);
    buttonCheckGameFilesME3->setHidden(!checked);
    buttonRepackGameFilesME3->setHidden(!checked);
    buttonUpdateTOCs->setHidden(!checked);
    buttonExtractDLCs->setHidden(!checked);
    buttonChangeGamePathME3->setHidden(!checked);
#if !defined(_WIN32)
    buttonChangeUserPathME3->setHidden(!checked);
#endif
    spacerBottomGameUtilitiesME3->changeSize(0, checked ? 10 : 0);
}

void LayoutMain::ButtonModsManagerME1Selected()
{
    bool checked = buttonModsManagerME1->isChecked();
    HideAllSubMenusME1();
    HideAllSubMenusME2();
    HideAllSubMenusME3();
    buttonTextureUtilitiesME1->setChecked(false);
    buttonGameUtilitiesME1->setChecked(false);
    buttonModsManagerME1->setChecked(checked);
    buttonInstallModsME1->setHidden(!checked);
    buttonExtractModsME1->setHidden(!checked);
    buttonConvertModME1->setHidden(!checked);
    buttonCreateModME1->setHidden(!checked);
    buttonCreateBinaryModME1->setHidden(!checked);
}

void LayoutMain::ButtonModsManagerME2Selected()
{
    bool checked = buttonModsManagerME2->isChecked();
    HideAllSubMenusME1();
    HideAllSubMenusME2();
    HideAllSubMenusME3();
    buttonTextureUtilitiesME2->setChecked(false);
    buttonGameUtilitiesME2->setChecked(false);
    buttonModsManagerME2->setChecked(checked);
    buttonInstallModsME2->setHidden(!checked);
    buttonExtractModsME2->setHidden(!checked);
    buttonConvertModME2->setHidden(!checked);
    buttonCreateModME2->setHidden(!checked);
    buttonCreateBinaryModME2->setHidden(!checked);
}

void LayoutMain::ButtonModsManagerME3Selected()
{
    bool checked = buttonModsManagerME3->isChecked();
    HideAllSubMenusME1();
    HideAllSubMenusME2();
    HideAllSubMenusME3();
    buttonTextureUtilitiesME3->setChecked(false);
    buttonGameUtilitiesME3->setChecked(false);
    buttonModsManagerME3->setChecked(checked);
    buttonInstallModsME3->setHidden(!checked);
    buttonExtractModsME3->setHidden(!checked);
    buttonConvertModME3->setHidden(!checked);
    buttonCreateModME3->setHidden(!checked);
    buttonCreateBinaryModME3->setHidden(!checked);
}