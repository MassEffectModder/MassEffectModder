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
#include <Image/Image.h>
#include <Program/ConfigIni.h>
#include <Resources/Resources.h>
#include <Texture/TextureScan.h>

class PixmapLabel : public QLabel
{
    Q_OBJECT

private:
    QPixmap pixmapImage;

public slots:
    void setPixmap(const QPixmap &p);

protected:
    void resizeEvent(QResizeEvent *event) override;

public:
    explicit PixmapLabel(QWidget *parent = nullptr);

    [[nodiscard]] QPixmap resizePixmap() const;
    [[nodiscard]] int heightForWidth(int w) const override;
};

struct ViewPackage {
    QString packageName;
    int     indexInTextures;
    int     indexInPackages;
};

struct ViewTexture {
    QString name;
    int     indexInTextures;
    int     indexInPackages;
};
Q_DECLARE_METATYPE(ViewTexture)

class LayoutTexturesManager: public LayoutHandle
{
    Q_OBJECT

public:
    explicit LayoutTexturesManager(MainWindow *window, MeType type);
    void Startup();

private slots:
    void ListLeftPackagesSelected(QListWidgetItem *current, QListWidgetItem *previous);
    void ListMiddleTextureSelected(QListWidgetItem *current, QListWidgetItem *previous);
    void ListMiddleContextMenu(const QPoint &pos);
    void ListRightSelected(QListWidgetItem *current, QListWidgetItem *previous);
    void ListRightContextMenu(const QPoint &pos);
    void ReplaceSelected();
    void ReplaceConvertSelected();
    void ExtractDDSSelected();
    void ExtractPNGSelected();
    void ViewImageSelected();
    void ViewImageAlphaSelected();
    void InfoSingleSelected();
    void InfoAllSelected();
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
    PixmapLabel    *labelImage;
    QSplitter      *splitter;
    QPushButton    *buttonPackageSingle;
    QPushButton    *buttonPackageMulti;
    QPushButton    *buttonSearch;
    QPushButton    *buttonExit;

    bool           singlePackageMode{};
    bool           singleInfoMode{};
    bool           imageViewMode{};
    bool           imageViewAlphaMode{};
    bool           textureSelected{};
    bool           packageSelected{};
    bool           textureInstanceSelected{};

    ConfigIni      configIni{};
    QList<TextureMapEntry> textures;
    Resources      resources;
    MeType         gameType;

    static void PrepareTexturesCallback(void *handle, int progress, const QString &stage);
    static void ReplaceTextureCallback(void *handle, int progress, const QString &stage);
    void LockGui(bool lock);
    void UpdateGui();
    void ExtractTexture(const ViewTexture &viewTexture, bool png);
    void ReplaceTexture(const QListWidgetItem *item, bool convertMode);
    void SearchTexture(const QString &name, uint crc);
    void selectFoundTexture(const QListWidgetItem *item);
    void UpdateRight(const QListWidgetItem *item);
    void SearchListSelected(QListWidgetItem *item);
    void UpdateRightList(const QListWidgetItem *item);
};

#endif // LAYOUT_TEXTURES_MANAGER_H
