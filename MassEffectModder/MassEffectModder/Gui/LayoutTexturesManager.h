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

#ifndef LAYOUT_TEXTURES_MANAGER_H
#define LAYOUT_TEXTURES_MANAGER_H

#include <Gui/MainWindow.h>

class LayoutTexturesManager: public LayoutHandle
{
    Q_OBJECT

public:
    explicit LayoutTexturesManager(MainWindow *window = nullptr);

private slots:
    void ReplaceSelected();
    void ReplaceConvertSelected();
    void ExtractDDSSelected();
    void ExtractPNGSelected();
    void ViewImageSelected();
    void ViewSingleSelected();
    void ViewMultiSelected();
    void PackageSingleSelected();
    void PackageMutiSelected();
    void SearchSelected();
    void ExitSelected();

private:
    const int kButtonMinWidth = 200;
    const int kButtonMinHeight = 22;
#if defined(__APPLE__)
    const int kFontSize = 12;
#elif defined(__linux__)
    const int kFontSize = 13;
#else
    const int kFontSize = 15;
#endif
    const int kWidgetImage = 0;
    const int kWidgetText = 1;
    const int kWidgetList = 2;

    MainWindow     *mainWindow;
    QListWidget    *listLeft;
    QListWidget    *listMiddle;
    QListWidget    *listRight;
    QStackedWidget *rightView;
    QPlainTextEdit *textRight;
    QLabel         *labelImage;
    QSplitter      *splitter;
    QPushButton    *buttonReplace;
    QPushButton    *buttonReplaceConvert;
    QPushButton    *buttonExtractToDDS;
    QPushButton    *buttonExtractToPNG;
    QPushButton    *buttonViewImage;
    QPushButton    *buttonInfoSingle;
    QPushButton    *buttonInfoAll;
    QPushButton    *buttonPackageSingle;
    QPushButton    *buttonPackageMulti;
    QPushButton    *buttonSearch;
    QPushButton    *buttonExit;

    bool           singlePackageMode = false;
    bool           singleViewMode = true;
    bool           imageViewMode = true;
    bool           textureSelected = false;
    bool           packageSelected = false;

    void LockGui(bool enable);
    void UpdateGui();
};

#endif // LAYOUT_TEXTURES_MANAGER_H
