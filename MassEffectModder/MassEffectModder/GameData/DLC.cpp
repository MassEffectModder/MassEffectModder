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
#include <Helpers/MemoryStream.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <GameData/DLC.h>
#include <GameData/GameData.h>
#include <Wrappers.h>

int ME3DLC::getNumberOfFiles(QString &path)
{
    if (!QFile(path).exists())
    {
        PERROR(QString("Filename missing: ") + path + "\n");
        return -1;
    }
    FileStream stream = FileStream(path, FileMode::Open, FileAccess::ReadOnly);
    uint tag = stream.ReadUInt32();
    if (tag != SfarTag)
    {
        PERROR("Wrong SFAR tag\n");
        return -1;
    }
    uint sfarVersion = stream.ReadUInt32();
    if (sfarVersion != SfarVersion)
    {
        PERROR("Wrong SFAR version\n");
        return -1;
    }

    stream.SkipInt32();
    stream.SkipInt32();
    return stream.ReadInt32();
}

bool ME3DLC::loadHeader(Stream *stream)
{
    uint tag = stream->ReadUInt32();
    if (tag != SfarTag)
    {
        CRASH_MSG("Wrong SFAR tag\n");
        return false;
    }
    uint sfarVersion = stream->ReadUInt32();
    if (sfarVersion != SfarVersion)
    {
        CRASH_MSG("Wrong SFAR version\n");
        return false;
    }

    stream->SkipInt32();
    uint entriesOffset = stream->ReadUInt32();
    filesCount = stream->ReadUInt32();
    uint sizesArrayOffset = stream->ReadUInt32();
    maxBlockSize = stream->ReadUInt32();
    uint compressionTag = stream->ReadUInt32();
    if (compressionTag != LZMATag)
    {
        CRASH_MSG("Not LZMA compression for SFAR file\n");
        return false;
    }

    uint numBlockSizes = 0;
    stream->JumpTo(entriesOffset);
    filesList.clear();
    for (uint i = 0; i < filesCount; i++)
    {
        FileEntry file{};
        stream->ReadToBuffer(file.filenameHash, 16);
        file.compressedBlockSizesIndex = stream->ReadInt32();
        file.uncomprSize = stream->ReadUInt32();
        stream->ReadByte();
        file.dataOffset = stream->ReadUInt32();
        stream->ReadByte();
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
            std::unique_ptr<char[]> outBuf (new char[dstLen]);
            LzmaDecompress(inBuf.ptr(), inBuf.size(), reinterpret_cast<quint8 *>(outBuf.get()), &dstLen);
            if (dstLen != (quint32)filesList[i].uncomprSize)
                CRASH();

            inBuf.Free();
            QTextStream filenamesStream(QByteArray(outBuf.get(), dstLen));
            while (!filenamesStream.atEnd())
            {
                QString name = filenamesStream.readLine();
                QByteArray hash = QCryptographicHash::hash(name.toLower().toUtf8(), QCryptographicHash::Md5);
                for (uint l = 0; l < filesCount; l++)
                {
                    if (memcmp(filesList[l].filenameHash, hash.constData(), 16) == 0)
                    {
                        FileEntry f = filesList[l];
                        f.filenamePath = name;
                        filesList[l] = f;
                        break;
                    }
                }
            }
            filenamesIndex = i;
            break;
        }
    }
    return true;
}

bool ME3DLC::extract(QString &SFARfilename, int &currentProgress,
                     int totalNumber, ProgressCallback callback,
                     void *callbackHandle)
{
    if (!QFile(SFARfilename).exists())
    {
        PERROR(QString("Filename missing: ") + SFARfilename + "\n");
        return false;
    }

    std::unique_ptr<Stream> stream (new MemoryStream(SFARfilename));

    if (!loadHeader(stream.get()))
        return false;

#ifdef GUI
    QElapsedTimer timer;
    timer.start();
#endif
    int lastProgress = -1;
    for (uint i = 0; i < filesCount; i++, currentProgress++)
    {
#ifdef GUI
        if (timer.elapsed() > 100)
        {
            QApplication::processEvents();
            timer.restart();
        }
#endif
        if ((uint)filenamesIndex == i)
            continue;
        if (filesList[i].filenamePath.length() == 0)
        {
            PERROR("Filename list missing in DLC\n");
            return false;
        }

        int newProgress = (100 * currentProgress) / totalNumber;
        if (lastProgress != newProgress)
        {
            lastProgress = newProgress;
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
                ConsoleSync();
            }
        }
        if (callback)
        {
            callback(callbackHandle, newProgress, "Unpacking DLC: " + g_GameData->RelativeGameData(SFARfilename));
        }

        QString filename = filesList[i].filenamePath.mid(QString(R"(BIOGame/DLC/)").length());
        QDir().mkpath(g_GameData->DLCData() + DirName(filename));
        {
            FileStream outputFile = FileStream(g_GameData->DLCData() + filename, FileMode::Create, FileAccess::WriteOnly);
            stream->JumpTo(filesList[i].dataOffset);
            if (filesList[i].compressedBlockSizesIndex == -1)
            {
                outputFile.CopyFrom(*stream, filesList[i].uncomprSize);
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
                    int uncompressedBlockSize = qMin((int)bytesLeft, (int)maxBlockSize);
                    if (compressedBlockSize == 0)
                    {
                        compressedBlockSize = (int)maxBlockSize;
                    }
                    compressedBlockBuffers.push_back(stream->ReadToBuffer(compressedBlockSize));
                    uncompressedBlockBuffers.push_back(ByteBuffer(uncompressedBlockSize));
                    bytesLeft -= uncompressedBlockSize;
                }

                bool status = true;
                #pragma omp parallel for
                for (uint j = 0; j < filesList[i].numBlocks; j++)
                {
                    int compressedBlockSize = blockSizes[filesList[i].compressedBlockSizesIndex + j];
                    if (compressedBlockSize == 0 || compressedBlockSize == blockBytesLeft[j])
                    {
                    }
                    else
                    {
                        uint dstLen = uncompressedBlockBuffers[j].size();
                        LzmaDecompress(compressedBlockBuffers[j].ptr(), compressedBlockBuffers[j].size(),
                                       uncompressedBlockBuffers[j].ptr(), &dstLen);
                        if (uncompressedBlockBuffers[j].size() == 0)
                        {
                            PERROR("Decompression failed\n");
                            status = false;
                        }
                    }
                }

                if (!status)
                    return false;

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

    QFile::remove(SFARfilename);
    FileStream outputFile = FileStream(SFARfilename, FileMode::Create, FileAccess::WriteOnly);
    outputFile.WriteUInt32(SfarTag);
    outputFile.WriteUInt32(SfarVersion);
    outputFile.WriteUInt32(HeaderSize);
    outputFile.WriteUInt32(HeaderSize);
    outputFile.WriteUInt32(0);
    outputFile.WriteUInt32(HeaderSize);
    outputFile.WriteUInt32((uint)MaxBlockSize);
    outputFile.WriteUInt32(LZMATag);

    return true;
}

void ME3DLC::unpackAllDLC(ProgressCallback callback, void *callbackHandle)
{
    if (!QDir(g_GameData->DLCData()).exists())
        return;

#ifdef GUI
    QElapsedTimer timer;
    timer.start();
#endif
    QStringList sfarFiles;
    int totalSfars = 0;
    QStringList DLCs = QDir(g_GameData->DLCData(), "DLC_*", QDir::NoSort, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryList();
    foreach (QString DLCDir, DLCs)
    {
#ifdef GUI
        if (timer.elapsed() > 100)
        {
            QApplication::processEvents();
            timer.restart();
        }
#endif
        QDirIterator iterator(g_GameData->DLCData() + "/" + DLCDir, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
        bool isValid = true;
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
        if (!isValid || sfarFile.length() == 0)
        {
            continue;
        }
        sfarFiles.push_back(sfarFile);
    }

    if (sfarFiles.count() == 0)
    {
        return;
    }
    if (g_ipc)
    {
        ConsoleWrite(QString("[IPC]STAGE_WEIGHT STAGE_UNPACKDLC %1").arg(
            QString::number(((float)sfarFiles.count()) / totalSfars)));
        ConsoleSync();
    }

    int totalNumFiles = 0;
    int currentProgress = 0;
    for (int i = 0; i < sfarFiles.count(); i++)
    {
        if (getNumberOfFiles(sfarFiles[i]) != -1)
            totalNumFiles += getNumberOfFiles(sfarFiles[i]);
    }

    for (int i = 0; i < sfarFiles.count(); i++)
    {
        ME3DLC dlc = ME3DLC();
        if (g_ipc)
        {
            ConsoleWrite("[IPC]PROCESSING_FILE " + g_GameData->RelativeGameData(sfarFiles[i]));
            ConsoleSync();
        }
        else
        {
            PINFO("Unpacking SFAR: " + g_GameData->RelativeGameData(sfarFiles[i]) + "\n");
        }

        if (!dlc.extract(sfarFiles[i], currentProgress, totalNumFiles, callback, callbackHandle))
        {
            if (g_ipc)
            {
                ConsoleWrite("[IPC]ERROR Failed to unpack DLC");
                ConsoleSync();
            }
            else
            {
                PERROR("Error: Failed to unpack: " + g_GameData->RelativeGameData(sfarFiles[i]) + "\n");
            }
        }
    }
}
