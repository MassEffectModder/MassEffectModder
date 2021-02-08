/*
 * MassEffectModder
 *
 * Copyright (C) 2019-2021 Pawel Kolodziejski
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

class LayoutMain;

class HoverLabel : public QLabel
{
    Q_OBJECT

private:
    bool       hover;
    int        idLabel;
    LayoutMain *layoutMain;

public:
    explicit HoverLabel(int id, LayoutMain *layout)
    {
        hover = false;
        idLabel = id;
        layoutMain = layout;
        setAttribute(Qt::WA_Hover, true);
    }

protected:
    void enterEvent(QEvent *ev) override;
    void leaveEvent(QEvent *ev) override;
};

class LayoutMain : public LayoutHandle
{
    Q_OBJECT

public:
    explicit LayoutMain(MainWindow *window = nullptr);

private slots:

    void ButtonTexturesMenagaerME1Selected() { ButtonTexturesMenagaerSelected(MeType::ME1_TYPE); }
    void ButtonTexturesMenagaerME2Selected() { ButtonTexturesMenagaerSelected(MeType::ME2_TYPE); }
    void ButtonTexturesMenagaerME3Selected() { ButtonTexturesMenagaerSelected(MeType::ME3_TYPE); }
    void ButtonTextureUtilitiesME1Selected();
    void ButtonTextureUtilitiesME2Selected();
    void ButtonTextureUtilitiesME3Selected();
    void ButtonGameUtilitiesME1Selected();
    void ButtonGameUtilitiesME2Selected();
    void ButtonGameUtilitiesME3Selected();
    void ButtonModsManagerME1Selected();
    void ButtonModsManagerME2Selected();
    void ButtonModsManagerME3Selected();

    void RemoveScanFileME1Selected() { RemoveScanFileSelected(MeType::ME1_TYPE); }
    void RemoveScanFileME2Selected() { RemoveScanFileSelected(MeType::ME2_TYPE); }
    void RemoveScanFileME3Selected() { RemoveScanFileSelected(MeType::ME3_TYPE); }
    void ApplyHQLODsME1Selected() { ApplyHQLODsSelected(MeType::ME1_TYPE); }
    void ApplyHQLODsME2Selected() { ApplyHQLODsSelected(MeType::ME2_TYPE); }
    void ApplyHQLODsME3Selected() { ApplyHQLODsSelected(MeType::ME3_TYPE); }
    void Apply2kLODsSelected();
    void ApplyVanillaLODsME1Selected() { ApplyVanillaLODsSelected(MeType::ME1_TYPE); }
    void ApplyVanillaLODsME2Selected() { ApplyVanillaLODsSelected(MeType::ME2_TYPE); }
    void ApplyVanillaLODsME3Selected() { ApplyVanillaLODsSelected(MeType::ME3_TYPE); }
    void ApplyHQGfxME1Selected() { ApplyHQGfxSelected(MeType::ME1_TYPE); }
    void ApplyHQGfxME2Selected() { ApplyHQGfxSelected(MeType::ME2_TYPE); }
    void ApplyHQGfxME3Selected() { ApplyHQGfxSelected(MeType::ME3_TYPE); }
    void ApplyHQGfxSoftShadowsSelected();

    void CheckGameFilesME1Selected() { CheckGameFilesSelected(MeType::ME1_TYPE); }
    void CheckGameFilesME2Selected() { CheckGameFilesSelected(MeType::ME2_TYPE); }
    void CheckGameFilesME3Selected() { CheckGameFilesSelected(MeType::ME3_TYPE); }
    void ChangeGamePathME1Selected() { ChangeGamePathSelected(MeType::ME1_TYPE); }
    void ChangeGamePathME2Selected() { ChangeGamePathSelected(MeType::ME2_TYPE); }
    void ChangeGamePathME3Selected() { ChangeGamePathSelected(MeType::ME3_TYPE); }
#if !defined(_WIN32)
    void ChangeUserPathME1Selected() { ChangeUserPathSelected(MeType::ME1_TYPE); }
    void ChangeUserPathME2Selected() { ChangeUserPathSelected(MeType::ME2_TYPE); }
    void ChangeUserPathME3Selected() { ChangeUserPathSelected(MeType::ME3_TYPE); }
#endif
    void RepackGameFilesME2Selected() { RepackGameFilesSelected(MeType::ME2_TYPE); }
    void RepackGameFilesME3Selected() { RepackGameFilesSelected(MeType::ME3_TYPE); }
    void UpdateTOCsSelected();
    void ExtractDLCsSelected();

    void InstallModsME1Selected() { ButtonModsInstallerSelected(MeType::ME1_TYPE); }
    void InstallModsME2Selected() { ButtonModsInstallerSelected(MeType::ME2_TYPE); }
    void InstallModsME3Selected() { ButtonModsInstallerSelected(MeType::ME3_TYPE); }
    void ExtractModsME1Selected() { ExtractModsSelected(MeType::ME1_TYPE); }
    void ExtractModsME2Selected() { ExtractModsSelected(MeType::ME2_TYPE); }
    void ExtractModsME3Selected() { ExtractModsSelected(MeType::ME3_TYPE); }
    void ConvertModME1Selected() { ConvertModSelected(MeType::ME1_TYPE); }
    void ConvertModME2Selected() { ConvertModSelected(MeType::ME2_TYPE); }
    void ConvertModME3Selected() { ConvertModSelected(MeType::ME3_TYPE); }
    void CreateModME1Selected() { CreateModSelected(MeType::ME1_TYPE); }
    void CreateModME2Selected() { CreateModSelected(MeType::ME2_TYPE); }
    void CreateModME3Selected() { CreateModSelected(MeType::ME3_TYPE); }
    void CreateBinaryModME1Selected() { CreateBinaryModSelected(MeType::ME1_TYPE); }
    void CreateBinaryModME2Selected() { CreateBinaryModSelected(MeType::ME2_TYPE); }
    void CreateBinaryModME3Selected() { CreateBinaryModSelected(MeType::ME3_TYPE); }

    friend class HoverLabel;

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

    int           currentMESelection;
    MainWindow    *mainWindow;

    HoverLabel    *labelME1;
    QLabel        *iconME1Logo;
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
    QGroupBox     *groupBoxViewME1;

    HoverLabel    *labelME2;
    QLabel        *iconME2Logo;
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
    QGroupBox     *groupBoxViewME2;

    HoverLabel    *labelME3;
    QLabel        *iconME3Logo;
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
    QGroupBox     *groupBoxViewME3;

    void OnStackChanged() override;

    void LockGui(bool lock);
    void DefaultMenuState();
    void HoverInME(int id);
    void HoverOutME(int id);
    void RemoveEffectsOnMenus();
    void HideAllSubMenusME1();
    void HideAllSubMenusME2();
    void HideAllSubMenusME3();

    void ButtonTexturesMenagaerSelected(MeType gameType);
    void ButtonModsInstallerSelected(MeType gameType);

    void RemoveScanFileSelected(MeType gameType);
    void ApplyHQLODsSelected(MeType gameType);
    void ApplyVanillaLODsSelected(MeType gameType);
    void ApplyHQGfxSelected(MeType gameType);
    void ApplyLODs(MeType gameType, bool lods2k);
    void ApplyHQGfx(MeType gameType, bool softShadows);

    void CheckGameFilesSelected(MeType gameType);
    void ChangeGamePathSelected(MeType gameType);
#if !defined(_WIN32)
    void ChangeUserPathSelected(MeType gameType);
#endif
    void RepackGameFilesSelected(MeType gameType);
    static void CheckCallback(void *handle, int progress, const QString &stage);
    static void ExtractDlcCallback(void *handle, int progress, const QString &stage);
    static void RepackCallback(void *handle, int progress, const QString &stage);

    void ExtractModsSelected(MeType gameType);
    void ConvertModSelected(MeType gameType);
    void CreateModSelected(MeType gameType);
    void CreateBinaryModSelected(MeType gameType);
    static void ExtractModCallback(void *handle, int progress, const QString &stage);
    static void ConvertModCallback(void *handle, int progress, const QString &stage);
    static void CreateModCallback(void *handle, int progress, const QString &stage);
};

#endif // LAYOUT_MAIN_H
