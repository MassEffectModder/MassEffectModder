/*
 * MassEffectModder
 *
 * Copyright (C) 2019-2022 Pawel Kolodziejski
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

#include <GameData/GameData.h>
#include <GameData/TOCFile.h>
#include <Gui/LayoutMain.h>
#include <Gui/LayoutTexturesManager.h>
#include <Gui/MainWindow.h>
#include <Gui/MessageWindow.h>
#include <Gui/PixmapLabel.h>
#include <Image/Image.h>
#include <Helpers/Exception.h>
#include <Helpers/QSort.h>
#include <Helpers/Logs.h>
#include <Helpers/MiscHelpers.h>
#include <MipMaps/MipMaps.h>
#include <Misc/Misc.h>
#include <Texture/Texture.h>
#include <Texture/TextureMovie.h>

LayoutTexturesManager::LayoutTexturesManager(MainWindow *window, MeType type)
    : mainWindow(window), gameType(type)
{
    layoutId = MainWindow::kLayoutTexturesManager;

    listLeftPackages = new QListWidget();
    connect(listLeftPackages, &QListWidget::currentItemChanged, this, &LayoutTexturesManager::ListLeftPackagesSelected);

    listLeftSearch = new QListWidget();
    connect(listLeftSearch, &QListWidget::itemDoubleClicked, this, &LayoutTexturesManager::SearchListSelected);

    leftWidget = new QStackedWidget();
    leftWidget->addWidget(listLeftPackages);
    leftWidget->addWidget(listLeftSearch);
    leftWidget->setCurrentIndex(kLeftWidgetPackages);
    listMiddle = new QListWidget();
    listMiddle->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(listMiddle, SIGNAL(customContextMenuRequested(QPoint)), SLOT(ListMiddleContextMenu(QPoint)));
    connect(listMiddle, &QListWidget::currentItemChanged, this, &LayoutTexturesManager::ListMiddleTextureSelected);

    labelImage = new PixmapLabel();
    textRight = new QPlainTextEdit;
    textRight->setReadOnly(true);
#if defined(__APPLE__)
    QFont font("Monaco");
    font.setPixelSize(10);
#elif defined(__linux__)
    QFont font("Monospace");
    font.setPixelSize(10);
#else
    QFont font("Monospace");
    font.setPixelSize(11);
#endif
    font.setFixedPitch(true);
    textRight->setFont(font);
    listRight = new QListWidget;
    listRight->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(listRight, SIGNAL(customContextMenuRequested(QPoint)), SLOT(ListRightContextMenu(QPoint)));
    connect(listRight, &QListWidget::currentItemChanged, this, &LayoutTexturesManager::ListRightSelected);
    rightView = new QStackedWidget();
    rightView->addWidget(labelImage);
    rightView->addWidget(textRight);
    rightView->addWidget(listRight);
    rightView->setCurrentIndex(kRightWidgetList);

    splitter = new QSplitter();
    splitter->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    splitter->addWidget(leftWidget);
    splitter->addWidget(listMiddle);
    splitter->addWidget(rightView);

    buttonPackageSingle = new QPushButton("Single Package");
    buttonPackageSingle->setMinimumWidth(kButtonMinWidth);
    buttonPackageSingle->setMinimumHeight(kButtonMinHeight);
    auto ButtonFont = buttonPackageSingle->font();
    ButtonFont.setPointSize(kFontSize);
    buttonPackageSingle->setFont(ButtonFont);
    connect(buttonPackageSingle, &QPushButton::clicked, this, &LayoutTexturesManager::PackageSingleSelected);

    buttonPackageMulti = new QPushButton("Multi Package");
    buttonPackageMulti->setMinimumWidth(kButtonMinWidth);
    buttonPackageMulti->setMinimumHeight(kButtonMinHeight);
    ButtonFont = buttonPackageMulti->font();
    ButtonFont.setPointSize(kFontSize);
    buttonPackageMulti->setFont(ButtonFont);
    connect(buttonPackageMulti, &QPushButton::clicked, this, &LayoutTexturesManager::PackageMutiSelected);

    auto VerticalLayoutListView = new QVBoxLayout();
    VerticalLayoutListView->addWidget(buttonPackageSingle);
    VerticalLayoutListView->addWidget(buttonPackageMulti);
    VerticalLayoutListView->setAlignment(Qt::AlignTop);
    auto GroupBoxView = new QGroupBox("Package Mode");
    GroupBoxView->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    GroupBoxView->setAlignment(Qt::AlignBottom);
    GroupBoxView->setLayout(VerticalLayoutListView);

    buttonSearch = new QPushButton("Search for a texture");
    buttonSearch->setMinimumWidth(kButtonMinWidth);
    buttonSearch->setMinimumHeight(kButtonMinHeight);
    buttonSearch->setShortcut(QKeySequence("Ctrl+F"));
    ButtonFont = buttonSearch->font();
    ButtonFont.setPointSize(kFontSize);
    buttonSearch->setFont(ButtonFont);
    connect(buttonSearch, &QPushButton::clicked, this, &LayoutTexturesManager::SearchSelected);

    buttonExit = new QPushButton("Exit Texture Manager");
    buttonExit->setMinimumWidth(kButtonMinWidth);
    buttonExit->setMinimumHeight(kButtonMinHeight);
    ButtonFont = buttonExit->font();
    ButtonFont.setPointSize(kFontSize);
    buttonExit->setFont(ButtonFont);
    connect(buttonExit, &QPushButton::clicked, this, &LayoutTexturesManager::ExitSelected);

    auto VerticalLayoutListMisc = new QVBoxLayout();
    VerticalLayoutListMisc->addWidget(buttonSearch);
    VerticalLayoutListMisc->addWidget(buttonExit);
    VerticalLayoutListMisc->setAlignment(Qt::AlignTop);

    auto GroupBoxMisc = new QGroupBox("Miscellaneous");
    GroupBoxMisc->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    GroupBoxMisc->setAlignment(Qt::AlignBottom);
    GroupBoxMisc->setLayout(VerticalLayoutListMisc);

    auto HorizontalLayoutList = new QHBoxLayout();
    HorizontalLayoutList->setAlignment(Qt::AlignLeft);
    HorizontalLayoutList->addWidget(GroupBoxView, 1);
    HorizontalLayoutList->addWidget(GroupBoxMisc, 1);
    auto WidgetBottom = new QWidget();
    WidgetBottom->setLayout(HorizontalLayoutList);

    auto VerticalLayout = new QVBoxLayout(this);
    VerticalLayout->addWidget(splitter);
    VerticalLayout->addWidget(WidgetBottom);

    auto shortcut = new QShortcut(QKeySequence("Ctrl+R"), this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(ReplaceSelected()));

    shortcut = new QShortcut(QKeySequence("Ctrl+E"), this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(ExtractDDSSelected()));

    shortcut = new QShortcut(QKeySequence("Ctrl+T"), this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(ExtractPNGSelected()));

    bool french = QApplication::inputMethod()->locale().language() == QLocale::French;
    if (french)
        shortcut = new QShortcut(QKeySequence("1"), this);
    else
        shortcut = new QShortcut(QKeySequence("Ctrl+1"), this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(ViewImageSelected()));

    if (french)
        shortcut = new QShortcut(QKeySequence("2"), this);
    else
        shortcut = new QShortcut(QKeySequence("Ctrl+2"), this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(ViewImageAlphaSelected()));

    if (french)
        shortcut = new QShortcut(QKeySequence("3"), this);
    else
        shortcut = new QShortcut(QKeySequence("Ctrl+3"), this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(InfoSingleSelected()));

    if (french)
        shortcut = new QShortcut(QKeySequence("4"), this);
    else
        shortcut = new QShortcut(QKeySequence("Ctrl+4"), this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(InfoAllSelected()));

    mainWindow->SetTitle(gameType, "Texture Manager");
}

static int compareTextures(const ViewPackage &e1, const ViewPackage &e2)
{
    int compResult = AsciiStringCompareCaseIgnore(e1.packageName, e2.packageName);
    if (compResult < 0)
        return -1;
    if (compResult > 0)
        return 1;
    if (e1.indexInTextures < e2.indexInTextures)
        return -1;
    if (e1.indexInTextures > e2.indexInTextures)
        return 1;
    if (e1.indexInPackages < e2.indexInPackages)
        return -1;
    if (e1.indexInPackages > e2.indexInPackages)
        return 1;
    return 0;
}

bool LayoutTexturesManager::Startup()
{
    LockGui(true);

    mainWindow->statusBar()->showMessage("Detecting game data...");
    QApplication::processEvents();
    g_GameData->Init(gameType, configIni, true);
    if (!Misc::CheckGamePath())
    {
        QMessageBox::critical(this, "Texture Manager", STR_GAME_DATA_NOT_FOUND);
        mainWindow->statusBar()->clearMessage();
        buttonExit->setEnabled(true);
        mainWindow->LockClose(false);
        return false;
    }

    QString path = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation).first() +
            "/MassEffectModder";
    QString filename = path + QString("/mele%1map.bin").arg(static_cast<int>(gameType));
    if (QFile::exists(filename))
    {
        FileStream fs = FileStream(filename, FileMode::Open, FileAccess::ReadOnly);
        uint tag = fs.ReadUInt32();
        uint version = fs.ReadUInt32();
        if (tag != textureMapBinTag || version != textureMapBinVersion)
        {
            QMessageBox::critical(this, "Texture Manager",
                                  QString("Detected a corrupt or old version of texture scan file!") +
                "\n\nYou need to restore the game to a vanilla state then reinstall non-texture mods." +
                "\n\nThen, from 'Texture Utilities', select 'Delete Texture Scan File' and start Texture Manager again.");
            mainWindow->statusBar()->clearMessage();
            buttonExit->setEnabled(true);
            mainWindow->LockClose(false);
            return false;
        }

        uint countTexture = fs.ReadUInt32();
        for (uint i = 0; i < countTexture; i++)
        {
            fs.Skip(fs.ReadInt32());
            fs.SkipInt32();
            uint countPackages = fs.ReadUInt32();
            for (uint k = 0; k < countPackages; k++)
            {
                fs.Skip(8);
                fs.Skip(fs.ReadInt32());
            }
        }

        QStringList packages = QStringList();
        int numPackages = fs.ReadInt32();
        for (int i = 0; i < numPackages; i++)
        {
            QString pkgPath;
            fs.ReadStringASCII(pkgPath, fs.ReadInt32());
            pkgPath.replace(QChar('\\'), QChar('/'));
            packages.push_back(pkgPath);
        }
        for (int i = 0; i < packages.count(); i++)
        {
            bool found = false;
            for (int f = 0; f < g_GameData->packageFiles.count(); f++)
            {
                if (AsciiStringMatchCaseIgnore(g_GameData->packageFiles[f], packages[i]))
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                QMessageBox::critical(this, "Texture Manager",
                                      QString("Detected removal of game files since the texture map was created. This is not supported.") +
                      "\n\nYou need to restore the game to a vanilla state then reinstall non-texture mods." +
                      "\n\nThen, from the 'Texture Utilities', select 'Delete Texture Scan File' and start Texture Manager again.");
                mainWindow->statusBar()->clearMessage();
                buttonExit->setEnabled(true);
                mainWindow->LockClose(false);
                return false;
            }
        }
        for (int i = 0; i < g_GameData->packageFiles.count(); i++)
        {
            bool found = false;
            for (int f = 0; f < packages.count(); f++)
            {
                if (AsciiStringMatchCaseIgnore(packages[f], g_GameData->packageFiles[i]))
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                QMessageBox::critical(this, "Texture Manager",
                                      QString("Detected new or modified game files that have changed since the texture scan was created. This is not supported.") +
                      "\n\nYou need to restore the game to a vanilla state then reinstall non-texture mods." +
                      "\n\nThen, from the 'Texture Utilities', select 'Delete Texture Scan File' and start Texture Manager again.");
                mainWindow->statusBar()->clearMessage();
                buttonExit->setEnabled(true);
                mainWindow->LockClose(false);
                return false;
            }
        }
        if (!TreeScan::loadTexturesMapFile(filename, textures, true))
        {
            QMessageBox::critical(this, "Texture Manager", "Failed to load texture map.");
            buttonExit->setEnabled(true);
            mainWindow->LockClose(false);
            return false;
        }
    }
    else
    {
        bool modded = Misc::detectMod();
        if (!modded)
            modded = Misc::MarkersPresent(&LayoutTexturesManager::PrepareTexturesCallback, mainWindow);
        mainWindow->statusBar()->clearMessage();
        if (modded)
        {
            QMessageBox::critical(this, "Texture Manager",
                                  QString("Detected game as already texture modded, but the game does not have a texture scan. The game is in a unsupported configuration.") +
                  "\n\nYou need to restore the game to a vanilla state then reinstall non-texture mods." +
                  "\n\nThen start Texture Manager again.");
            mainWindow->statusBar()->clearMessage();
            buttonExit->setEnabled(true);
            mainWindow->LockClose(false);
            return false;
        }
    }

    mainWindow->statusBar()->showMessage("Checking for incompatible mods...");
    QApplication::processEvents();
    QStringList badMods;
    Misc::detectBrokenMod(badMods);
    if (badMods.count() != 0)
    {
        QString list;
        for (int l = 0; l < badMods.count(); l++)
        {
            list += badMods[l] + "\n";
        }
        QMessageBox::critical(this, "Texture Manager",
                              QString("Detected incompatible mods: \n\n") + list);
        mainWindow->statusBar()->clearMessage();
        buttonExit->setEnabled(true);
        mainWindow->LockClose(false);
        return false;
    }

    mainWindow->statusBar()->clearMessage();
    QApplication::processEvents();
    if (!QFile::exists(filename))
    {
        int result = QMessageBox::question(this, "Texture Manager",
                              QString("Replacing textures and creating mods requires generating a map of the game's textures.\n") +
                              "This scan only needs to be done once.\n\n" + // this 'scan' mention is here as it is mentioned in other parts of the UI
                              "IMPORTANT! Your game needs to be in a non-texture modded state, and all non-texture mods must be installed at ths point, they cannot be installed later.\n\n" +
                              "You can continue to make the map, or abort.", "Continue", "Abort");
        if (result != 0)
        {
            buttonExit->setEnabled(true);
            mainWindow->LockClose(false);
            return false;
        }
    }

    if (!Misc::checkWriteAccessDir(g_GameData->MainData()))
    {
        QMessageBox::critical(this, "Texture Manager",
                              QString("The current user does not have write access to game folder.") +
                              "\nGrant write access to the game directory and then open Texture Manager again.");

        buttonExit->setEnabled(true);
        mainWindow->LockClose(false);
        return false;
    }

    if (!QFile::exists(filename))
    {
        mainWindow->statusBar()->showMessage("Preparing to scan textures...");
        QApplication::processEvents();
        resources.loadMD5Tables();
        g_logs->BufferClearErrors();
        g_logs->BufferEnableErrors(true);
        TreeScan::PrepareListOfTextures(gameType, resources,
                                        textures, true,
                                        &LayoutTexturesManager::PrepareTexturesCallback,
                                        mainWindow);
        g_logs->BufferEnableErrors(false);
        mainWindow->statusBar()->clearMessage();
        QApplication::processEvents();
        if (g_logs->BufferGetErrors() != "")
        {
            MessageWindow msg;
            msg.Show(mainWindow, "Errors while scanning package files", g_logs->BufferGetErrors());
            return false;
        }
    }

    QElapsedTimer timer;
    timer.start();
    mainWindow->statusBar()->showMessage("Preparing tree view...");
    QApplication::processEvents();
    QVector<ViewPackage> ViewPackageList;
    for (int t = 0; t < textures.count(); t++)
    {
        if (timer.elapsed() > 100)
        {
            QApplication::processEvents();
            timer.restart();
        }
        for (int l = 0; l < textures[t].list.count(); l++)
        {
            if (textures[t].list[l].path.length() == 0)
                continue;
            ViewPackage texture;
            texture.packageName = BaseNameWithoutExt(textures[t].list[l].path);
            texture.indexInTextures = t;
            texture.indexInPackages = l;
            ViewPackageList.append(texture);
        }
    }
    QSort(ViewPackageList, 0, ViewPackageList.count() - 1, compareTextures);

    QString lastPackageName;
    int lastIndexInTextures = -1;
    int index = -1;
    listLeftPackages->setUpdatesEnabled(false);
    for (int l = 0; l < ViewPackageList.count(); l++)
    {
        if (timer.elapsed() > 100)
        {
            QApplication::processEvents();
            timer.restart();
        }
        ViewTexture texture;
        texture.name = textures[ViewPackageList[l].indexInTextures].name;
        texture.indexInTextures = ViewPackageList[l].indexInTextures;
        texture.indexInPackages = ViewPackageList[l].indexInPackages;
        if (!AsciiStringMatch(ViewPackageList[l].packageName, lastPackageName))
        {
            QVector<ViewTexture> list;
            list.append(texture);
            auto item = new QListWidgetItem(ViewPackageList[l].packageName);
            item->setData(Qt::UserRole, QVariant::fromValue<QVector<ViewTexture>>(list));
            listLeftPackages->addItem(item);
            lastPackageName = ViewPackageList[l].packageName;
            lastIndexInTextures = ViewPackageList[l].indexInTextures;
            index++;
        }
        else
        {
            if (ViewPackageList[l].indexInTextures == lastIndexInTextures)
                continue;
            lastIndexInTextures = ViewPackageList[l].indexInTextures;
            auto item = listLeftPackages->item(index);
            auto list = item->data(Qt::UserRole).value<QVector<ViewTexture>>();
            list.append(texture);
            item->setData(Qt::UserRole, QVariant::fromValue<QVector<ViewTexture>>(list));
        }
    }
    listLeftPackages->setUpdatesEnabled(true);
    mainWindow->statusBar()->clearMessage();

    singlePackageMode = false;
    singleInfoMode = true;
    imageViewMode = true;
    imageViewAlphaMode = false;
    textureSelected = false;
    packageSelected = false;
    textureInstanceSelected = false;

    LockGui(false);
    UpdateGui();

    return true;
}

void LayoutTexturesManager::ListMiddleContextMenu(const QPoint &pos)
{
    bool enableInfoSingle;
    if (imageViewMode || imageViewAlphaMode)
        enableInfoSingle = !singlePackageMode;
    else
        enableInfoSingle = !singleInfoMode && !singlePackageMode;
    bool enableInfoAll;
    if (imageViewMode || imageViewAlphaMode)
        enableInfoAll = !singlePackageMode;
    else
        enableInfoAll = singleInfoMode && !singlePackageMode;

    auto item = listMiddle->itemAt(pos);
    if (!item)
        return;
    auto viewTexture = item->data(Qt::UserRole).value<ViewTexture>();
    TextureMapPackageEntry nodeTexture;
    for (int index = 0; index < textures[viewTexture.indexInTextures].list.count(); index++)
    {
        if (textures[viewTexture.indexInTextures].list[index].path.length() != 0)
        {
            nodeTexture = textures[viewTexture.indexInTextures].list[index];
            break;
        }
    }

    if (!singlePackageMode)
    {
        auto menu = new QMenu(this);
        auto subMenu = new QAction("Replace Texture", this);
        connect(subMenu, &QAction::triggered, this, &LayoutTexturesManager::ReplaceSelected);
        menu->addAction(subMenu);
        if (!nodeTexture.movieTexture)
        {
            subMenu = new QAction("Replace Texture (upgrade)", this);
            connect(subMenu, &QAction::triggered, this, &LayoutTexturesManager::ReplaceConvertSelected);
            menu->addAction(subMenu);
            subMenu = new QAction("Extract to DDS", this);
            connect(subMenu, &QAction::triggered, this, &LayoutTexturesManager::ExtractDDSSelected);
            menu->addAction(subMenu);
            subMenu = new QAction("Extract to PNG", this);
            connect(subMenu, &QAction::triggered, this, &LayoutTexturesManager::ExtractPNGSelected);
            menu->addAction(subMenu);
        }
        else
        {
            subMenu = new QAction("Extract to BIK", this);
            connect(subMenu, &QAction::triggered, this, &LayoutTexturesManager::ExtractBIKSelected);
            menu->addAction(subMenu);
        }
        QMenu *menuViewMode = menu->addMenu("View");
        subMenu = new QAction("Preview", this);
        subMenu->setCheckable(true);
        subMenu->setChecked(imageViewMode);
        subMenu->setEnabled(!imageViewMode);
        connect(subMenu, &QAction::triggered, this, &LayoutTexturesManager::ViewImageSelected);
        menuViewMode->addAction(subMenu);
        if (!nodeTexture.movieTexture)
        {
            subMenu = new QAction("Preview (alpha only)", this);
            subMenu->setCheckable(true);
            subMenu->setChecked(imageViewAlphaMode);
            subMenu->setEnabled(!imageViewAlphaMode);
            connect(subMenu, &QAction::triggered, this, &LayoutTexturesManager::ViewImageAlphaSelected);
            menuViewMode->addAction(subMenu);
        }
        subMenu = new QAction("Info (single instance)", this);
        subMenu->setCheckable(true);
        subMenu->setChecked(!imageViewMode && !imageViewAlphaMode && singleInfoMode);
        subMenu->setEnabled(enableInfoSingle);
        connect(subMenu, &QAction::triggered, this, &LayoutTexturesManager::InfoSingleSelected);
        menuViewMode->addAction(subMenu);
        subMenu = new QAction("Info (all instances)", this);
        subMenu->setCheckable(true);
        subMenu->setChecked(!imageViewMode && !imageViewAlphaMode && !singleInfoMode);
        subMenu->setEnabled(enableInfoAll);
        connect(subMenu, &QAction::triggered, this, &LayoutTexturesManager::InfoAllSelected);
        menuViewMode->addAction(subMenu);
        menu->popup(listMiddle->viewport()->mapToGlobal(pos));
    }
}

void LayoutTexturesManager::ListRightContextMenu(const QPoint &pos)
{
    auto item = listRight->itemAt(pos);
    auto viewTexture = item->data(Qt::UserRole).value<ViewTexture>();
    TextureMapPackageEntry nodeTexture;
    for (int index = 0; index < textures[viewTexture.indexInTextures].list.count(); index++)
    {
        if (textures[viewTexture.indexInTextures].list[index].path.length() != 0)
        {
            nodeTexture = textures[viewTexture.indexInTextures].list[index];
            break;
        }
    }

    if (item && singlePackageMode)
    {
        auto menu = new QMenu(this);
        auto subMenu = new QAction("Replace Texture", this);
        connect(subMenu, &QAction::triggered, this, &LayoutTexturesManager::ReplaceSelected);
        menu->addAction(subMenu);
        if (!nodeTexture.movieTexture)
        {
            subMenu = new QAction("Replace Texture (uncompressed)", this);
            connect(subMenu, &QAction::triggered, this, &LayoutTexturesManager::ReplaceConvertSelected);
            menu->addAction(subMenu);
            subMenu = new QAction("Extract to DDS", this);
            connect(subMenu, &QAction::triggered, this, &LayoutTexturesManager::ExtractDDSSelected);
            menu->addAction(subMenu);
            subMenu = new QAction("Extract to PNG", this);
            connect(subMenu, &QAction::triggered, this, &LayoutTexturesManager::ExtractPNGSelected);
            menu->addAction(subMenu);
        }
        else
        {
            subMenu = new QAction("Extract to BIK", this);
            connect(subMenu, &QAction::triggered, this, &LayoutTexturesManager::ExtractBIKSelected);
            menu->addAction(subMenu);
        }
        menu->popup(listRight->viewport()->mapToGlobal(pos));
    }
}

void LayoutTexturesManager::PrepareTexturesCallback(void *handle, int progress, const QString &stage)
{
    auto *win = static_cast<MainWindow *>(handle);
    win->statusBar()->showMessage(QString("Preparing... Stage: ") + stage +
                                  " Progress: " + QString::number(progress) + "%");
    QApplication::processEvents();
}

void LayoutTexturesManager::ReplaceTextureCallback(void *handle, int progress, const QString & /*stage*/)
{
    auto *win = static_cast<MainWindow *>(handle);
    win->statusBar()->showMessage(QString("Replacing texture...") +
                                  " Progress: " + QString::number(progress) + "%");
    QApplication::processEvents();
}

void LayoutTexturesManager::LockGui(bool lock)
{
    listLeftPackages->setEnabled(!lock);
    listLeftSearch->setEnabled(!lock);
    listMiddle->setEnabled(!lock);
    listRight->setEnabled(!lock);
    textRight->setEnabled(!lock);
    rightView->setEnabled(!lock);
    splitter->setEnabled(!lock);
    buttonPackageSingle->setEnabled(!lock);
    buttonPackageMulti->setEnabled(!lock);
    buttonSearch->setEnabled(!lock);
    buttonExit->setEnabled(!lock);
    mainWindow->LockClose(lock);
    if (!lock)
        UpdateGui();
}

void LayoutTexturesManager::UpdateGui()
{
    if (imageViewMode || imageViewAlphaMode)
    {
        if (singlePackageMode)
            rightView->setCurrentIndex(kRightWidgetList);
        else
            rightView->setCurrentIndex(kRightWidgetImage);
        buttonPackageSingle->setEnabled(textureSelected && !singlePackageMode);
        buttonPackageMulti->setEnabled(textureSelected && singlePackageMode);
    }
    else
    {
        if (singlePackageMode)
            rightView->setCurrentIndex(kRightWidgetList);
        else
            rightView->setCurrentIndex(kRightWidgetText);
        buttonPackageSingle->setEnabled(textureSelected && !singlePackageMode);
        buttonPackageMulti->setEnabled(textureSelected & singlePackageMode);
    }
}

void LayoutTexturesManager::ListLeftPackagesSelected(QListWidgetItem *current,
                                                     QListWidgetItem * /*previous*/)
{
    listMiddle->clear();
    listMiddle->setUpdatesEnabled(false);
    listRight->clear();
    labelImage->clear();
    if (current == nullptr)
    {
        packageSelected = false;
    }
    else
    {
        packageSelected = true;
        auto list = current->data(Qt::UserRole).value<QVector<ViewTexture>>();
        for (int i = 0; i < list.count(); i++)
        {
            auto textureItem = new QListWidgetItem(list[i].name);
            textureItem->setData(Qt::UserRole, QVariant::fromValue<ViewTexture>(list[i]));
            listMiddle->addItem(textureItem);
        }
        listMiddle->sortItems();
    }
    listMiddle->setUpdatesEnabled(true);
    UpdateGui();
}

void LayoutTexturesManager::UpdateRight(const QListWidgetItem *item)
{
    auto viewTexture = item->data(Qt::UserRole).value<ViewTexture>();
    if (imageViewMode || imageViewAlphaMode)
    {
        TextureMapPackageEntry nodeTexture;
        for (int index = 0; index < textures[viewTexture.indexInTextures].list.count(); index++)
        {
            if (textures[viewTexture.indexInTextures].list[index].path.length() != 0)
            {
                nodeTexture = textures[viewTexture.indexInTextures].list[index];
                break;
            }
        }

        if (nodeTexture.movieTexture)
        {
            labelImage->clear();
            return;
        }

        Package package;
        package.Open(g_GameData->GamePath() + nodeTexture.path);
        ByteBuffer exportData = package.getExportData(nodeTexture.exportID);
        if (exportData.ptr() == nullptr)
        {
            PERROR(QString(QString("Error: Texture ") + package.exportsTable[nodeTexture.exportID].objectName +
                    " has broken export data in package: " +
                    nodeTexture.path +"\nExport UIndex: " + QString::number(nodeTexture.exportID + 1) +
                    "\nSkipping...\n").toStdString().c_str());
            return;
        }
        Texture texture(package, nodeTexture.exportID, exportData);
        exportData.Free();
        ByteBuffer data = texture.getTopImageData();
        if (data.ptr() == nullptr)
        {
            PERROR(QString(QString("Error: Texture ") + package.exportsTable[nodeTexture.exportID].objectName +
                    " has broken export data in package: " +
                    nodeTexture.path +"\nExport UIndex: " + QString::number(nodeTexture.exportID + 1) +
                    "\nSkipping...\n").toStdString().c_str());
            return;
        }
        int width = texture.getTopMipmap().width;
        int height = texture.getTopMipmap().height;
        PixelFormat pixelFormat = Image::getPixelFormatType(texture.getProperties().getProperty("Format").getValueName());
        if (texture.getProperties().exists("CompressionSettings") &&
            texture.getProperties().getProperty("CompressionSettings").getValueName() == "TC_HighDynamicRange")
        {
            pixelFormat = PixelFormat::RGBE;
        }
        bool oneBitAlpha = texture.getProperties().exists("CompressionSettings") &&
                           texture.getProperties().getProperty("CompressionSettings").getValueName() == "TC_OneBitAlpha";
        bool clearAlpha = (pixelFormat == PixelFormat::DXT1) && !oneBitAlpha;
        ByteBuffer bitmap;
        if (imageViewMode)
        {
            bitmap = Image::convertRawToBGR(data, width, height, pixelFormat, clearAlpha);
        }
        else
        {
            bitmap = Image::convertRawToAlphaGreyscale(data, width, height, pixelFormat, clearAlpha);
        }
        QImage image = QImage(bitmap.ptr(), width, height, width * 3, QImage::Format::Format_RGB888);
        data.Free();
        auto pixmap = QPixmap::fromImage(image);
        labelImage->setPixmap(pixmap);
        bitmap.Free();
    }
    else
    {
        if (!singleInfoMode && textures[viewTexture.indexInTextures].list.count() > 50)
        {
            int answer = QMessageBox::question(this, "Texture Manager",
                                  QString("Detected more than 50 instances of this texture.") +
                  "\n\nYou can continue or switch to single instance view.", "Continue", "Single mode");
            if (answer != 0)
            {
                singleInfoMode = true;
            }
        }

        QString text;
        text += "Texture original CRC:  " +
                QString::asprintf("0x%08X", textures[viewTexture.indexInTextures].crc) + "\n";
        text += "Node name:             " + textures[viewTexture.indexInTextures].name + "\n";
        for (int index2 = 0; index2 < (!singleInfoMode ? textures[viewTexture.indexInTextures].list.count() : 1); index2++)
        {
            if (textures[viewTexture.indexInTextures].list[index2].path.count() == 0)
                continue;
            TextureMapPackageEntry nodeTexture = textures[viewTexture.indexInTextures].list[index2];
            Package package;
            package.Open(g_GameData->GamePath() + nodeTexture.path);
            ByteBuffer exportData = package.getExportData(nodeTexture.exportID);
            if (exportData.ptr() == nullptr)
            {
                text += "Error: Texture " + package.exportsTable[nodeTexture.exportID].objectName +
                        " has broken export data in package: " +
                        nodeTexture.path +"\nExport UIndex: " + QString::number(nodeTexture.exportID + 1) +
                        "\nSkipping...\n";
                continue;
            }
            if (nodeTexture.movieTexture)
            {
                TextureMovie textureMovie(package, nodeTexture.exportID, exportData);
                exportData.Free();
                text += "\nTexture instance: " + QString::number(index2 + 1) + "\n";
                text += "  Texture name:       " + package.exportsTable[nodeTexture.exportID].objectName + "\n";
                text += "  Export UIndex:          " + QString::number(nodeTexture.exportID + 1) + "\n";
                text += "  Bik data size:      " + QString::number(textureMovie.getUncompressedSize()) + "\n";
                text += "  Package path:       " + nodeTexture.path + "\n";
                text += "  Texture properties:\n";
                for (int l = 0; l < textureMovie.getProperties().propertyList.count(); l++)
                {
                    text += "  " + textureMovie.getProperties().getDisplayString(l);
                }
            }
            else
            {
                Texture texture(package, nodeTexture.exportID, exportData);
                exportData.Free();
                text += "\nTexture instance: " + QString::number(index2 + 1) + "\n";
                text += "  Texture name:       " + package.exportsTable[nodeTexture.exportID].objectName + "\n";
                text += "  Export UIndex:          " + QString::number(nodeTexture.exportID + 1) + "\n";
                text += "  Package path:       " + nodeTexture.path + "\n";
                text += "  Alpha fully opaque: " + (!nodeTexture.hasAlphaData ? QString("yes") : QString("no")) + "\n";
                text += "  Texture properties:\n";
                for (int l = 0; l < texture.getProperties().propertyList.count(); l++)
                {
                    text += "  " + texture.getProperties().getDisplayString(l);
                }
                text += "\n";
                for (int l = 0; l < texture.mipMapsList.count(); l++)
                {
                    text += "  MipMap: " + QString::number(l) + ", " + QString::number(texture.mipMapsList[l].width) +
                            "x" + QString::number(texture.mipMapsList[l].height) + "\n";
                    text += "    StorageType: " + Package::StorageTypeToString(texture.mipMapsList[l].storageType) + "\n";
                    text += "    DataOffset:  " + QString::number((int)texture.mipMapsList[l].dataOffset) + "\n";
                    text += "    CompSize:    " + QString::number(texture.mipMapsList[l].compressedSize) + "\n";
                    text += "    UnCompSize:  " + QString::number(texture.mipMapsList[l].uncompressedSize) + "\n";
                }
            }
        }
        textRight->setPlainText(text);
    }
}

void LayoutTexturesManager::ListMiddleTextureSelected(QListWidgetItem *current,
                                                      QListWidgetItem * /*previous*/)
{
    if (current != nullptr)
    {
        if (singlePackageMode)
            UpdateRightList(current);
        else
            UpdateRight(current);
        textureSelected = true;
    }
    else
    {
        textureSelected = false;
        textRight->clear();
        listRight->clear();
        labelImage->clear();
    }
    UpdateGui();
}

void LayoutTexturesManager::ReplaceTexture(const QListWidgetItem *item, bool convertMode)
{
    LockGui(true);

    auto viewTexture = item->data(Qt::UserRole).value<ViewTexture>();
    TextureMapPackageEntry nodeTexture;
    for (int index = 0; index < textures[viewTexture.indexInTextures].list.count(); index++)
    {
        if (textures[viewTexture.indexInTextures].list[index].path.length() != 0)
        {
            nodeTexture = textures[viewTexture.indexInTextures].list[index];
            break;
        }
    }
    QString filter;
    if (nodeTexture.movieTexture)
        filter = "Movie Texture (*.bik)";
    else
        filter = "Texture (*.dds *.png *.bmp *.tga)";
    QString file = QFileDialog::getOpenFileName(this,
            "Please select texture file", "", filter);
    if (file.length() == 0)
    {
        LockGui(false);
        return;
    }

    g_logs->BufferClearErrors();
    g_logs->BufferEnableErrors(true);
    Image *image = nullptr;
    if (!nodeTexture.movieTexture)
    {
        image = new Image(file);
        if (image->getPixelFormat() == PixelFormat::Internal && !image->isSource8Bits())
            image->convertInternalToRGBA16();
    }
    mainWindow->statusBar()->clearMessage();
    g_logs->BufferEnableErrors(false);
    if (g_logs->BufferGetErrors() != "")
    {
        MessageWindow msg;
        msg.Show(mainWindow, "Replacing texture", g_logs->BufferGetErrors());
        LockGui(false);
        return;
    }

    MipMaps mipMaps;
    QList<ModEntry> modsToReplace;
    QStringList pkgsToMarker;
    QStringList pkgsToRepack;
    ModEntry modEntry;
    modEntry.injectedTexture = image;
    auto texture = textures[viewTexture.indexInTextures];
    if (nodeTexture.movieTexture)
    {
        FileStream fs(file, FileMode::Open);
        quint32 tag = fs.ReadUInt32();
        if (tag != BIK1_TAG && tag != BIK2_TAG)
        {
            QMessageBox::critical(this, "Replacing texture",
                                  QString("File does not contain a supported version of Bik."));
            LockGui(false);
            return;
        }
        int dataSize = fs.ReadInt32() + 8;
        if (dataSize != QFile(file).size())
        {
            QMessageBox::critical(this, "Replacing texture",
                                  QString("This movie texture lists the wrong size (bad header)."));
            LockGui(false);
            return;
        }
        auto f = Misc::FoundTextureInTheInternalMap(gameType, texture.crc);
        if (f.crc != 0)
        {
            fs.Skip(12);
            int w = fs.ReadInt32();
            int h = fs.ReadInt32();
            if (w / h != f.width / f.height)
            {
                QMessageBox::critical(this, "Replacing texture",
                                      QString("This movie texture has the wrong aspect ratio (bad header)."));
                LockGui(false);
                return;
            }
        }
        fs.SeekBegin();
        ByteBuffer data = fs.ReadToBuffer(dataSize);
        modEntry.injectedMovieTexture = data;
    }
    modEntry.textureCrc = texture.crc;
    modEntry.textureName = texture.name;
    modEntry.markConvert = convertMode;
    modEntry.instance = 1;
    modsToReplace.append(modEntry);
    mainWindow->statusBar()->clearMessage();
    g_logs->BufferEnableErrors(false);
    if (singlePackageMode)
    {
        MapPackagesToModEntry entry{};
        entry.texturesIndex = viewTexture.indexInTextures;
        entry.listIndex = viewTexture.indexInPackages;
        MapPackagesToMod mapEntry;
        mapEntry.textures.append(entry);
        mapEntry.packagePath = texture.list[viewTexture.indexInPackages].path;
        QList<MapPackagesToMod> mapPackages;
        mapPackages.append(mapEntry);
        mipMaps.replaceTextures(mapPackages, textures, pkgsToMarker, modsToReplace,
                                false, false, -1,
                                &LayoutTexturesManager::ReplaceTextureCallback, mainWindow);
    }
    else
    {
        mipMaps.replaceModsFromList(textures, pkgsToMarker, modsToReplace,
                                    false, false, -1,
                                    &LayoutTexturesManager::ReplaceTextureCallback, mainWindow);
    }
    delete image;
    if (nodeTexture.movieTexture)
    {
        modEntry.injectedMovieTexture.Free();
    }
    TOCBinFile::UpdateAllTOCBinFiles(gameType);
    mainWindow->statusBar()->clearMessage();
    UpdateRight(item);
    if (g_logs->BufferGetErrors() != "")
    {
        MessageWindow msg;
        msg.Show(mainWindow, "Replacing texture", g_logs->BufferGetErrors());
    }
    else
    {
        QMessageBox::information(this, "Replacing texture", "Replacing texture completed.");
    }

    LockGui(false);
}

void LayoutTexturesManager::ReplaceSelected()
{
    QListWidgetItem *item;
    if (!textureSelected)
        return;
    if (singlePackageMode)
        item = listRight->currentItem();
    else
        item = listMiddle->currentItem();
    if (item != nullptr)
        ReplaceTexture(item, false);
}

void LayoutTexturesManager::ReplaceConvertSelected()
{
    QListWidgetItem *item;
    if (singlePackageMode)
        item = listRight->currentItem();
    else
        item = listMiddle->currentItem();
    if (item != nullptr)
    {
        ReplaceTexture(item, true);
    }
}

void LayoutTexturesManager::ExtractTexture(const ViewTexture& viewTexture, bool png)
{
    LockGui(true);

    QString outputDir = QFileDialog::getExistingDirectory(this,
            "Please select destination directory for extracted file");
    if (outputDir == "")
    {
        LockGui(false);
        return;
    }
    outputDir = QDir::cleanPath(outputDir);

    TextureMapPackageEntry nodeTexture;
    if (singlePackageMode)
    {
        nodeTexture = textures[viewTexture.indexInTextures].list[viewTexture.indexInPackages];
    }
    else
    {
        for (int index = 0; index < textures[viewTexture.indexInTextures].list.count(); index++)
        {
            if (textures[viewTexture.indexInTextures].list[index].path.length() != 0)
            {
                nodeTexture = textures[viewTexture.indexInTextures].list[index];
                break;
            }
        }
    }
    Package package;
    package.Open(g_GameData->GamePath() + nodeTexture.path);
    ByteBuffer exportData = package.getExportData(nodeTexture.exportID);
    if (exportData.ptr() == nullptr)
    {
        QMessageBox::critical(this, "Extracting texture", QString("Error: Texture ") +
                              package.exportsTable[nodeTexture.exportID].objectName +
                              " has broken export data in package: " +
                              nodeTexture.path +"\nExport UIndex: " + QString::number(nodeTexture.exportID + 1));
        LockGui(false);
        return;
    }
    if (nodeTexture.movieTexture)
    {
        TextureMovie textureMovie(package, nodeTexture.exportID, exportData);
        exportData.Free();

        uint crc = Misc::GetCRCFromTextureMap(textures, nodeTexture.exportID, nodeTexture.path);
        if (crc == 0)
            crc = textureMovie.getCrcData();
        if (crc == 0)
        {
            QMessageBox::critical(this, "Extracting texture", QString("Error: Movie texture ") +
                                  package.exportsTable[nodeTexture.exportID].objectName +
                                  " has broken export data in package: " +
                                  nodeTexture.path +"\nExport UIndex: " + QString::number(nodeTexture.exportID + 1));
            LockGui(false);
            return;
        }

        QString outputFile = outputDir + "/" +
                package.exportsTable[nodeTexture.exportID].objectName +
                QString::asprintf("_0x%08X.bik", crc);
        if (QFile(outputFile).exists())
            QFile(outputFile).remove();
        auto data = textureMovie.getData();
        FileStream fs = FileStream(outputFile, FileMode::Create);
        fs.WriteFromBuffer(data);
        data.Free();
    }
    else
    {
        Texture texture(package, nodeTexture.exportID, exportData);
        exportData.Free();

        uint crc = Misc::GetCRCFromTextureMap(textures, nodeTexture.exportID, nodeTexture.path);
        if (crc == 0)
            crc = texture.getCrcTopMipmap();
        if (crc == 0)
        {
            QMessageBox::critical(this, "Extracting texture", QString("Error: Texture ") +
                                  package.exportsTable[nodeTexture.exportID].objectName +
                                  " has broken export data in package: " +
                                  nodeTexture.path +"\nExport UIndex: " + QString::number(nodeTexture.exportID + 1));
            LockGui(false);
            return;
        }

        QString outputFile = outputDir + "/" +
                package.exportsTable[nodeTexture.exportID].objectName +
                QString::asprintf("_0x%08X", crc);
        if (png)
        {
            outputFile += ".png";
        }
        else
        {
            outputFile += ".dds";
        }
        if (QFile(outputFile).exists())
            QFile(outputFile).remove();

        PixelFormat pixelFormat = Image::getPixelFormatType(texture.getProperties().getProperty("Format").getValueName());
        if (texture.getProperties().exists("CompressionSettings") &&
            texture.getProperties().getProperty("CompressionSettings").getValueName() == "TC_HighDynamicRange")
        {
            pixelFormat = PixelFormat::RGBE;
        }

        bool oneBitAlpha = texture.getProperties().exists("CompressionSettings") &&
                           texture.getProperties().getProperty("CompressionSettings").getValueName() == "TC_OneBitAlpha";
        bool clearAlpha = (pixelFormat == PixelFormat::DXT1) && !oneBitAlpha;
        bool storeAs16Bits = false;
        if (pixelFormat == PixelFormat::RGBE ||
            pixelFormat == PixelFormat::R10G10B10A2 ||
            pixelFormat == PixelFormat::R16G16B16A16)
        {
            storeAs16Bits = true;
        }
        if (png)
        {
            ByteBuffer data = texture.getTopImageData();
            if (data.ptr() == nullptr)
            {
                QMessageBox::critical(this, "Extracting texture", QString("Error: Texture ") +
                                      package.exportsTable[nodeTexture.exportID].objectName +
                                      " has broken export data in package: " +
                                      nodeTexture.path +"\nExport UIndex: " + QString::number(nodeTexture.exportID + 1));
                LockGui(false);
                return;
            }
            Texture::TextureMipMap mipmap = texture.getTopMipmap();
            Image::saveToPng(data, mipmap.width, mipmap.height, pixelFormat, outputFile, !storeAs16Bits, clearAlpha);
            data.Free();
        }
        else
        {
            texture.removeEmptyMips();
            QList<MipMap *> mipmaps = QList<MipMap *>();
            for (int k = 0; k < texture.mipMapsList.count(); k++)
            {
                ByteBuffer data = texture.getMipMapDataByIndex(k);
                if (data.ptr() == nullptr)
                {
                    QMessageBox::critical(this, "Extracting texture", QString("Error: Texture ") +
                                          package.exportsTable[nodeTexture.exportID].objectName +
                                          " has broken export data in package: " +
                                          nodeTexture.path +"\nExport UIndex: " + QString::number(nodeTexture.exportID + 1));
                    LockGui(false);
                    return;
                }
                mipmaps.push_back(new MipMap(data, texture.mipMapsList[k].width, texture.mipMapsList[k].height, pixelFormat));
                data.Free();
            }
            Image image = Image(mipmaps, pixelFormat);
            FileStream fs = FileStream(outputFile, FileMode::Create, FileAccess::WriteOnly);
            image.StoreImageToDDS(fs);
        }
    }

    QMessageBox::information(this, "Extracting texture", "Completed extracting texture.");

    LockGui(false);
}

void LayoutTexturesManager::ExtractDDSSelected()
{
    QListWidgetItem *item;
    if (!textureSelected)
        return;
    if (singlePackageMode)
        item = listRight->currentItem();
    else
        item = listMiddle->currentItem();
    if (item != nullptr)
        ExtractTexture(item->data(Qt::UserRole).value<ViewTexture>(), false);
}

void LayoutTexturesManager::ExtractPNGSelected()
{
    QListWidgetItem *item;
    if (!textureSelected)
        return;
    if (singlePackageMode)
        item = listRight->currentItem();
    else
        item = listMiddle->currentItem();
    if (item != nullptr)
        ExtractTexture(item->data(Qt::UserRole).value<ViewTexture>(), true);
}

void LayoutTexturesManager::ExtractBIKSelected()
{
    QListWidgetItem *item;
    if (!textureSelected)
        return;
    if (singlePackageMode)
        item = listRight->currentItem();
    else
        item = listMiddle->currentItem();
    if (item != nullptr)
        ExtractTexture(item->data(Qt::UserRole).value<ViewTexture>(), true);
}

void LayoutTexturesManager::ViewImageSelected()
{
    if (!textureSelected)
        return;
    if (imageViewMode)
        return;
    if (singlePackageMode)
        return;
    singleInfoMode = false;
    imageViewMode = true;
    imageViewAlphaMode = false;
    auto item = listMiddle->currentItem();
    if (item != nullptr)
        UpdateRight(item);
    UpdateGui();
}

void LayoutTexturesManager::ViewImageAlphaSelected()
{
    if (!textureSelected)
        return;
    if (imageViewAlphaMode)
        return;
    if (singlePackageMode)
        return;
    singleInfoMode = false;
    imageViewMode = false;
    imageViewAlphaMode = true;
    auto item = listMiddle->currentItem();
    if (item != nullptr)
        UpdateRight(item);
    UpdateGui();
}

void LayoutTexturesManager::InfoSingleSelected()
{
    if (!textureSelected)
        return;
    if (singleInfoMode && !imageViewMode && !imageViewAlphaMode)
        return;
    if (singlePackageMode)
        return;
    singleInfoMode = true;
    imageViewMode = false;
    imageViewAlphaMode = false;
    auto item = listMiddle->currentItem();
    if (item != nullptr)
        UpdateRight(item);
    UpdateGui();
}

void LayoutTexturesManager::InfoAllSelected()
{
    if (!textureSelected)
        return;
    if (!singleInfoMode && !imageViewMode && !imageViewAlphaMode)
        return;
    if (singlePackageMode)
        return;
    singleInfoMode = false;
    imageViewMode = false;
    imageViewAlphaMode = false;
    auto item = listMiddle->currentItem();
    if (item != nullptr)
        UpdateRight(item);
    UpdateGui();
}

void LayoutTexturesManager::ListRightSelected(QListWidgetItem *current,
                                              QListWidgetItem * /*previous*/)
{
    textureInstanceSelected = current != nullptr;
    UpdateGui();
}

void LayoutTexturesManager::UpdateRightList(const QListWidgetItem *item)
{
    listRight->clear();
    int indexInTextures = item->data(Qt::UserRole).value<ViewTexture>().indexInTextures;
    for (int index = 0; index < textures[indexInTextures].list.count(); index++)
    {
        if (textures[indexInTextures].list[index].path.length() != 0)
        {
            ViewTexture newTexture;
            newTexture.indexInTextures = indexInTextures;
            newTexture.indexInPackages = index;
            auto textureItem = new QListWidgetItem(QString::number(index + 1) + " - " +
                                                   BaseNameWithoutExt(textures[indexInTextures].list[index].path));
            textureItem->setData(Qt::UserRole, QVariant::fromValue<ViewTexture>(newTexture));
            listRight->addItem(textureItem);
        }
    }
}

void LayoutTexturesManager::PackageSingleSelected()
{
    singlePackageMode = true;
    listRight->clear();
    auto item = listMiddle->currentItem();
    if (item != nullptr)
        UpdateRightList(item);
    UpdateGui();
}

void LayoutTexturesManager::PackageMutiSelected()
{
    singlePackageMode = false;
    listRight->clear();
    auto item = listMiddle->currentItem();
    if (item != nullptr)
        UpdateRightList(item);
    UpdateGui();
}

void LayoutTexturesManager::selectFoundTexture(const QListWidgetItem *item)
{
    leftWidget->setCurrentIndex(kLeftWidgetPackages);
    auto searchTexture = item->data(Qt::UserRole).value<ViewTexture>();
    QString packageName = BaseNameWithoutExt(textures[searchTexture.indexInTextures].list[searchTexture.indexInPackages].path);
    for (int p = 0; p < listLeftPackages->count(); p++)
    {
        auto itemPackage = listLeftPackages->item(p);
        if (itemPackage->text() == packageName)
        {
            itemPackage->setSelected(true);
            listLeftPackages->setCurrentRow(p);
            for (int t = 0; t < listMiddle->count(); t++)
            {
                auto searchItem = listMiddle->item(t);
                auto viewTexture = searchItem->data(Qt::UserRole).value<ViewTexture>();
                if (viewTexture.indexInTextures == searchTexture.indexInTextures)
                {
                    searchItem->setSelected(true);
                    listMiddle->setCurrentRow(t);
                    return;
                }
            }
        }
    }
}

void LayoutTexturesManager::SearchListSelected(QListWidgetItem *item)
{
    if (item != nullptr)
    {
        LockGui(true);
        selectFoundTexture(item);
        LockGui(false);
    }
}

void LayoutTexturesManager::SearchTexture(const QString &name, uint crc)
{
    listLeftSearch->setUpdatesEnabled(false);
    listLeftSearch->clear();
    for (int l = 0; l < textures.count(); l++)
    {
        const TextureMapEntry& foundTexture = textures[l];
        bool found = false;
        if (name != "")
        {
            if (name.contains("*"))
            {
                QRegExp regex(name.toLower());
                regex.setPatternSyntax(QRegExp::Wildcard);
                if (regex.exactMatch(foundTexture.name.toLower()))
                {
                    found = true;
                }
            }
            else if (foundTexture.name.toLower() == name.toLower())
            {
                found = true;
            }
        }
        else if (crc != 0 && foundTexture.crc == crc)
        {
            found = true;
        }
        if (found)
        {
            TextureMapPackageEntry nodeTexture;
            int indexInPackages;
            for (indexInPackages = 0; indexInPackages < foundTexture.list.count(); indexInPackages++)
            {
                if (foundTexture.list[indexInPackages].path.length() != 0)
                {
                    nodeTexture = foundTexture.list[indexInPackages];
                    break;
                }
            }
            auto item = new QListWidgetItem(foundTexture.name +
                                            " (" + BaseNameWithoutExt(nodeTexture.path) + ")");
            ViewTexture texture;
            texture.name = foundTexture.name;
            texture.indexInTextures = l;
            texture.indexInPackages = indexInPackages;
            item->setData(Qt::UserRole, QVariant::fromValue<ViewTexture>(texture));
            listLeftSearch->addItem(item);
        }
    }
    listLeftSearch->sortItems();
    listLeftSearch->setUpdatesEnabled(true);
    if (listLeftSearch->count() > 1)
    {
        leftWidget->setCurrentIndex(kLeftWidgetSearch);
        listMiddle->clear();
        labelImage->clear();
    }
    if (listLeftSearch->count() == 1)
    {
        listMiddle->setFocus(Qt::OtherFocusReason);
        selectFoundTexture(listLeftSearch->item(0));
    }
    if (listLeftSearch->count() == 0)
    {
        QMessageBox::information(this, "Search for texture", "Texture not found.");
    }
}

void LayoutTexturesManager::SearchSelected()
{
    bool ok;
    QString text = QInputDialog::getText(this, "Search texture",
                                         "Please enter texture name or CRC (0xhhhhhhhh).\n\nYou can use * as wilcards for texture name.",
                                         QLineEdit::Normal, "", &ok);
    if (!ok || text.length() == 0)
        return;

    LockGui(true);
    uint crc = 0;
    if (text.contains("0x"))
    {
        int idx = text.indexOf("0x");
        if (text.size() - idx >= 10)
        {
            QString crcStr = text.mid(idx, 10);
            bool ok;
            crc = crcStr.toUInt(&ok, 16);
        }
    }

    if (crc != 0)
    {
        SearchTexture("", crc);
    }
    else
    {
        text = text.split('.').first(); // in case filename
        SearchTexture(text, 0);
    }

    LockGui(false);
}

void LayoutTexturesManager::ExitSelected()
{
    mainWindow->SwitchLayoutById(MainWindow::kLayoutMain);
    mainWindow->GetLayout()->removeWidget(this);
    mainWindow->SetTitle(MeType::UNKNOWN_TYPE, "");
}
