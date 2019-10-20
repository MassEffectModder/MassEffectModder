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

#include "Gui/LayoutMeSelect.h"
#include "Gui/LayoutModsManager.h"
#include "Gui/MainWindow.h"
#include "Gui/MessageWindow.h"
#include "Helpers/MiscHelpers.h"
#include "Helpers/Logs.h"
#include "Wrappers.h"
#include "GameData.h"
#include "Misc.h"
#include "MipMaps.h"
#include "Package.h"

LayoutModsManager::LayoutModsManager(MainWindow *window)
    : mainWindow(window)
{
    if (window == nullptr)
        CRASH();

    layoutId = MainWindow::kLayoutModsManager;

    auto ButtonInstallMods = new QPushButton("Install Mods");
    ButtonInstallMods->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonInstallMods->setMinimumWidth(kButtonMinWidth);
    ButtonInstallMods->setMinimumHeight(kButtonMinHeight);
    QFont ButtonFont = ButtonInstallMods->font();
    ButtonFont.setPointSize(kFontSize);
    ButtonInstallMods->setFont(ButtonFont);
    connect(ButtonInstallMods, &QPushButton::clicked, this, &LayoutModsManager::InstallModsSelected);

    auto ButtonExtractMods = new QPushButton("Extract Mods");
    ButtonExtractMods->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonExtractMods->setMinimumWidth(kButtonMinWidth);
    ButtonExtractMods->setMinimumHeight(kButtonMinHeight);
    ButtonExtractMods->setFont(ButtonFont);
    connect(ButtonExtractMods, &QPushButton::clicked, this, &LayoutModsManager::ExtractModsSelected);

    auto ButtonCreateMod = new QPushButton("Create Mod");
    ButtonCreateMod->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonCreateMod->setMinimumWidth(kButtonMinWidth);
    ButtonCreateMod->setMinimumHeight(kButtonMinHeight);
    ButtonCreateMod->setFont(ButtonFont);
    connect(ButtonCreateMod, &QPushButton::clicked, this, &LayoutModsManager::CreateModSelected);

    auto ButtonCreateBinaryMod = new QPushButton("Create Binary Mod");
    ButtonCreateBinaryMod->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonCreateBinaryMod->setMinimumWidth(kButtonMinWidth);
    ButtonCreateBinaryMod->setMinimumHeight(kButtonMinHeight);
    ButtonCreateBinaryMod->setFont(ButtonFont);
    connect(ButtonCreateBinaryMod, &QPushButton::clicked, this, &LayoutModsManager::CreateBinaryModSelected);

    auto ButtonReturn = new QPushButton("Return");
    ButtonReturn->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    ButtonReturn->setMinimumWidth(kButtonMinWidth);
    ButtonReturn->setMinimumHeight(kButtonMinHeight / 2);
    ButtonReturn->setFont(ButtonFont);
    connect(ButtonReturn, &QPushButton::clicked, this, &LayoutModsManager::ReturnSelected);

    auto *horizontalLayout = new QHBoxLayout(this);
    horizontalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 40));
    auto *verticalLayout = new QVBoxLayout();
    verticalLayout->setAlignment(Qt::AlignVCenter);
    verticalLayout->addWidget(ButtonInstallMods, 1);
    verticalLayout->addWidget(ButtonExtractMods, 1);
    verticalLayout->addWidget(ButtonCreateMod, 1);
    verticalLayout->addWidget(ButtonCreateBinaryMod, 1);
    verticalLayout->addSpacing(20);
    verticalLayout->addWidget(ButtonReturn, 1);
    horizontalLayout->addLayout(verticalLayout);
    horizontalLayout->addSpacing(PERCENT_OF_SIZE(MainWindow::kMinWindowWidth, 40));
}

void LayoutModsManager::LockGui(bool enable)
{
    foreach (QWidget *widget, this->findChildren<QWidget*>())
    {
        widget->setEnabled(!enable);
    }
    mainWindow->LockClose(enable);
}

void LayoutModsManager::InstallModsSelected()
{
}

void LayoutModsManager::ExtractMEMCallback(void *handle, int progress)
{
    auto *win = static_cast<MainWindow *>(handle);
    win->statusBar()->showMessage(QString("Extracting MEM files... Progress: ") + QString::number(progress) + "%");
    QApplication::processEvents();
}

void LayoutModsManager::ExtractModsSelected()
{
    LockGui(true);
    QStringList files = QFileDialog::getOpenFileNames(this,
            "Please select Mod file", "", "MEM mod file (*.mem)");
    if (files.count() == 0)
    {
        LockGui(false);
        return;
    }
    QString outDir = QFileDialog::getExistingDirectory(this,
            "Please select destination directory for MEM extraction");
    if (outDir == "")
    {
        LockGui(false);
        return;
    }
    outDir = QDir::cleanPath(outDir);
    QFileInfoList list;
    long diskFreeSpace = Misc::getDiskFreeSpace(outDir);
    long diskUsage = 0;
    foreach (QString file, files)
    {
        auto info = QFileInfo(file);
        diskUsage += info.size();
        list.push_back(info);
    }
    diskUsage = (long)(diskUsage * 2.5);
    if (diskUsage >= diskFreeSpace)
    {
        QMessageBox::critical(this, "Extracting MEM file(s)",
                              "You have not enough disk space remaining. You need about " +
                              Misc::getBytesFormat(diskUsage) + " free disk space.");
        LockGui(false);
        return;
    }

    g_logs->BufferClearErrors();
    g_logs->BufferEnableErrors(true);
    mainWindow->statusBar()->clearMessage();
    Misc::extractMEM(mainWindow->gameType, list, outDir,
                     &LayoutModsManager::ExtractMEMCallback, mainWindow);
    mainWindow->statusBar()->clearMessage();
    g_logs->BufferEnableErrors(false);
    if (g_logs->BufferGetErrors() != "")
    {
        MessageWindow msg;
        msg.Show("Extracting MEM file(s)", g_logs->BufferGetErrors());
    }
    else
    {
        QMessageBox::information(this, "Checking game", "All game files checked.");
    }

    LockGui(false);
}

void LayoutModsManager::CreateModSelected()
{
}

void LayoutModsManager::CreateBinaryModSelected()
{
    LockGui(true);

    ConfigIni configIni{};
    g_GameData->Init(mainWindow->gameType, configIni);
    if (g_GameData->GamePath().length() == 0 || !QDir(g_GameData->GamePath()).exists())
    {
        QMessageBox::critical(this, "Creating Binary mod files", "Game data not found.");
        LockGui(false);
        return;
    }

    QString modsDir = QFileDialog::getExistingDirectory(this,
            "Please select source directory of modded package files");
    if (modsDir == "")
    {
        LockGui(false);
        return;
    }
    modsDir = QDir::cleanPath(modsDir);

    mainWindow->statusBar()->showMessage(QString("Checking source intergrity..."));
    bool foundExe = false;
    QDirIterator iterator(modsDir, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (iterator.hasNext())
    {
        QApplication::processEvents();
        iterator.next();
        if (iterator.filePath().endsWith(".exe", Qt::CaseInsensitive))
        {
            foundExe = true;
            break;
        }
    }
    if (!foundExe)
    {
        mainWindow->statusBar()->clearMessage();
        QMessageBox::critical(this, "Creating Binary mod files",
                              "The source directory doesn't seems right, aborting...");
        LockGui(false);
        return;
    }

    QStringList packagesList;
    QDirIterator iterator2(modsDir, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (iterator2.hasNext())
    {
        QApplication::processEvents();
        iterator.next();
        if (iterator2.filePath().endsWith(".upk", Qt::CaseInsensitive) ||
            iterator2.filePath().endsWith(".u", Qt::CaseInsensitive) ||
            iterator2.filePath().endsWith(".sfm", Qt::CaseInsensitive) ||
            iterator2.filePath().endsWith(".pcc", Qt::CaseInsensitive))
        {
            packagesList.append(iterator2.filePath());
        }
    }
    for (int i = 0; i < packagesList.count(); i++)
    {
        FileStream fs = FileStream(packagesList[i], FileMode::Open, FileAccess::ReadOnly);
        fs.SeekEnd();
        fs.Seek(-MEMMarkerLenght, SeekOrigin::Current);
        QString marker;
        fs.ReadStringASCII(marker, MEMMarkerLenght);
        if (marker == QString(MEMendFileMarker))
        {
            mainWindow->statusBar()->clearMessage();
            QMessageBox::critical(this, "Creating Binary mod files",
                                  "Mod files must be based on vanilla game data, aborting...");
            LockGui(false);
            return;
        }
    }

    mainWindow->statusBar()->showMessage(QString("Scanning mods..."));
    QList<BinaryMod> modFiles;
    for (int i = 0; i < packagesList.count(); i++)
    {
        Package vanillaPkg;
        Package modPkg;
        bool vanilla = true;
        bool found = false;
        for (int v = 0; v < g_GameData->packageFiles.count(); v++)
        {
            if (BaseName(packagesList[i]).compare(BaseName(g_GameData->packageFiles[v]), Qt::CaseInsensitive) == 0)
            {
                modPkg.Open(packagesList[i]);
                vanillaPkg.Open(g_GameData->GamePath() + g_GameData->packageFiles[v]);
                if (modPkg.exportsTable.count() != vanillaPkg.exportsTable.count() ||
                    modPkg.namesTable.count() != vanillaPkg.namesTable.count() ||
                    modPkg.importsTable.count() != vanillaPkg.importsTable.count())
                {
                    found = true;
                    vanilla = false;
                    continue;
                }
                found = true;
                break;
            }
        }
        if (found && !vanilla)
        {
            mainWindow->statusBar()->clearMessage();
            QMessageBox::critical(this, "Creating Binary mod files",
                                  "Package file is not compatible: " + packagesList[i] + ", aborting...");
            LockGui(false);
            return;
        }
        if (!found)
        {
            mainWindow->statusBar()->clearMessage();
            QMessageBox::critical(this, "Creating Binary mod files",
                                  "Package is not present in vanilla game data: " + packagesList[i] + ", aborting...");
            LockGui(false);
            return;
        }

        mainWindow->statusBar()->showMessage(QString("Creating mods..."));
        for (int e = 0; e < modPkg.exportsTable.count(); e++)
        {
            auto vanillaExport = vanillaPkg.getExportData(e);
            auto modExport = modPkg.getExportData(e);
            if (vanillaExport.size() == modExport.size())
            {
                if (memcmp(vanillaExport.ptr(), modExport.ptr(), vanillaExport.size()) == 0)
                    continue;
            }

            BinaryMod mod;
            mod.packagePath = vanillaPkg.packagePath;
            mod.exportId = e;

            if (vanillaExport.size() == modExport.size())
            {
                unsigned char *delta;
                unsigned int deltaSize;
                XDelta3Compress(vanillaExport.ptr(), modExport.ptr(), vanillaExport.size(),
                                delta, &deltaSize);
                mod.data = ByteBuffer(delta, deltaSize);
                mod.binaryModType = 2;
            }
            else
            {
                mod.data = ByteBuffer(modExport.ptr(), modExport.size());
                mod.binaryModType = 1;
            }

            QString name;
            if (mod.packagePath.contains("/DLC/", Qt::CaseInsensitive))
            {
                QString dlcName = mod.packagePath.split(QChar('/'))[3];
                name = "D" + QString::number(dlcName.size()) + "-" + dlcName + "-";
            }
            else
            {
                name = "B";
            }
            name += QString::number(BaseName(mod.packagePath).size()) +
                    "-" + BaseName(mod.packagePath) +
                    "-E" + QString::number(mod.exportId);

            if (mod.binaryModType == 1)
                name += ".bin";
            else if (mod.binaryModType == 2)
                name += ".xdelta";

            mod.textureName = name;
            modFiles.append(mod);
        }

        if (modFiles.count() == 0)
        {
            mainWindow->statusBar()->clearMessage();
            QMessageBox::critical(this, "Creating Binary mod files",
                                  "Nothing to mod, exiting...");
            LockGui(false);
            return;
        }

        mainWindow->statusBar()->showMessage(QString("Creating mem mod file..."));

        QString modFile = QFileDialog::getOpenFileName(this,
                "Please selecct new MEM mod file", "", "MEM mod file (*.mem)");
        if (modFile == "")
        {
            mainWindow->statusBar()->clearMessage();
            LockGui(false);
            return;
        }

        if (QFile::exists(modFile))
            QFile::remove(modFile);

        FileStream outFs = FileStream(modFile, FileMode::Create, FileAccess::WriteOnly);
        outFs.WriteUInt32(TextureModTag);
        outFs.WriteUInt32(TextureModVersion);
        outFs.WriteInt64(0); // filled later

        for (int i = 0; i < modFiles.count(); i++)
        {
            std::unique_ptr<Stream> dst (new MemoryStream());
            MipMaps::compressData(modFiles[i].data, *dst);
            BinaryMod bmod = modFiles[i];
            bmod.offset = outFs.Position();
            bmod.size = dst->Length();
            modFiles[i] = bmod;
            outFs.WriteInt32(modFiles[i].exportId);
            outFs.WriteStringASCIINull(modFiles[i].packagePath);
            outFs.CopyFrom(*dst, dst->Length());
        }

        long pos = outFs.Position();
        outFs.SeekBegin();
        outFs.WriteUInt32(TextureModTag);
        outFs.WriteUInt32(TextureModVersion);
        outFs.WriteInt64(pos);
        outFs.JumpTo(pos);
        outFs.WriteUInt32((uint)mainWindow->gameType);
        outFs.WriteInt32(modFiles.count());

        for (int i = 0; i < modFiles.count(); i++)
        {
            if (modFiles[i].binaryModType == 1)
                outFs.WriteUInt32(FileBinaryTag);
            else if (modFiles[i].binaryModType == 2)
                outFs.WriteUInt32(FileXdeltaTag);
            outFs.WriteStringASCIINull(modFiles[i].textureName);
            outFs.WriteInt64(modFiles[i].offset);
            outFs.WriteInt64(modFiles[i].size);
        }
    }

    mainWindow->statusBar()->clearMessage();
    QMessageBox::information(this, "Creating Binary mod files", "Finished.");
    LockGui(false);
}

void LayoutModsManager::ReturnSelected()
{
    mainWindow->SwitchLayoutById(MainWindow::kLayoutModules);
    mainWindow->GetLayout()->removeWidget(this);
}
