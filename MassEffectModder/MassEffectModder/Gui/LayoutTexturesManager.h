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
#include <Program/ConfigIni.h>
#include <Resources/Resources.h>
#include <Texture/TextureScan.h>

struct ViewTexture {
    QString name;
    uint    crc;
    QString packageName;
};
Q_DECLARE_METATYPE(ViewTexture)

class LayoutTexturesManager: public LayoutHandle
{
    Q_OBJECT

public:
    explicit LayoutTexturesManager(MainWindow *window = nullptr);
    void Startup();

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
    const int kFontSize = 8;
#else
    const int kFontSize = 8;
#endif
    const int kRightWidgetImage = 0;
    const int kRightWidgetText = 1;
    const int kRightWidgetList = 2;
    const int kLeftWidgetPackages = 0;
    const int kLeftWidgetSearch = 1;

    struct TreeItem {
        QString            packageName;
        QList<ViewTexture> list;
    };

    MainWindow     *mainWindow;
    QListWidget    *listLeftPackages;
    QListWidget    *listLeftSearch;
    QListWidget    *listMiddle;
    QListWidget    *listRight;
    QStackedWidget *leftWidget;
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

    bool           singlePackageMode{};
    bool           singleViewMode{};
    bool           imageViewMode{};
    bool           textureSelected{};
    bool           packageSelected{};

    ConfigIni      configIni{};
    QList<TextureMapEntry> textures;
    Resources      resources;

    static void PrepareTexturesCallback(void *handle, int progress, const QString &stage);
    void LockGui(bool lock);
    void UpdateGui();
};

#endif // LAYOUT_TEXTURES_MANAGER_H
