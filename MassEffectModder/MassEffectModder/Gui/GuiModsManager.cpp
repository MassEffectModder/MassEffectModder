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

#include <Gui/LayoutInstallMods.h>
#include <Gui/LayoutMain.h>
#include <Gui/MainWindow.h>
#include <Gui/MessageWindow.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <Wrappers.h>
#include <GameData/GameData.h>
#include <GameData/Package.h>
#include <Misc/Misc.h>
#include <MipMaps/MipMaps.h>

void LayoutMain::ExtractModCallback(void *handle, int progress, const QString & /*stage*/)
{
    auto *win = static_cast<MainWindow *>(handle);
    win->statusBar()->showMessage(QString("Extracting MEM files... Progress: ") + QString::number(progress) + "%");
    QApplication::processEvents();
}

void LayoutMain::ExtractModsSelected(MeType gameType)
{
    LockGui(true);

    QStringList files = QFileDialog::getOpenFileNames(this,
            "Please select MEM file(s)", "", "MEM file (*.mem)");
    if (files.count() == 0)
    {
        LockGui(false);
        return;
    }
    QString outDir = QFileDialog::getExistingDirectory(this,
            "Please select destination directory for MEM file(s) extraction");
    if (outDir == "")
    {
        LockGui(false);
        return;
    }
    outDir = QDir::cleanPath(outDir);
    QFileInfoList listMEM;
    quint64 diskFreeSpace = Misc::getDiskFreeSpace(outDir);
    quint64 diskUsage = 0;
    foreach (QString file, files)
    {
        auto info = QFileInfo(file);
        diskUsage += info.size();
        if (file.endsWith(".mem", Qt::CaseSensitivity::CaseInsensitive))
            listMEM.push_back(info);
    }
    diskUsage = (quint64)(diskUsage * 2.5);
    if (diskUsage >= diskFreeSpace)
    {
        QMessageBox::critical(this, "Extracting MEM file(s)",
                              "You have not enough disk space remaining. You need about " +
                              Misc::getBytesFormat(diskUsage) + " free disk space.");
        LockGui(false);
        return;
    }

    QList<TextureMapEntry> textures;
    Resources resources;
    resources.loadMD5Tables();
    TreeScan::loadTexturesMap(gameType, resources, textures);

    g_logs->BufferClearErrors();
    g_logs->BufferEnableErrors(true);
    mainWindow->statusBar()->clearMessage();
    if (!Misc::extractMEM(gameType, listMEM, outDir,
                     &LayoutMain::ExtractModCallback, mainWindow))
    {
        QMessageBox::critical(this, "Extracting MEM file(s)", "Extraction failed!");
    }
    mainWindow->statusBar()->clearMessage();
    g_logs->BufferEnableErrors(false);
    if (g_logs->BufferGetErrors() != "")
    {
        MessageWindow msg;
        msg.Show(mainWindow, "Extracting MEM file(s)", g_logs->BufferGetErrors());
    }
    else
    {
        QMessageBox::information(this, "Extracting MEM file(s)", "All files extracted.");
    }

    LockGui(false);
}

void LayoutMain::CreateModCallback(void *handle, int progress, const QString & /*stage*/)
{
    auto *win = static_cast<MainWindow *>(handle);
    win->statusBar()->showMessage(QString("Creating MEM mod... Progress: ") + QString::number(progress) + "%");
    QApplication::processEvents();
}

void LayoutMain::CreateModSelected(MeType gameType)
{
    LockGui(true);

    QString inputDir = QFileDialog::getExistingDirectory(this,
            "Please select source directory");
    if (inputDir == "")
    {
        LockGui(false);
        return;
    }
    inputDir = QDir::cleanPath(inputDir);

    QString modFile = QFileDialog::getSaveFileName(this,
            "Please select new MEM mod file", "", "MEM mod file (*.mem)");
    if (modFile == "")
    {
        LockGui(false);
        return;
    }

    QFileInfoList list;
    QFileInfoList list2;
    list = QDir(inputDir, "*.mem", QDir::SortFlag::IgnoreCase | QDir::SortFlag::Name, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    list2 += QDir(inputDir, "*.dds", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    list2 += QDir(inputDir, "*.png", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    list2 += QDir(inputDir, "*.bmp", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    list2 += QDir(inputDir, "*.tga", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    list2 += QDir(inputDir, "*.bik", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    std::sort(list2.begin(), list2.end(), Misc::compareFileInfoPath);
    list.append(list2);

    QString outDir = DirName(modFile);
    quint64 diskFreeSpace = Misc::getDiskFreeSpace(outDir);
    quint64 diskUsage = 0;
    foreach (QFileInfo info, list)
    {
        diskUsage += info.size();
    }
    diskUsage = (quint64)(diskUsage / 1.5);
    if (diskUsage >= diskFreeSpace)
    {
        QMessageBox::critical(this, "Creating MEM mod",
                              "You have not enough disk space remaining. You need about " +
                              Misc::getBytesFormat(diskUsage) + " free disk space.");
        LockGui(false);
        return;
    }

    g_logs->BufferClearErrors();
    g_logs->BufferEnableErrors(true);

    QList<TextureMapEntry> textures;
    Resources resources;
    resources.loadMD5Tables();
    TreeScan::loadTexturesMap(gameType, resources, textures);
    if (!Misc::convertDataModtoMem(list, modFile, gameType, textures, false, false, false, 0.2f,
                              &LayoutMain::CreateModCallback, mainWindow))
    {
        QMessageBox::critical(this, "Creating MEM mod", "Creating MEM mod failed!");
    }
    mainWindow->statusBar()->clearMessage();
    g_logs->BufferEnableErrors(false);
    if (g_logs->BufferGetErrors() != "")
    {
        MessageWindow msg;
        msg.Show(mainWindow, "Creating MEM mod", g_logs->BufferGetErrors());
    }
    else
    {
        QMessageBox::information(this, "Creating MEM mod", "Mod created.");
    }
    LockGui(false);
}
