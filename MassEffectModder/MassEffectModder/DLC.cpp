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
#include "Helpers/MemoryStream.h"
#include "Helpers/MiscHelpers.h"
#include "DLC.h"
#include "GameData.h"
#include "Wrappers.h"

#include <QCryptographicHash>
#include <QTextStream>

int ME3DLC::getNumberOfFiles(QString &path)
{
    if (!QFile(path).exists())
        CRASH_MSG("filename missing");
    FileStream stream = FileStream(path, FileMode::Open, FileAccess::ReadOnly);
    uint tag = stream.ReadUInt32();
    if (tag != SfarTag)
        CRASH_MSG("Wrong SFAR tag");
    uint sfarVersion = stream.ReadUInt32();
    if (sfarVersion != SfarVersion)
        CRASH_MSG("Wrong SFAR version");

    stream.SkipInt32();
    stream.SkipInt32();
    return stream.ReadInt32();
}

void ME3DLC::loadHeader(Stream *stream)
{
    uint tag = stream->ReadUInt32();
    if (tag != SfarTag)
        CRASH_MSG("Wrong SFAR tag");
    uint sfarVersion = stream->ReadUInt32();
    if (sfarVersion != SfarVersion)
        CRASH_MSG("Wrong SFAR version");

    stream->SkipInt32();
    uint entriesOffset = stream->ReadUInt32();
    filesCount = stream->ReadUInt32();
    uint sizesArrayOffset = stream->ReadUInt32();
    maxBlockSize = stream->ReadUInt32();
    uint compressionTag = stream->ReadUInt32();
    if (compressionTag != LZMATag)
        CRASH_MSG("Not LZMA compression for SFAR file");

    uint numBlockSizes = 0;
    stream->JumpTo(entriesOffset);
    filesList.clear();
    for (uint i = 0; i < filesCount; i++)
    {
        FileEntry file{};
        stream->ReadToBuffer(file.filenameHash, 16);
        file.compressedBlockSizesIndex = stream->ReadInt32();
        file.uncomprSize = stream->ReadUInt32();
        file.uncomprSize |= (long)stream->ReadByte() << 32;
        file.dataOffset = stream->ReadUInt32();
        file.dataOffset |= (long)stream->ReadByte() << 32;
        file.numBlocks = (uint)((file.uncomprSize + maxBlockSize - 1) / maxBlockSize);
        filesList.push_back(file);
        numBlockSizes += file.numBlocks;
    }

    stream->JumpTo(sizesArrayOffset);
    blockSizes.clear();
    for (uint i = 0; i < numBlockSizes; i++)
    {
        blockSizes.push_back(stream->ReadUInt16());
    }

    filenamesIndex = -1;
    for (uint i = 0; i < filesCount; i++)
    {
        if (memcmp(filesList[i].filenameHash, FileListHash, 16) == 0)
        {
            stream->JumpTo(filesList[i].dataOffset);
            int compressedBlockSize = blockSizes[filesList[i].compressedBlockSizesIndex];
            quint32 dstLen = filesList[i].uncomprSize;
            auto inBuf = stream->ReadToBuffer(compressedBlockSize);
            auto outBuf = new char[dstLen];
            LzmaDecompress(inBuf.ptr(), inBuf.size(), reinterpret_cast<quint8 *>(outBuf), &dstLen);
            if (dstLen != filesList[i].uncomprSize)
                CRASH();

            inBuf.Free();
            QByteArray outArray = QByteArray(outBuf, dstLen);
            delete[] outBuf;

            QTextStream *filenamesStream = new QTextStream(outArray);
            while (!filenamesStream->atEnd())
            {
                QString name = filenamesStream->readLine();
                QByteArray hash = QCryptographicHash::hash(name.toLower().toUtf8(), QCryptographicHash::Md5);
                for (uint l = 0; l < filesCount; l++)
                {
                    if (memcmp(filesList[l].filenameHash, hash.constData(), 16) == 0)
                    {
                        FileEntry f = filesList[l];
                        f.filenamePath = name;
                        filesList[l] = f;
                    }
                }
            }
            filenamesIndex = i;
            break;
        }
    }
    if (filenamesIndex == -1)
        CRASH_MSG("filenames entry not found");
}

void ME3DLC::extract(QString &SFARfilename, QString &outPath, bool ipc, int &currentProgress, int totalNumber)
{
    if (!QFile(SFARfilename).exists())
        CRASH_MSG("filename missing");

    auto stream = reinterpret_cast<Stream *>(new MemoryStream(SFARfilename));

    QFile::remove(SFARfilename);

    {
        FileStream outputFile = FileStream(SFARfilename, FileMode::Create, FileAccess::WriteOnly);
        outputFile.WriteUInt32(SfarTag);
        outputFile.WriteUInt32(SfarVersion);
        outputFile.WriteUInt32(HeaderSize);
        outputFile.WriteUInt32(HeaderSize);
        outputFile.WriteUInt32(0);
        outputFile.WriteUInt32(HeaderSize);
        outputFile.WriteUInt32((uint)MaxBlockSize);
        outputFile.WriteUInt32(LZMATag);
    }

    loadHeader(stream);

    int lastProgress = -1;
    for (uint i = 0; i < filesCount; i++, currentProgress++)
    {
        if ((uint)filenamesIndex == i)
            continue;
        if (filesList[i].filenamePath == "")
            CRASH_MSG("filename missing");

        if (ipc)
        {
            int newProgress = (100 * currentProgress) / totalNumber;
            if (lastProgress != newProgress)
            {
                ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + newProgress);
                ConsoleSync();
                lastProgress = newProgress;
            }
        }

        int pos = filesList[i].filenamePath.indexOf(R"(\BIOGame\DLC\)", Qt::CaseInsensitive);
        QString filename = filesList[i].filenamePath.mid(pos + QString(R"(\BIOGame\DLC\)").length());
        QString dir = DirName(outPath);
        QDir().mkpath(dir + filename);
        {
            FileStream outputFile = FileStream(dir + filename, FileMode::Create, FileAccess::WriteOnly);
            stream->JumpTo(filesList[i].dataOffset);
            if (filesList[i].compressedBlockSizesIndex == -1)
            {
                outputFile.CopyFrom(stream, filesList[i].uncomprSize);
            }
            else
            {
                QList<ByteBuffer> uncompressedBlockBuffers{};
                QList<ByteBuffer> compressedBlockBuffers{};
                QList<long> blockBytesLeft = QList<long>();
                long bytesLeft = filesList[i].uncomprSize;
                for (uint j = 0; j < filesList[i].numBlocks; j++)
                {
                    blockBytesLeft.push_back(bytesLeft);
                    int compressedBlockSize = blockSizes[filesList[i].compressedBlockSizesIndex + j];
                    int uncompressedBlockSize = (int)qMin((uint)bytesLeft, maxBlockSize);
                    if (compressedBlockSize == 0)
                    {
                        compressedBlockSize = (int)maxBlockSize;
                    }
                    compressedBlockBuffers.push_back(stream->ReadToBuffer(compressedBlockSize));
                    uncompressedBlockBuffers.push_back(ByteBuffer());
                    bytesLeft -= uncompressedBlockSize;
                }

                for (uint j = 0; j < filesList[i].numBlocks; j++)
                {
                    int compressedBlockSize = blockSizes[filesList[i].compressedBlockSizesIndex + j];
                    if (compressedBlockSize == 0 || compressedBlockSize == blockBytesLeft[j])
                    {
                        uncompressedBlockBuffers[j] = compressedBlockBuffers[j];
                    }
                    else
                    {
                        uint dstLen = uncompressedBlockBuffers[j].size();
                        LzmaDecompress(compressedBlockBuffers[j].ptr(), compressedBlockBuffers[j].size(),
                                       uncompressedBlockBuffers[j].ptr(), &dstLen);
                        if (uncompressedBlockBuffers[j].size() == 0)
                            CRASH();
                    }
                }

                for (uint j = 0; j < filesList[i].numBlocks; j++)
                {
                    int compressedBlockSize = blockSizes[filesList[i].compressedBlockSizesIndex + j];
                    if (compressedBlockSize == 0 || compressedBlockSize == blockBytesLeft[j])
                        outputFile.WriteFromBuffer(compressedBlockBuffers[j]);
                    else
                        outputFile.WriteFromBuffer(uncompressedBlockBuffers[j]);
                    compressedBlockBuffers[j].Free();
                    uncompressedBlockBuffers[j].Free();
                }
            }
        }
    }

    delete stream;
}

void ME3DLC::unpackAllDLC(bool ipc)
{
    if (!QDir(g_GameData->DLCData()).exists())
        return;

    QStringList sfarFiles;
    int totalSfars = 0;
    QStringList DLCs = QDir(g_GameData->DLCData(), "DLC_*", QDir::NoSort, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryList();
    foreach (QString DLCDir, DLCs)
    {
        QDirIterator iterator(g_GameData->DLCData() + "/" + DLCDir, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
        bool isValid = false;
        QString sfarFile;
        while (iterator.hasNext())
        {
            iterator.next();
            if (iterator.filePath().endsWith("Default.sfar", Qt::CaseInsensitive))
            {
                sfarFile = iterator.filePath();
                totalSfars++;
                continue;
            }
            if (iterator.filePath().endsWith("Mount.dlc", Qt::CaseInsensitive))
            {
                isValid = false;
                break;
            }
        }
        if (!isValid || sfarFile == "")
        {
            continue;
        }
        sfarFiles.push_back(sfarFile);
    }

    if (sfarFiles.count() == 0)
    {
        return;
    }
    if (ipc)
    {
        ConsoleWrite(QString("[IPC]STAGE_WEIGHT STAGE_UNPACKDLC %1").arg(
            QString::number(((float)sfarFiles.count()) / totalSfars)));
        ConsoleSync();
    }

    int totalNumFiles = 0;
    int currentProgress = 0;
    for (int i = 0; i < sfarFiles.count(); i++)
    {
        totalNumFiles += getNumberOfFiles(sfarFiles[i]);
    }

    for (int i = 0; i < sfarFiles.count(); i++)
    {
        QString DLCname = BaseName(DirName(DirName(sfarFiles[i])));
        QString outPath = g_GameData->DLCData() + "/" + DLCname;
        ME3DLC dlc = ME3DLC();
        if (ipc)
        {
            ConsoleWrite("[IPC]PROCESSING_FILE " + sfarFiles[i]);
            ConsoleSync();
        }
        dlc.extract(sfarFiles[i], outPath, ipc, currentProgress, totalNumFiles);
    }
}
