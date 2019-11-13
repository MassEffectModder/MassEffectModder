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
#include <Gui/LayoutMeSelect.h>
#include <Gui/LayoutTexturesManager.h>
#include <Gui/MainWindow.h>
#include <Gui/MessageWindow.h>
#include <Helpers/Exception.h>
#include <Helpers/Logs.h>
#include <Helpers/MiscHelpers.h>
#include <Misc/Misc.h>

LayoutTexturesManager::LayoutTexturesManager(MainWindow *window)
    : mainWindow(window)
{
    layoutId = MainWindow::kLayoutTexturesManager;

    listLeft = new QListWidget();
    listMiddle = new QListWidget();

    labelImage = new QLabel();
    labelImage->setAutoFillBackground(true);
    textRight = new QPlainTextEdit;
    listRight = new QListWidget;
    rightView = new QStackedWidget();
    rightView->addWidget(labelImage);
    rightView->addWidget(textRight);
    rightView->addWidget(listRight);

    splitter = new QSplitter();
    splitter->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    splitter->addWidget(listLeft);
    splitter->addWidget(listMiddle);
    splitter->addWidget(rightView);

    buttonReplace = new QPushButton("Replace");
    buttonReplace->setMinimumWidth(kButtonMinWidth);
    buttonReplace->setMinimumHeight(kButtonMinHeight);
    auto ButtonFont = buttonReplace->font();
    ButtonFont.setPointSize(kFontSize);
    buttonReplace->setFont(ButtonFont);
    connect(buttonReplace, &QPushButton::clicked, this, &LayoutTexturesManager::ReplaceSelected);

    buttonReplaceConvert = new QPushButton("Replace (Force Convert)");
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
    connect(buttonInfoSingle, &QPushButton::clicked, this, &LayoutTexturesManager::ViewSingleSelected);

    buttonInfoAll = new QPushButton("Info - All");
    buttonInfoAll->setMinimumWidth(kButtonMinWidth);
    buttonInfoAll->setMinimumHeight(kButtonMinHeight);
    ButtonFont = buttonInfoAll->font();
    ButtonFont.setPointSize(kFontSize);
    buttonInfoAll->setFont(ButtonFont);
    connect(buttonInfoAll, &QPushButton::clicked, this, &LayoutTexturesManager::ViewMultiSelected);

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

    mainWindow->SetTitle("Texture Manager");
}

void LayoutTexturesManager::Startup()
{
    LockGui(true);

    mainWindow->statusBar()->showMessage("Detecting game data...");
    QApplication::processEvents();
    g_GameData->Init(mainWindow->gameType, configIni, true);
    if (!Misc::CheckGamePath())
    {
        QMessageBox::critical(this, "Texture Manager", "Game data not found.");
        mainWindow->statusBar()->clearMessage();
        buttonExit->setEnabled(true);
        mainWindow->LockClose(false);
        return;
    }

    if (mainWindow->gameType == MeType::ME3_TYPE && Misc::unpackSFARisNeeded())
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
    QString filename = path + QString("/me%1map.bin").arg(static_cast<int>(mainWindow->gameType));
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
        QApplication::processEvents();
    }
    else
    {
        bool modded = Misc::detectMod(mainWindow->gameType);
        if (!modded)
            modded = Misc::CheckForMarkers(&LayoutTexturesManager::ScanTexturesCallback, mainWindow);
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
    int exportIdVerify;
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
    bool removeEmptyMips = true;
    if (gameHasEmptyMips)
    {
        int result = QMessageBox::question(this, "Texture Manager",
                              QString("Replacing textures and creating mods requires generating a map of the game's textures.\n") +
                              "You only need to do it once.\n\n" +
                              "IMPORTANT! Your game needs to be in vanilla state and have optional DLC/PCC mods installed.\n\n" +
                              "You can continue or skip this step", "Continue", "Exit");
        if (result != 0)
        {
            buttonExit->setEnabled(true);
            mainWindow->LockClose(false);
            return;
        }
        int answer = QMessageBox::question(this, "Texture Manager",
                              QString("Detected empty mips in game files.") +
              "\n\nYou can remove empty mips or you can skip this step.", "Remove", "Skip");
        if (answer != 0)
            removeEmptyMips = false;
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

    mainWindow->statusBar()->showMessage("Preparing to scan textures...");
    QApplication::processEvents();
    resources.loadMD5Tables();
    g_logs->BufferClearErrors();
    g_logs->BufferEnableErrors(true);
    TreeScan::PrepareListOfTextures(mainWindow->gameType, resources,
                                    textures, removeEmptyMips, true,
                                    &LayoutTexturesManager::ScanTexturesCallback,
                                    mainWindow);
    g_logs->BufferEnableErrors(false);
    mainWindow->statusBar()->clearMessage();
    QApplication::processEvents();
    if (g_logs->BufferGetErrors() != "")
    {
        MessageWindow msg;
        msg.Show(mainWindow, "Errors while scanning package files", g_logs->BufferGetErrors());
    }

    QList<TreeItem> tree;
    int lastProgress = -1;
    for (int t = 0; t < textures.count(); t++)
    {
        if (t % 50 == 0)
            QApplication::processEvents();
        int newProgress = t * 100 / textures.count();
        if (lastProgress != newProgress)
        {
            lastProgress = newProgress;
            mainWindow->statusBar()->showMessage("Preparing tree view... Progress: " + QString::number(newProgress) + " %");
            QApplication::processEvents();
        }
        for (int l = 0; l < textures[t].list.count(); l++)
        {
            if (textures[t].list[l].path.length() == 0)
                continue;

            QString packageName = BaseNameWithoutExt(textures[t].list[l].path);
            bool found = false;
            for (int n = 0; n < tree.count(); n++)
            {
                auto item = tree[n];
                if (item.packageName == packageName)
                {
                    bool found2 = false;
                    for (int x = 0; x < item.list.count(); x++)
                    {
                        if (item.list[x].crc == textures[t].crc)
                        {
                            found2 = true;
                            break;
                        }
                    }
                    if (!found2)
                    {
                        ViewTexture texture;
                        texture.crc = textures[t].crc;
                        texture.name = textures[t].name;
                        item.list.append(texture);
                    }
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                ViewTexture texture;
                texture.crc = textures[t].crc;
                texture.name = textures[t].name;
                TreeItem item;
                item.packageName = packageName;
                item.list.append(texture);
                tree.append(item);
            }
        }
    }

    listLeft->setUpdatesEnabled(false);
    for (int n = 0; n < tree.count(); n++)
    {
        QList<ViewTexture> list;
        for (int l = 0; l < tree[n].list.count(); l++)
        {
            ViewTexture texture;
            texture.crc = tree[n].list[l].crc;
            texture.name = tree[n].list[l].name;
            list.append(texture);
        }
        auto item = new QListWidgetItem(tree[n].packageName);
        item->setData(Qt::UserRole, QVariant::fromValue<QList<ViewTexture>>(list));
        listLeft->addItem(item);
        item->setData(Qt::UserRole, QVariant::fromValue<QList<ViewTexture>>(list));
    }
    listLeft->sortItems();
    listLeft->setUpdatesEnabled(true);

    mainWindow->statusBar()->clearMessage();

    singlePackageMode = false;
    singleViewMode = true;
    imageViewMode = true;
    textureSelected = false;
    packageSelected = false;

    LockGui(false);
    UpdateGui();
}

void LayoutTexturesManager::ScanTexturesCallback(void *handle, int progress, const QString &stage)
{
    auto *win = static_cast<MainWindow *>(handle);
    win->statusBar()->showMessage(QString("Preparing... Stage: ") + stage +
                                  " -  Progress: " + QString::number(progress) + "%");
    QApplication::processEvents();
}

void LayoutTexturesManager::LockGui(bool lock)
{
    listLeft->setEnabled(!lock);
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
}

void LayoutTexturesManager::UpdateGui()
{
    buttonReplace->setEnabled(textureSelected);
    buttonReplaceConvert->setEnabled(textureSelected);
    buttonExtractToDDS->setEnabled(textureSelected);
    buttonExtractToPNG->setEnabled(textureSelected);
    if (imageViewMode)
    {
        rightView->setCurrentIndex(kWidgetImage);
        buttonViewImage->setEnabled(false);
        if (mainWindow->gameType != MeType::ME1_TYPE)
            buttonInfoSingle->setEnabled(packageSelected);
        buttonInfoAll->setEnabled(packageSelected);
        buttonPackageSingle->setEnabled(packageSelected);
        buttonPackageMulti->setEnabled(packageSelected);
    }
    else
    {
        if (singlePackageMode)
            rightView->setCurrentIndex(kWidgetImage);
        else
            rightView->setCurrentIndex(kWidgetText);
        buttonViewImage->setEnabled(packageSelected);
        buttonInfoSingle->setEnabled(!packageSelected);
        buttonInfoAll->setEnabled(!packageSelected);
        buttonPackageSingle->setEnabled(!packageSelected);
        buttonPackageMulti->setEnabled(!packageSelected);
    }
}

void LayoutTexturesManager::ReplaceSelected()
{
}

void LayoutTexturesManager::ReplaceConvertSelected()
{
}

void LayoutTexturesManager::ExtractDDSSelected()
{
}

void LayoutTexturesManager::ExtractPNGSelected()
{
}

void LayoutTexturesManager::ViewImageSelected()
{
}

void LayoutTexturesManager::ViewSingleSelected()
{
}

void LayoutTexturesManager::ViewMultiSelected()
{
}

void LayoutTexturesManager::PackageSingleSelected()
{
}

void LayoutTexturesManager::PackageMutiSelected()
{
}

void LayoutTexturesManager::SearchSelected()
{
}

void LayoutTexturesManager::ExitSelected()
{
    mainWindow->SwitchLayoutById(MainWindow::kLayoutModules);
    mainWindow->GetLayout()->removeWidget(this);
    mainWindow->SetTitle("Modules Selection");
}
