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

#ifndef LAYOUT_MAIN_H
#define LAYOUT_MAIN_H

#include <Gui/MainWindow.h>

class LayoutMain: public LayoutHandle
{
    Q_OBJECT

public:
    explicit LayoutMain(MainWindow *window = nullptr);

private slots:

    void ButtonTextureUtilitiesME1Selected();
    void ButtonTextureUtilitiesME2Selected();
    void ButtonTextureUtilitiesME3Selected();
    void ButtonGameUtilitiesME1Selected();
    void ButtonGameUtilitiesME2Selected();
    void ButtonGameUtilitiesME3Selected();
    void ButtonModsManagerME1Selected();
    void ButtonModsManagerME2Selected();
    void ButtonModsManagerME3Selected();

private:
    const int kButtonMinWidth = 270;
    const int kButtonMinSmallWidth = 220;
    const int kButtonMinHeight = 22;
#if defined(__APPLE__)
    const int kFontSize = 12;
#elif defined(__linux__)
    const int kFontSize = 8;
#else
    const int kFontSize = 8;
#endif

    MainWindow    *mainWindow;

    QPushButton   *buttonTextureUtilitiesME1;
    QPushButton   *buttonRemoveScanFileME1;
    QPushButton   *buttonApplyHQLODsME1;
    QPushButton   *buttonApply2kLODsME1;
    QPushButton   *buttonApplyVanillaLODsME1;
    QPushButton   *buttonApplyHQGfxME1;
    QPushButton   *buttonApplyHQGfxSoftShadows;
    QSpacerItem   *spacerBottomTextureUtilitiesME1;
    QPushButton   *buttonGameUtilitiesME1;
    QPushButton   *buttonCheckGameFilesME1;
    QPushButton   *buttonChangeGamePathME1;
#if !defined(_WIN32)
    QPushButton   *buttonChangeUserPathME1;
#endif
    QSpacerItem   *spacerBottomGameUtilitiesME1;
    QPushButton   *buttonModsManagerME1;
    QPushButton   *buttonInstallModsME1;
    QPushButton   *buttonExtractModsME1;
    QPushButton   *buttonConvertModME1;
    QPushButton   *buttonCreateModME1;
    QPushButton   *buttonCreateBinaryModME1;

    QPushButton   *buttonTextureUtilitiesME2;
    QPushButton   *buttonRemoveScanFileME2;
    QPushButton   *buttonApplyHQLODsME2;
    QPushButton   *buttonApplyVanillaLODsME2;
    QPushButton   *buttonApplyHQGfxME2;
    QSpacerItem   *spacerBottomTextureUtilitiesME2;
    QPushButton   *buttonGameUtilitiesME2;
    QPushButton   *buttonCheckGameFilesME2;
    QPushButton   *buttonRepackGameFilesME2;
    QPushButton   *buttonChangeGamePathME2;
#if !defined(_WIN32)
    QPushButton   *buttonChangeUserPathME2;
#endif
    QSpacerItem   *spacerBottomGameUtilitiesME2;
    QPushButton   *buttonModsManagerME2;
    QPushButton   *buttonInstallModsME2;
    QPushButton   *buttonExtractModsME2;
    QPushButton   *buttonConvertModME2;
    QPushButton   *buttonCreateModME2;
    QPushButton   *buttonCreateBinaryModME2;

    QPushButton   *buttonTextureUtilitiesME3;
    QPushButton   *buttonRemoveScanFileME3;
    QPushButton   *buttonApplyHQLODsME3;
    QPushButton   *buttonApplyVanillaLODsME3;
    QPushButton   *buttonApplyHQGfxME3;
    QSpacerItem   *spacerBottomTextureUtilitiesME3;
    QPushButton   *buttonGameUtilitiesME3;
    QPushButton   *buttonCheckGameFilesME3;
    QPushButton   *buttonRepackGameFilesME3;
    QPushButton   *buttonUpdateTOCs;
    QPushButton   *buttonExtractDLCs;
    QPushButton   *buttonChangeGamePathME3;
#if !defined(_WIN32)
    QPushButton   *buttonChangeUserPathME3;
#endif
    QSpacerItem   *spacerBottomGameUtilitiesME3;
    QPushButton   *buttonModsManagerME3;
    QPushButton   *buttonInstallModsME3;
    QPushButton   *buttonExtractModsME3;
    QPushButton   *buttonConvertModME3;
    QPushButton   *buttonCreateModME3;
    QPushButton   *buttonCreateBinaryModME3;

    void HideAllSubMenusME1();
    void HideAllSubMenusME2();
    void HideAllSubMenusME3();
};

#endif // LAYOUT_ME_SELECT_H
