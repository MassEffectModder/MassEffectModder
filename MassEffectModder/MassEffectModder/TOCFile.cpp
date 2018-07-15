/*
 * MassEffectModder
 *
 * Copyright (C) 2018 Pawel Kolodziejski <aquadran at users.sourceforge.net>
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

#include "Helpers/FileStream.h"
#include "Helpers/MiscHelpers.h"

#include "TOCFile.h"
#include "GameData.h"

void TOCBinFile::UpdateAllTOCBinFiles()
{
    GenerateMainTocBinFile();
    GenerateDLCsTocBinFiles();
}

void TOCBinFile::GenerateMainTocBinFile()
{
    QStringList MainFiles;
    int pathLen = g_GameData->GamePath().length();
    QDirIterator MainIterator(g_GameData->MainData(), QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (MainIterator.hasNext())
    {
        MainIterator.next();
        MainFiles.push_back(MainIterator.filePath().mid(pathLen));
    }

    QStringList MoviesFiles;
    QDirIterator MoviesIterator(g_GameData->bioGamePath() + "/Movies", QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (MoviesIterator.hasNext())
    {
        MoviesIterator.next();
        MoviesFiles.push_back(MoviesIterator.filePath().mid(pathLen));
    }

    QStringList files = MainFiles.filter(QRegExp("*.pcc", Qt::CaseInsensitive, QRegExp::Wildcard));
    files += MainFiles.filter(QRegExp("*.upk", Qt::CaseInsensitive, QRegExp::Wildcard));
    files += MainFiles.filter(QRegExp("*.tfc", Qt::CaseInsensitive, QRegExp::Wildcard));
    files += MainFiles.filter(QRegExp("*.tlk", Qt::CaseInsensitive, QRegExp::Wildcard));
    files += MainFiles.filter(QRegExp("*.afc", Qt::CaseInsensitive, QRegExp::Wildcard));
    files += MainFiles.filter(QRegExp("*.cnd", Qt::CaseInsensitive, QRegExp::Wildcard));
    files += MainFiles.filter(QRegExp("*.txt", Qt::CaseInsensitive, QRegExp::Wildcard));
    files += MainFiles.filter(QRegExp("*.bin", Qt::CaseInsensitive, QRegExp::Wildcard));
    files += MoviesFiles.filter(QRegExp("*.bik", Qt::CaseInsensitive, QRegExp::Wildcard));

    QList<FileEntry> filesList;
    for (int f = 0; f < files.count(); f++)
    {
        FileEntry file{};
        if (!files[f].contains("pcconsoletoc.bin", Qt::CaseInsensitive))
            file.size = QFileInfo(files[f]).size();
        file.path = files[f].mid(pathLen + 1).replace(QChar('/'), QChar('\\'), Qt::CaseInsensitive);
        filesList.push_back(file);
    }
    QString tocFile = g_GameData->bioGamePath() + "/PCConsoleTOC.bin";
    CreateTocBinFile(tocFile, filesList);
}

void TOCBinFile::GenerateDLCsTocBinFiles()
{
    if (QDir(g_GameData->DLCData()).exists())
    {
        int pathLen = g_GameData->GamePath().length();
        QStringList DLCs = QDir(g_GameData->DLCData(), "DLC_*", QDir::NoSort, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryList();
        foreach (QString DLCDir, DLCs)
        {
            QStringList DLCfiles;
            QDirIterator iterator(g_GameData->DLCData() + "/" + DLCDir, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
            bool isValid = false;
            while (iterator.hasNext())
            {
                iterator.next();
                if (iterator.filePath().contains("Mount.dlc", Qt::CaseInsensitive))
                {
                    isValid = true;
                    break;
                }
                DLCfiles.push_back(iterator.filePath().mid(pathLen));
            }
            if (!isValid)
            {
                DLCs.removeOne(DLCDir);
                continue;
            }
            QStringList files = DLCfiles.filter(QRegExp("*.pcc", Qt::CaseInsensitive, QRegExp::Wildcard));
            files += DLCfiles.filter(QRegExp("*.upk", Qt::CaseInsensitive, QRegExp::Wildcard));
            files += DLCfiles.filter(QRegExp("*.tfc", Qt::CaseInsensitive, QRegExp::Wildcard));
            files += DLCfiles.filter(QRegExp("*.tlk", Qt::CaseInsensitive, QRegExp::Wildcard));
            files += DLCfiles.filter(QRegExp("*.afc", Qt::CaseInsensitive, QRegExp::Wildcard));
            files += DLCfiles.filter(QRegExp("*.cnd", Qt::CaseInsensitive, QRegExp::Wildcard));
            files += DLCfiles.filter(QRegExp("*.bik", Qt::CaseInsensitive, QRegExp::Wildcard));
            files += DLCfiles.filter(QRegExp("*.txt", Qt::CaseInsensitive, QRegExp::Wildcard));
            files += DLCfiles.filter(QRegExp("*.bin", Qt::CaseInsensitive, QRegExp::Wildcard));

            QList<FileEntry> filesList;
            int DLCPathLength = (g_GameData->DLCData() + "/" + DLCDir).size();
            for (int f = 0; f < files.count(); f++)
            {
                FileEntry file{};
                if (!files[f].contains("pcconsoletoc.bin", Qt::CaseInsensitive))
                    file.size = QFileInfo(files[f]).size();
                file.path = files[f].mid(DLCPathLength + 1).replace(QChar('/'), QChar('\\'), Qt::CaseInsensitive);
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
