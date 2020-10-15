/*
 * MassEffectModder
 *
 * Copyright (C) 2019-2020 Pawel Kolodziejski
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
            "Please select MEM/TPF/MOD file(s)", "", "MEM/TPF/MOD  file (*.mem *.tpf *.mod)");
    if (files.count() == 0)
    {
        LockGui(false);
        return;
    }
    QString outDir = QFileDialog::getExistingDirectory(this,
            "Please select destination directory for MEM/TPF/MOD file(s) extraction");
    if (outDir == "")
    {
        LockGui(false);
        return;
    }
    outDir = QDir::cleanPath(outDir);
    QFileInfoList listMEM, listTPF, listMOD;
    quint64 diskFreeSpace = Misc::getDiskFreeSpace(outDir);
    quint64 diskUsage = 0;
    foreach (QString file, files)
    {
        auto info = QFileInfo(file);
        diskUsage += info.size();
        if (file.endsWith(".mem", Qt::CaseSensitivity::CaseInsensitive))
            listMEM.push_back(info);
        else if (file.endsWith(".tpf", Qt::CaseSensitivity::CaseInsensitive))
            listTPF.push_back(info);
        else if (file.endsWith(".mod", Qt::CaseSensitivity::CaseInsensitive))
            listMOD.push_back(info);
    }
    diskUsage = (quint64)(diskUsage * 2.5);
    if (diskUsage >= diskFreeSpace)
    {
        QMessageBox::critical(this, "Extracting MEM/TPF/MEM file(s)",
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
    mainWindow->statusBar()->showMessage(QString("Extracting TPF file(s)..."));
    if (!Misc::extractTPF(listTPF, outDir))
    {
        QMessageBox::critical(this, "Extracting TPF file(s)", "Extraction failed!");
    }
    mainWindow->statusBar()->showMessage(QString("Extracting MOD file(s)..."));
    if (!Misc::extractMOD(listMOD, textures, outDir))
    {
        QMessageBox::critical(this, "Extracting MOD file(s)", "Extraction failed!");
    }
    mainWindow->statusBar()->clearMessage();
    g_logs->BufferEnableErrors(false);
    if (g_logs->BufferGetErrors() != "")
    {
        MessageWindow msg;
        msg.Show(mainWindow, "Extracting MEM/TPF/MOD file(s)", g_logs->BufferGetErrors());
    }
    else
    {
        QMessageBox::information(this, "Extracting MEM/TPF/MOD file(s)", "All files extracted.");
    }

    LockGui(false);
}

void LayoutMain::ConvertModCallback(void *handle, int progress, const QString & /*stage*/)
{
    auto *win = static_cast<MainWindow *>(handle);
    win->statusBar()->showMessage(QString("Converting to MEM mod... Progress: ") + QString::number(progress) + "%");
    QApplication::processEvents();
}

void LayoutMain::ConvertModSelected(MeType gameType)
{
    LockGui(true);

    QString path;
    QFileDialog dialog = QFileDialog(this, "Please select mod file to convert",
                                     "", "Mod file (*.mod *.tpf)");
    dialog.setFileMode(QFileDialog::ExistingFile);
    if (dialog.exec())
    {
        path = dialog.selectedFiles().first();
    }
    if (path.length() == 0 || !QFile(path).exists())
    {
        LockGui(false);
        return;
    }

    QString modFile = QFileDialog::getSaveFileName(this,
            "Please select new MEM mod file", "", "MEM mod file (*.mem)");
    if (modFile == "")
    {
        LockGui(false);
        return;
    }

    QFileInfoList file;
    file.append(QFileInfo(path));

    QList<TextureMapEntry> textures;
    Resources resources;
    resources.loadMD5Tables();

    g_logs->BufferClearErrors();
    g_logs->BufferEnableErrors(true);

    TreeScan::loadTexturesMap(gameType, resources, textures);
    if (!Misc::convertDataModtoMem(file, modFile, gameType, textures, false,
                              &LayoutMain::ConvertModCallback, mainWindow))
    {
        QMessageBox::critical(this, "Converting to MEM mod", "Conversion failed!");
    }
    mainWindow->statusBar()->clearMessage();

    g_logs->BufferEnableErrors(false);
    if (g_logs->BufferGetErrors() != "")
    {
        MessageWindow msg;
        msg.Show(mainWindow, "Converting to MEM mod", g_logs->BufferGetErrors());
    }
    else
    {
        QMessageBox::information(this, "Converting to MEM mod", "Mod converted.");
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
    list2 = QDir(inputDir, "*.tpf", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    list2 += QDir(inputDir, "*.mod", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    list2 += QDir(inputDir, "*.bin", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    list2 += QDir(inputDir, "*.xdelta", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
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
    if (!Misc::convertDataModtoMem(list, modFile, gameType, textures, false,
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

void LayoutMain::CreateBinaryModSelected(MeType gameType)
{
    LockGui(true);

    ConfigIni configIni{};
    g_GameData->Init(gameType, configIni, true);
    if (g_GameData->GamePath().length() == 0 || !QDir(g_GameData->GamePath()).exists())
    {
        QMessageBox::critical(this, "Creating Binary mod files", "Game data not found.");
        LockGui(false);
        return;
    }

    QString modsDir = QFileDialog::getExistingDirectory(this,
            "Please select source game directory of modded package files");
    if (modsDir == "")
    {
        LockGui(false);
        return;
    }
    modsDir = QDir::cleanPath(modsDir);

    QStringList packagesList;
    QDirIterator iterator2(modsDir, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (iterator2.hasNext())
    {
        QApplication::processEvents();
        iterator2.next();
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
        QApplication::processEvents();
        FileStream fs = FileStream(packagesList[i], FileMode::Open, FileAccess::ReadOnly);
        fs.SeekEnd();
        fs.Seek(-MEMMarkerLength, SeekOrigin::Current);
        QString marker;
        fs.ReadStringASCII(marker, MEMMarkerLength);
        if (marker == QString(MEMendFileMarker))
        {
            mainWindow->statusBar()->clearMessage();
            QMessageBox::critical(this, "Creating Binary mod files",
                                  "Mod files must be based on vanilla game data, aborting...");
            LockGui(false);
            return;
        }
    }

    QList<BinaryMod> modFiles;
    for (int i = 0; i < packagesList.count(); i++)
    {
        mainWindow->statusBar()->showMessage(QString("Scanning mods...     Total progress - " +
                                                     QString::number(i * 100 / packagesList.count())) + " %");
        QApplication::processEvents();

        if (packagesList[i].endsWith("localshadercache-pc-d3d-sm3.upk", Qt::CaseInsensitive))
            continue;
        if (packagesList[i].endsWith("refshadercache-pc-d3d-sm3.upk", Qt::CaseInsensitive))
            continue;
        if (packagesList[i].contains("guidcache", Qt::CaseInsensitive))
            continue;

        Package vanillaPkg;
        Package modPkg;
        bool vanilla = false;
        bool found = false;
        QString basename = BaseName(packagesList[i]);
        for (int v = 0; v < g_GameData->packageFiles.count(); v++)
        {
            QApplication::processEvents();
            QString basename2 = BaseName(g_GameData->packageFiles[v]);
            if (AsciiStringMatchCaseIgnore(basename, basename2))
            {
                vanilla = true;
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

        mainWindow->statusBar()->showMessage(QString("Comparing package... Total progress - " +
                                                     QString::number(i * 100 / packagesList.count())) + " % - " +
                                             vanillaPkg.packagePath);
        for (int e = 0; e < modPkg.exportsTable.count(); e++)
        {
            QApplication::processEvents();
            auto vanillaExport = vanillaPkg.getExportData(e);
            auto modExport = modPkg.getExportData(e);
            if (vanillaExport.size() == modExport.size())
            {
                if (memcmp(vanillaExport.ptr(), modExport.ptr(), vanillaExport.size()) == 0)
                    continue;
            }
            auto className = vanillaPkg.getClassName(vanillaPkg.exportsTable[e].getClassId());
            if (className == "ShaderCache" ||
                className == "WwiseStream" ||
                className == "WwiseBank" ||
                className == "Texture2D" ||
                className == "LightMapTexture2D" ||
                className == "ShadowMapTexture2D" ||
                className == "TextureFlipBook" ||
                className == "TextureMovie")
            {
                continue;
            }

            BinaryMod mod;
            mod.packagePath = vanillaPkg.packagePath;
            mod.exportId = e;

            if (vanillaExport.size() == modExport.size())
            {
                auto delta = ByteBuffer(vanillaExport.size());
                unsigned int deltaSize = 0;
                if (XDelta3Compress(vanillaExport.ptr(), modExport.ptr(), vanillaExport.size(),
                                    delta.ptr(), &deltaSize) != 0)
                {
                    CRASH();
                }
                mod.data = ByteBuffer(delta.ptr(), deltaSize);
                delta.Free();
                mod.binaryModType = 2;
            }
            else
            {
                mod.data = ByteBuffer(modExport.ptr(), modExport.size());
                mod.binaryModType = 1;
            }
            vanillaExport.Free();
            modExport.Free();

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

    QString modFile = QFileDialog::getSaveFileName(this,
            "Please select new MEM mod file", "", "MEM mod file (*.mem)");
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
        Misc::compressData(modFiles[i].data, *dst);
        BinaryMod bmod = modFiles[i];
        bmod.offset = outFs.Position();
        bmod.size = dst->Length();
        modFiles[i] = bmod;
        outFs.WriteInt32(modFiles[i].exportId);
        outFs.WriteStringASCIINull(modFiles[i].packagePath.replace('/', '\\'));
        dst->SeekBegin();
        outFs.CopyFrom(*dst, dst->Length());
    }

    long pos = outFs.Position();
    outFs.SeekBegin();
    outFs.WriteUInt32(TextureModTag);
    outFs.WriteUInt32(TextureModVersion);
    outFs.WriteInt64(pos);
    outFs.JumpTo(pos);
    outFs.WriteUInt32((uint)gameType);
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

    mainWindow->statusBar()->clearMessage();
    QMessageBox::information(this, "Creating Binary mod files", "Finished.");
    LockGui(false);
}
