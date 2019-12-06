/*
 * MassEffectModder
 *
 * Copyright (C) 2018-2019 Pawel Kolodziejski
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

#include <Helpers/FileStream.h>
#include <Helpers/MiscHelpers.h>
#include <GameData/TOCFile.h>
#include <GameData/GameData.h>

static bool compareFullPath(const QString &e1, const QString &e2)
{
    return e1.compare(e2, Qt::CaseInsensitive) < 0;
}

void TOCBinFile::UpdateAllTOCBinFiles()
{
    GenerateMainTocBinFile();
    GenerateDLCsTocBinFiles();
}

void TOCBinFile::GenerateMainTocBinFile()
{
    QStringList files;
    int pathLen = g_GameData->GamePath().length();
    QDirIterator MainIterator(g_GameData->MainData(), QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (MainIterator.hasNext())
    {
#ifdef GUI
        QApplication::processEvents();
#endif
        MainIterator.next();
        if (MainIterator.filePath().endsWith(".pcc", Qt::CaseInsensitive) ||
            MainIterator.filePath().endsWith(".upk", Qt::CaseInsensitive) ||
            MainIterator.filePath().endsWith(".tfc", Qt::CaseInsensitive) ||
            MainIterator.filePath().endsWith(".tlk", Qt::CaseInsensitive) ||
            MainIterator.filePath().endsWith(".afc", Qt::CaseInsensitive) ||
            MainIterator.filePath().endsWith(".cnd", Qt::CaseInsensitive) ||
            MainIterator.filePath().endsWith(".txt", Qt::CaseInsensitive) ||
            MainIterator.filePath().endsWith(".bin", Qt::CaseInsensitive))
        {
            files.push_back(MainIterator.filePath().mid(pathLen + 1));
        }
    }

    QDirIterator MoviesIterator(g_GameData->bioGamePath() + "/Movies", QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (MoviesIterator.hasNext())
    {
        MoviesIterator.next();
        if (MoviesIterator.filePath().endsWith(".bik", Qt::CaseInsensitive))
            files.push_back(MoviesIterator.filePath().mid(pathLen + 1));
    }

    std::sort(files.begin(), files.end(), compareFullPath);

    QList<FileEntry> filesList;
    for (int f = 0; f < files.count(); f++)
    {
        FileEntry file{};
        if (!files[f].endsWith("pcconsoletoc.bin", Qt::CaseInsensitive))
            file.size = QFileInfo(g_GameData->GamePath() + "/" + files[f]).size();
        file.path = files[f].replace(QChar('/'), QChar('\\'), Qt::CaseInsensitive);
        filesList.push_back(file);
    }
    QString tocFile = g_GameData->bioGamePath() + "/PCConsoleTOC.bin";
    CreateTocBinFile(tocFile, filesList);
}

void TOCBinFile::GenerateDLCsTocBinFiles()
{
    if (QDir(g_GameData->DLCData()).exists())
    {
        int pathLen = g_GameData->DLCData().length();
        QStringList DLCs = QDir(g_GameData->DLCData(), "DLC_*", QDir::NoSort, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryList();
        foreach (QString DLCDir, DLCs)
        {
            QStringList files;
            QDirIterator iterator(g_GameData->DLCData() + "/" + DLCDir, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
            bool isValid = false;
            while (iterator.hasNext())
            {
                iterator.next();
                if (iterator.filePath().endsWith("Mount.dlc", Qt::CaseInsensitive))
                {
                    isValid = true;
                }
                if (iterator.filePath().endsWith(".pcc", Qt::CaseInsensitive) ||
                    iterator.filePath().endsWith(".upk", Qt::CaseInsensitive) ||
                    iterator.filePath().endsWith(".tfc", Qt::CaseInsensitive) ||
                    iterator.filePath().endsWith(".tlk", Qt::CaseInsensitive) ||
                    iterator.filePath().endsWith(".afc", Qt::CaseInsensitive) ||
                    iterator.filePath().endsWith(".cnd", Qt::CaseInsensitive) ||
                    iterator.filePath().endsWith(".bik", Qt::CaseInsensitive) ||
                    iterator.filePath().endsWith(".txt", Qt::CaseInsensitive) ||
                    iterator.filePath().endsWith(".bin", Qt::CaseInsensitive))
                {
                    files.push_back(iterator.filePath().mid(pathLen + DLCDir.length() + 2));
                }
            }
            if (!isValid)
            {
                continue;
            }

            std::sort(files.begin(), files.end(), compareFullPath);

            QList<FileEntry> filesList;
            for (int f = 0; f < files.count(); f++)
            {
                FileEntry file{};
                if (!files[f].endsWith("pcconsoletoc.bin", Qt::CaseInsensitive))
                    file.size = QFileInfo(g_GameData->DLCData() + "/" + DLCDir + "/" + files[f]).size();
                file.path = files[f].replace(QChar('/'), QChar('\\'));
                filesList.push_back(file);
            }
            QString tocFile = g_GameData->DLCData() + "/" + DLCDir + "/PCConsoleTOC.bin";
            CreateTocBinFile(tocFile, filesList);
        }
    }
}

void TOCBinFile::CreateTocBinFile(QString &path, QList<FileEntry> filesList)
{
    if (QFile(path).exists())
        QFile::remove(path);

    FileStream tocFile = FileStream(path, FileMode::Create, FileAccess::WriteOnly);
    tocFile.WriteUInt32(TOCTag);
    tocFile.WriteUInt32(0);
    tocFile.WriteUInt32(1);
    tocFile.WriteUInt32(8);
    tocFile.WriteInt32(filesList.count());

    long lastOffset = 0;
    for (int f = 0; f < filesList.count(); f++)
    {
        long fileOffset = lastOffset = tocFile.Position();
        int blockSize = ((28 + (filesList[f].path.length() + 1) + 3) / 4) * 4; // align to 4
        tocFile.WriteUInt16((ushort)blockSize);
        tocFile.WriteUInt16(0);
        tocFile.WriteUInt32(filesList[f].size);
        tocFile.WriteZeros(20);
        tocFile.WriteStringASCIINull(filesList[f].path);
        tocFile.JumpTo(fileOffset + blockSize - 1);
        tocFile.WriteByte(0); // make sure all bytes are written after seek
    }
    if (lastOffset != 0)
    {
        tocFile.JumpTo(lastOffset);
        tocFile.WriteUInt16(0);
    }
}
