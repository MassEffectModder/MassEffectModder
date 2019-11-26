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

#include <GameData/GameData.h>
#include <GameData/TOCFile.h>
#include <Gui/LayoutMain.h>
#include <Gui/LayoutTexturesManager.h>
#include <Gui/MainWindow.h>
#include <Gui/MessageWindow.h>
#include <Image/Image.h>
#include <Helpers/Exception.h>
#include <Helpers/QSort.h>
#include <Helpers/Logs.h>
#include <Helpers/MiscHelpers.h>
#include <MipMaps/MipMaps.h>
#include <Misc/Misc.h>
#include <Texture/Texture.h>

PixmapLabel::PixmapLabel(QWidget *parent) :
    QLabel(parent)
{
    setMinimumSize(1, 1);
}

void PixmapLabel::setPixmap(const QPixmap &p)
{
    pixmapImage = p;
    QLabel::setPixmap(resizePixmap());
}

int PixmapLabel::heightForWidth(int w) const
{
    if (pixmapImage.isNull())
        return height();
    return ((qreal)pixmapImage.height() * w) / pixmapImage.width();
}

void PixmapLabel::resizeEvent(QResizeEvent *event)
{
    if (!pixmapImage.isNull())
        QLabel::setPixmap(resizePixmap());
    QLabel::resizeEvent(event);
}

QPixmap PixmapLabel::resizePixmap() const
{
    auto resizedPixmap = pixmapImage.scaled(size() * devicePixelRatioF(),
                                            Qt::KeepAspectRatio, Qt::SmoothTransformation);
    resizedPixmap.setDevicePixelRatio(devicePixelRatioF());
    return resizedPixmap;
}

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

    buttonReplace = new QPushButton("Replace");
    buttonReplace->setMinimumWidth(kButtonMinWidth);
    buttonReplace->setMinimumHeight(kButtonMinHeight);
    auto ButtonFont = buttonReplace->font();
    ButtonFont.setPointSize(kFontSize);
    buttonReplace->setFont(ButtonFont);
    connect(buttonReplace, &QPushButton::clicked, this, &LayoutTexturesManager::ReplaceSelected);

    buttonReplaceConvert = new QPushButton("Replace (Uncompressed)");
    buttonReplaceConvert->setMinimumWidth(kButtonMinWidth);
    buttonReplaceConvert->setMinimumHeight(kButtonMinHeight);
    ButtonFont = buttonReplaceConvert->font();
    ButtonFont.setPointSize(kFontSize);
    buttonReplaceConvert->setFont(ButtonFont);
    connect(buttonReplaceConvert, &QPushButton::clicked, this, &LayoutTexturesManager::ReplaceConvertSelected);

    auto VerticalLayoutListReplace = new QVBoxLayout();
    VerticalLayoutListReplace->addWidget(buttonReplace);
    VerticalLayoutListReplace->addWidget(buttonReplaceConvert);
    VerticalLayoutListReplace->setAlignment(Qt::AlignTop);
    auto GroupBoxReplace = new QGroupBox("Replace Texture");
    GroupBoxReplace->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    GroupBoxReplace->setAlignment(Qt::AlignBottom);
    GroupBoxReplace->setLayout(VerticalLayoutListReplace);

    buttonExtractToDDS = new QPushButton("Extract to DDS file");
    buttonExtractToDDS->setMinimumWidth(kButtonMinWidth);
    buttonExtractToDDS->setMinimumHeight(kButtonMinHeight);
    ButtonFont = buttonExtractToDDS->font();
    ButtonFont.setPointSize(kFontSize);
    buttonExtractToDDS->setFont(ButtonFont);
    connect(buttonExtractToDDS, &QPushButton::clicked, this, &LayoutTexturesManager::ExtractDDSSelected);

    buttonExtractToPNG = new QPushButton("Extract to PNG file");
    buttonExtractToPNG->setMinimumWidth(kButtonMinWidth);
    buttonExtractToPNG->setMinimumHeight(kButtonMinHeight);
    ButtonFont = buttonExtractToPNG->font();
    ButtonFont.setPointSize(kFontSize);
    buttonExtractToPNG->setFont(ButtonFont);
    connect(buttonExtractToPNG, &QPushButton::clicked, this, &LayoutTexturesManager::ExtractPNGSelected);

    auto VerticalLayoutListExtract = new QVBoxLayout();
    VerticalLayoutListExtract->addWidget(buttonExtractToDDS);
    VerticalLayoutListExtract->addWidget(buttonExtractToPNG);
    VerticalLayoutListExtract->setAlignment(Qt::AlignTop);
    auto GroupBoxExtract = new QGroupBox("Extract Texture");
    GroupBoxExtract->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    GroupBoxExtract->setAlignment(Qt::AlignBottom);
    GroupBoxExtract->setLayout(VerticalLayoutListExtract);

    buttonViewImage = new QPushButton("View - Image");
    buttonViewImage->setMinimumWidth(kButtonMinWidth);
    buttonViewImage->setMinimumHeight(kButtonMinHeight);
    ButtonFont = buttonViewImage->font();
    ButtonFont.setPointSize(kFontSize);
    buttonViewImage->setFont(ButtonFont);
    connect(buttonViewImage, &QPushButton::clicked, this, &LayoutTexturesManager::ViewImageSelected);

    buttonInfoSingle = new QPushButton("Info - Single");
    buttonInfoSingle->setMinimumWidth(kButtonMinWidth);
    buttonInfoSingle->setMinimumHeight(kButtonMinHeight);
    ButtonFont = buttonInfoSingle->font();
    ButtonFont.setPointSize(kFontSize);
    buttonInfoSingle->setFont(ButtonFont);
    connect(buttonInfoSingle, &QPushButton::clicked, this, &LayoutTexturesManager::InfoSingleSelected);

    buttonInfoAll = new QPushButton("Info - All");
    buttonInfoAll->setMinimumWidth(kButtonMinWidth);
    buttonInfoAll->setMinimumHeight(kButtonMinHeight);
    ButtonFont = buttonInfoAll->font();
    ButtonFont.setPointSize(kFontSize);
    buttonInfoAll->setFont(ButtonFont);
    connect(buttonInfoAll, &QPushButton::clicked, this, &LayoutTexturesManager::InfoAllSelected);

    buttonPackageSingle = new QPushButton("Single Package");
    buttonPackageSingle->setMinimumWidth(kButtonMinWidth);
    buttonPackageSingle->setMinimumHeight(kButtonMinHeight);
    ButtonFont = buttonPackageSingle->font();
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
    VerticalLayoutListView->addWidget(buttonViewImage);
    VerticalLayoutListView->addWidget(buttonInfoSingle);
    VerticalLayoutListView->addWidget(buttonInfoAll);
    VerticalLayoutListView->addWidget(buttonPackageSingle);
    VerticalLayoutListView->addWidget(buttonPackageMulti);
    VerticalLayoutListView->setAlignment(Qt::AlignTop);
    auto GroupBoxView = new QGroupBox("Right Panel View");
    GroupBoxView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    GroupBoxView->setAlignment(Qt::AlignBottom);
    GroupBoxView->setLayout(VerticalLayoutListView);

    buttonSearch = new QPushButton("Search Texture");
    buttonSearch->setMinimumWidth(kButtonMinWidth);
    buttonSearch->setMinimumHeight(kButtonMinHeight);
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
    GroupBoxMisc->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    GroupBoxMisc->setAlignment(Qt::AlignBottom);
    GroupBoxMisc->setLayout(VerticalLayoutListMisc);

    auto HorizontalLayoutList = new QHBoxLayout();
    HorizontalLayoutList->addWidget(GroupBoxReplace, 1);
    HorizontalLayoutList->addWidget(GroupBoxExtract, 1);
    HorizontalLayoutList->addWidget(GroupBoxView, 1);
    HorizontalLayoutList->addWidget(GroupBoxMisc, 1);
    auto WidgetBottom = new QWidget();
    WidgetBottom->setLayout(HorizontalLayoutList);

    auto VerticalLayout = new QVBoxLayout(this);
    VerticalLayout->addWidget(splitter);
    VerticalLayout->addWidget(WidgetBottom);

    mainWindow->SetTitle(gameType, "Texture Manager");
}

static int compareTextures(const ViewPackage &e1, const ViewPackage &e2)
{
    int compResult = e1.packageName.compare(e2.packageName, Qt::CaseInsensitive);
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

void LayoutTexturesManager::Startup()
{
    LockGui(true);

    mainWindow->statusBar()->showMessage("Detecting game data...");
    QApplication::processEvents();
    g_GameData->Init(gameType, configIni, true);
    if (!Misc::CheckGamePath())
    {
        QMessageBox::critical(this, "Texture Manager", "Game data not found.");
        mainWindow->statusBar()->clearMessage();
        buttonExit->setEnabled(true);
        mainWindow->LockClose(false);
        return;
    }

    if (gameType == MeType::ME3_TYPE && Misc::unpackSFARisNeeded())
    {
        QMessageBox::critical(this, "Texture Manager", QString("Game has NOT unpacked DLCs.") +
                              "\n\nPlease select 'Unpack DLCs'\n" + "from the 'Game Utilities'\n" +
                              "then start Texture Manager again.");
        mainWindow->statusBar()->clearMessage();
        buttonExit->setEnabled(true);
        mainWindow->LockClose(false);
        return;
    }

    QString path = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation).first() +
            "/MassEffectModder";
    QString filename = path + QString("/me%1map.bin").arg(static_cast<int>(gameType));
    if (QFile::exists(filename))
    {
        FileStream fs = FileStream(filename, FileMode::Open, FileAccess::ReadOnly);
        uint tag = fs.ReadUInt32();
        uint version = fs.ReadUInt32();
        if (tag != textureMapBinTag || version != textureMapBinVersion)
        {
            QMessageBox::critical(this, "Texture Manager",
                                  QString("Detected wrong or old version of textures scan file!") +
                "\n\nYou need to restore the game to vanilla state then reinstall optional DLC/PCC mods." +
                "\n\nThen from the 'Texture Utilities', select 'Delete Textures Scan File' and start Texture Manager again.");
            mainWindow->statusBar()->clearMessage();
            buttonExit->setEnabled(true);
            mainWindow->LockClose(false);
            return;
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
            if (!g_GameData->packageFiles.contains(packages[i], Qt::CaseInsensitive))
            {
                QMessageBox::critical(this, "Texture Manager",
                                      QString("Detected removal of game files since last game data scan.") +
                      "\n\nYou need to restore the game to vanilla state then reinstall optional DLC/PCC mods." +
                      "\n\nThen from the 'Texture Utilities', select 'Delete Textures Scan File' and start Texture Manager again.");
                mainWindow->statusBar()->clearMessage();
                buttonExit->setEnabled(true);
                mainWindow->LockClose(false);
                return;
            }
        }
        for (int i = 0; i < g_GameData->packageFiles.count(); i++)
        {
            if (!packages.contains(g_GameData->packageFiles[i], Qt::CaseInsensitive))
            {
                QMessageBox::critical(this, "Texture Manager",
                                      QString("Detected additional game files not present in latest game data scan.") +
                      "\n\nYou need to restore the game to vanilla state then reinstall optional DLC/PCC mods." +
                      "\n\nThen from the 'Texture Utilities', select 'Delete Textures Scan File' and start Texture Manager again.");
                mainWindow->statusBar()->clearMessage();
                buttonExit->setEnabled(true);
                mainWindow->LockClose(false);
                return;
            }
        }
        if (!TreeScan::loadTexturesMapFile(filename, textures))
        {
            QMessageBox::critical(this, "Texture Manager", "Failed to load texture map.");
            buttonExit->setEnabled(true);
            mainWindow->LockClose(false);
            return;
        }
    }
    else
    {
        bool modded = Misc::detectMod(gameType);
        if (!modded)
            modded = Misc::CheckForMarkers(&LayoutTexturesManager::PrepareTexturesCallback, mainWindow);
        mainWindow->statusBar()->clearMessage();
        if (!modded)
        {
            QMessageBox::critical(this, "Texture Manager",
                                  QString("Detected game as already modded.") +
                  "\n\nYou need to restore the game to vanilla state then reinstall optional DLC/PCC mods." +
                  "\n\nThen start Texture Manager again.");
            mainWindow->statusBar()->clearMessage();
            buttonExit->setEnabled(true);
            mainWindow->LockClose(false);
            return;
        }
    }

    mainWindow->statusBar()->showMessage("Checking for not compatible mods...");
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
                              QString("Detected not compatible mods: \n\n") + list);
        mainWindow->statusBar()->clearMessage();
        buttonExit->setEnabled(true);
        mainWindow->LockClose(false);
        return;
    }

    mainWindow->statusBar()->showMessage("Checking for empty mips...");
    QApplication::processEvents();
    bool gameHasEmptyMips = false;
    QString packageVerify;
    int exportIdVerify = 0;
    if (GameData::gameType == MeType::ME1_TYPE)
    {
        packageVerify = "/BioGame/CookedPC/Packages/VFX_Prototype/v_Explosion_PrototypeTest_01.upk";
        exportIdVerify = 4888;
    }
    else if (GameData::gameType == MeType::ME2_TYPE)
    {
        packageVerify = "/BioGame/CookedPC/BioA_CitHub_500Udina.pcc";
        exportIdVerify = 3655;
    }
    else if (GameData::gameType == MeType::ME3_TYPE)
    {
        packageVerify = "/BioGame/CookedPCConsole/BIOG_UIWorld.pcc";
        exportIdVerify = 464;
    }

    if (QFile::exists(g_GameData->GamePath() + packageVerify))
    {
        Package package;
        int status = package.Open(g_GameData->GamePath() + packageVerify);
        if (status != 0)
        {
            QMessageBox::critical(this, "Texture Manager",
                                  QString("Failed game data detection."));
            buttonExit->setEnabled(true);
            mainWindow->LockClose(false);
            return;
        }
        ByteBuffer exportData = package.getExportData(exportIdVerify);
        if (exportData.ptr() == nullptr)
        {
            QMessageBox::critical(this, "Texture Manager",
                                  QString("Failed game data detection."));
            buttonExit->setEnabled(true);
            mainWindow->LockClose(false);
            return;
        }
        Texture texture(package, exportIdVerify, exportData);
        exportData.Free();
        gameHasEmptyMips = texture.hasEmptyMips();
    }

    mainWindow->statusBar()->clearMessage();
    QApplication::processEvents();
    bool removeEmptyMips = false;
    if (!QFile::exists(filename))
    {
        int result = QMessageBox::question(this, "Texture Manager",
                              QString("Replacing textures and creating mods requires generating a map of the game's textures.\n") +
                              "You only need to do it once.\n\n" +
                              "IMPORTANT! Your game needs to be in vanilla state and have optional DLC/PCC mods installed.\n\n" +
                              "You can continue or abort", "Continue", "Abort");
        if (result != 0)
        {
            buttonExit->setEnabled(true);
            mainWindow->LockClose(false);
            return;
        }
    }

    if (gameHasEmptyMips)
    {
        int answer = QMessageBox::question(this, "Texture Manager",
                              QString("Detected empty mips in game files.") +
              "\n\nYou can remove empty mips or you can skip this step.", "Remove", "Skip");
        if (answer == 0)
            removeEmptyMips = true;
    }

    if (!Misc::checkWriteAccessDir(g_GameData->MainData()))
    {
        QMessageBox::critical(this, "Texture Manager",
                              QString("Detected program has not write access to game folder.") +
              "\n\nCorrect access to game directory." +
              "\n\nThen start Texture Manager again.");
        buttonExit->setEnabled(true);
        mainWindow->LockClose(false);
        return;
    }

    if (!QFile::exists(filename))
    {
        mainWindow->statusBar()->showMessage("Preparing to scan textures...");
        QApplication::processEvents();
        resources.loadMD5Tables();
        g_logs->BufferClearErrors();
        g_logs->BufferEnableErrors(true);
        TreeScan::PrepareListOfTextures(gameType, resources,
                                        textures, removeEmptyMips, true,
                                        &LayoutTexturesManager::PrepareTexturesCallback,
                                        mainWindow);
        g_logs->BufferEnableErrors(false);
        mainWindow->statusBar()->clearMessage();
        QApplication::processEvents();
        if (g_logs->BufferGetErrors() != "")
        {
            MessageWindow msg;
            msg.Show(mainWindow, "Errors while scanning package files", g_logs->BufferGetErrors());
        }
    }
    else if (removeEmptyMips)
    {
        mainWindow->statusBar()->showMessage("Preparing to remove empty mips...");
        QApplication::processEvents();
        MipMaps mipMaps;
        QStringList pkgsToRepack;
        QStringList pkgsToMarker;
        g_logs->BufferClearErrors();
        g_logs->BufferEnableErrors(true);
        Misc::RemoveMipmaps(mipMaps, textures, pkgsToMarker, pkgsToRepack, false, false, true,
                            &LayoutTexturesManager::PrepareTexturesCallback, mainWindow);
        mainWindow->statusBar()->clearMessage();
        QApplication::processEvents();
        if (g_logs->BufferGetErrors() != "")
        {
            MessageWindow msg;
            msg.Show(mainWindow, "Errors while removing empty mips", g_logs->BufferGetErrors());
            buttonExit->setEnabled(true);
            mainWindow->LockClose(false);
            return;
        }
        if (GameData::gameType == MeType::ME3_TYPE)
            TOCBinFile::UpdateAllTOCBinFiles();
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
        if (ViewPackageList[l].packageName != lastPackageName)
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
    textureSelected = false;
    packageSelected = false;
    textureInstanceSelected = false;

    LockGui(false);
    UpdateGui();
}

void LayoutTexturesManager::PrepareTexturesCallback(void *handle, int progress, const QString &stage)
{
    auto *win = static_cast<MainWindow *>(handle);
    win->statusBar()->showMessage(QString("Preparing... Stage: ") + stage +
                                  " -  Progress: " + QString::number(progress) + "%");
    QApplication::processEvents();
}

void LayoutTexturesManager::ReplaceTextureCallback(void *handle, int progress, const QString & /*stage*/)
{
    auto *win = static_cast<MainWindow *>(handle);
    win->statusBar()->showMessage(QString("Replacing texture...") +
                                  " -  Progress: " + QString::number(progress) + "%");
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
    buttonReplace->setEnabled(!lock);
    buttonReplaceConvert->setEnabled(!lock);
    buttonExtractToDDS->setEnabled(!lock);
    buttonExtractToPNG->setEnabled(!lock);
    buttonViewImage->setEnabled(!lock);
    buttonInfoSingle->setEnabled(!lock);
    buttonInfoAll->setEnabled(!lock);
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
    bool enableTextureButtons = ((singlePackageMode && textureInstanceSelected) ||
                                 !singlePackageMode) && textureSelected;
    buttonReplace->setEnabled(enableTextureButtons);
    buttonReplaceConvert->setEnabled(enableTextureButtons);
    buttonExtractToDDS->setEnabled(enableTextureButtons);
    buttonExtractToPNG->setEnabled(enableTextureButtons);
    if (imageViewMode)
    {
        if (singlePackageMode)
            rightView->setCurrentIndex(kRightWidgetList);
        else
            rightView->setCurrentIndex(kRightWidgetImage);
        buttonViewImage->setEnabled(false);
        buttonInfoSingle->setEnabled(textureSelected && !singlePackageMode);
        buttonInfoAll->setEnabled(textureSelected && !singlePackageMode);
        if (gameType != MeType::ME1_TYPE)
            buttonPackageSingle->setEnabled(textureSelected && !singlePackageMode);
        buttonPackageMulti->setEnabled(textureSelected && singlePackageMode);
    }
    else
    {
        if (singlePackageMode)
            rightView->setCurrentIndex(kRightWidgetList);
        else
            rightView->setCurrentIndex(kRightWidgetText);
        buttonViewImage->setEnabled(textureSelected && !singlePackageMode);
        buttonInfoSingle->setEnabled(textureSelected && !singleInfoMode && !singlePackageMode);
        buttonInfoAll->setEnabled(textureSelected && singleInfoMode && !singlePackageMode);
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
    if (imageViewMode)
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
        Package package;
        package.Open(g_GameData->GamePath() + nodeTexture.path);
        ByteBuffer exportData = package.getExportData(nodeTexture.exportID);
        if (exportData.ptr() == nullptr)
        {
            PERROR(QString(QString("Error: Texture ") + package.exportsTable[nodeTexture.exportID].objectName +
                    " has broken export data in package: " +
                    nodeTexture.path +"\nExport Id: " + QString::number(nodeTexture.exportID + 1) +
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
                    nodeTexture.path +"\nExport Id: " + QString::number(nodeTexture.exportID + 1) +
                    "\nSkipping...\n").toStdString().c_str());
            return;
        }
        int width = texture.getTopMipmap().width;
        int height = texture.getTopMipmap().height;
        PixelFormat pixelFormat = Image::getPixelFormatType(texture.getProperties().getProperty("Format").valueName);
        auto bitmap = Image::convertRawToARGB(data.ptr(), width, height, pixelFormat);
        data.Free();
        QImage image(bitmap.ptr(), width, height, width * 4, QImage::Format::Format_ARGB32);
        auto pixmap = QPixmap::fromImage(image);
        labelImage->setPixmap(pixmap);
        bitmap.Free();
    }
    else
    {
        QString text;
        text += "Texture original CRC:  " +
                QString().sprintf("0x%08X", textures[viewTexture.indexInTextures].crc) + "\n";
        text += "Node name:     " + textures[viewTexture.indexInTextures].name + "\n";
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
                        nodeTexture.path +"\nExport Id: " + QString::number(nodeTexture.exportID + 1) +
                        "\nSkipping...\n";
                continue;
            }
            Texture texture(package, nodeTexture.exportID, exportData);
            exportData.Free();
            text += "\nTexture instance: " + QString::number(index2 + 1) + "\n";
            text += "  Texture name:       " + package.exportsTable[nodeTexture.exportID].objectName + "\n";
            text += "  Export Id:          " + QString::number(nodeTexture.exportID + 1) + "\n";
            if (g_GameData->GamePath() == MeType::ME1_TYPE)
            {
                if (nodeTexture.linkToMaster == -1)
                    text += "  Master Texture\n";
                else
                {
                    text += "  Slave Texture\n";
                    text += "    Refer to package: " + texture.basePackageName + "\n";
                    text += "    Refer to texture: " + QString::number(nodeTexture.linkToMaster + 1) + "\n";
                }
            }
            text += "  Package path:       " + nodeTexture.path + "\n";
            text += "  Texture properties:\n";
            for (int l = 0; l < texture.getProperties().texPropertyList.count(); l++)
            {
                text += "  " + texture.getProperties().getDisplayString(l);
            }
            text += "\n";
            for (int l = 0; l < texture.mipMapsList.count(); l++)
            {
                text += "  MipMap: " + QString::number(l) + ", " + QString::number(texture.mipMapsList[l].width) +
                        "x" + QString::number(texture.mipMapsList[l].height) + "\n";
                text += "    StorageType: " + Texture::StorageTypeToString(texture.mipMapsList[l].storageType) + "\n";
                text += "    DataOffset:  " + QString::number((int)texture.mipMapsList[l].dataOffset) + "\n";
                text += "    CompSize:    " + QString::number(texture.mipMapsList[l].compressedSize) + "\n";
                text += "    UnCompSize:  " + QString::number(texture.mipMapsList[l].uncompressedSize) + "\n";
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
    QString file = QFileDialog::getOpenFileName(this,
            "Please select texture file", "", "Texture (*.dds *.png *.bmp *.tga)");
    if (file.length() == 0)
    {
        LockGui(false);
        return;
    }

    g_logs->BufferClearErrors();
    g_logs->BufferEnableErrors(true);
    auto image = new Image(file);
    mainWindow->statusBar()->clearMessage();
    g_logs->BufferEnableErrors(false);
    if (g_logs->BufferGetErrors() != "")
    {
        MessageWindow msg;
        msg.Show(mainWindow, "Replacing texture", g_logs->BufferGetErrors());
        LockGui(false);
        return;
    }

    TextureMapPackageEntry nodeTexture;
    for (int index = 0; index < textures[viewTexture.indexInTextures].list.count(); index++)
    {
        if (textures[viewTexture.indexInTextures].list[index].path.length() != 0)
        {
            nodeTexture = textures[viewTexture.indexInTextures].list[index];
            break;
        }
    }
    MipMaps mipMaps;
    QList<ModEntry> modsToReplace;
    QStringList pkgsToMarker;
    QStringList pkgsToRepack;
    ModEntry modEntry;
    modEntry.injectedTexture = image;
    modEntry.textureCrc = textures[viewTexture.indexInTextures].crc;
    modEntry.textureName = textures[viewTexture.indexInTextures].name;
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
        mapEntry.packagePath = textures[viewTexture.indexInTextures].list[viewTexture.indexInPackages].path;
        QList<MapPackagesToMod> mapPackages;
        mapPackages.append(mapEntry);
        mipMaps.replaceTextures(mapPackages, textures, pkgsToMarker, pkgsToRepack, modsToReplace,
                                false, false, true, false, -1,
                                &LayoutTexturesManager::ReplaceTextureCallback, mainWindow);
    }
    else
    {
        mipMaps.replaceModsFromList(textures, pkgsToMarker, pkgsToRepack, modsToReplace,
                                    false, false, true, false, -1,
                                    &LayoutTexturesManager::ReplaceTextureCallback, mainWindow);
    }
    delete image;
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
                              nodeTexture.path +"\nExport Id: " + QString::number(nodeTexture.exportID + 1));
        LockGui(false);
        return;
    }
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
                              nodeTexture.path +"\nExport Id: " + QString::number(nodeTexture.exportID + 1));
        LockGui(false);
        return;
    }

    QString outputFile = outputDir + "/" +
            package.exportsTable[nodeTexture.exportID].objectName +
            QString().sprintf("_0x%08X", crc);
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

    PixelFormat pixelFormat = Image::getPixelFormatType(texture.getProperties().getProperty("Format").valueName);
    if (png)
    {
        ByteBuffer data = texture.getTopImageData();
        if (data.ptr() == nullptr)
        {
            QMessageBox::critical(this, "Extracting texture", QString("Error: Texture ") +
                                  package.exportsTable[nodeTexture.exportID].objectName +
                                  " has broken export data in package: " +
                                  nodeTexture.path +"\nExport Id: " + QString::number(nodeTexture.exportID + 1));
            LockGui(false);
            return;
        }
        Texture::TextureMipMap mipmap = texture.getTopMipmap();
        Image::saveToPng(data.ptr(), mipmap.width, mipmap.height, pixelFormat, outputFile);
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
                                      nodeTexture.path +"\nExport Id: " + QString::number(nodeTexture.exportID + 1));
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

    QMessageBox::information(this, "Extracting texture", "Completed extracting texture.");

    LockGui(false);
}

void LayoutTexturesManager::ExtractDDSSelected()
{
    QListWidgetItem *item;
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
    if (singlePackageMode)
        item = listRight->currentItem();
    else
        item = listMiddle->currentItem();
    if (item != nullptr)
        ExtractTexture(item->data(Qt::UserRole).value<ViewTexture>(), true);
}

void LayoutTexturesManager::ViewImageSelected()
{
    if (!imageViewMode)
    {
        singleInfoMode = false;
        imageViewMode = true;
        UpdateGui();
    }
}

void LayoutTexturesManager::InfoSingleSelected()
{
    singleInfoMode = true;
    imageViewMode = false;
    auto item = listMiddle->currentItem();
    if (item != nullptr)
        UpdateRight(item);
    UpdateGui();
}

void LayoutTexturesManager::InfoAllSelected()
{
    singleInfoMode = false;
    imageViewMode = false;
    auto item = listMiddle->currentItem();
    if (item != nullptr)
        UpdateRight(item);
    UpdateGui();
}

void LayoutTexturesManager::ListRightSelected(QListWidgetItem *current,
                                              QListWidgetItem * /*previous*/)
{
    if (current != nullptr)
        textureInstanceSelected = true;
    else
        textureInstanceSelected = false;
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
        QMessageBox::information(this, "Search texture", "Texture not found.");
    }
}

void LayoutTexturesManager::SearchSelected()
{
    bool ok;
    QString text = QInputDialog::getText(this, "Search texture",
                                         "Please enter texture name or CRC (0xhhhhhhhh).\n\nYou can use * as wilcards.",
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
