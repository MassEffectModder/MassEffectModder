/*
 * MassEffectModder
 *
 * Copyright (C) 2018-2021 Pawel Kolodziejski
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

#include <Misc/Misc.h>
#include <MipMaps/MipMaps.h>
#include <Wrappers.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>

uint Misc::scanFilenameForCRC(const QString &inputFile)
{
    QString filename = BaseNameWithoutExt(inputFile);
    if (!filename.contains("0x"))
    {
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]ERROR_FILE_NOT_COMPATIBLE ") + BaseName(inputFile));
            ConsoleSync();
        }
        else
        {
            PERROR(QString("Texture filename not valid: ") + BaseName(inputFile) +
                         " Texture filename must include texture CRC (0xhhhhhhhh). Skipping texture...\n");
        }
        return false;
    }
    int idx = filename.indexOf("0x");
    if (filename.size() - idx < 10)
    {
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]ERROR_FILE_NOT_COMPATIBLE ") + BaseName(inputFile));
            ConsoleSync();
        }
        else
        {
            PERROR(QString("Texture filename not valid: ") + BaseName(inputFile) +
                         " Texture filename must include texture CRC (0xhhhhhhhh). Skipping texture...\n");
        }
        return false;
    }
    QString crcStr = filename.mid(idx, 10);
    bool ok;
    uint crc = crcStr.toUInt(&ok, 16);
    if (crc == 0)
    {
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]ERROR_FILE_NOT_COMPATIBLE ") + BaseName(inputFile));
            ConsoleSync();
        }
        else
        {
            PERROR(QString("Texture filename not valid: ") + BaseName(inputFile) +
                         " Texture filename must include texture CRC (0xhhhhhhhh). Skipping texture...\n");
        }
        return false;
    }

    return crc;
}

bool Misc::CheckMEMHeader(FileStream &fs, const QString &file)
{
    uint tag = fs.ReadUInt32();
    uint version = fs.ReadUInt32();
    if (tag != TextureModTag || version != TextureModVersion)
    {
        if (version != TextureModVersion)
        {
            PERROR(QString("File ") + BaseName(file) + " was made with an older version of MEM, skipping...\n");
        }
        else
        {
            PERROR(QString("File ") + BaseName(file) + " is not a valid MEM mod, skipping...\n");
        }
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]ERROR_FILE_NOT_COMPATIBLE ") + BaseName(file));
            ConsoleSync();
        }
        return false;
    }
    return true;
}

bool Misc::CheckMEMGameVersion(FileStream &fs, const QString &file, int gameId)
{
    uint gameType = 0;
    fs.JumpTo(fs.ReadInt64());
    gameType = fs.ReadUInt32();
    if ((MeType)gameType != gameId)
    {
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]ERROR_FILE_NOT_COMPATIBLE ") + BaseName(file));
            ConsoleSync();
        }
        else
        {
            PERROR(QString("File ") + file + " is not a MEM mod valid for this game.\n");
        }
        return false;
    }
    return true;
}

bool Misc::DetectMarkToConvertFromFile(const QString &file)
{
    int idx = file.indexOf("-memconvert", Qt::CaseInsensitive);
    return idx > 0;
}

bool Misc::DetectHashFromFile(const QString &file)
{
    int idx = file.indexOf("-hash", Qt::CaseInsensitive);
    return idx > 0;
}

bool Misc::DetectBc7FromFile(const QString &file)
{
    int idx = file.indexOf("-bc7format", Qt::CaseInsensitive);
    return idx > 0;
}

bool Misc::convertDataModtoMem(QFileInfoList &files, QString &memFilePath,
                               MeType gameId, QList<TextureMapEntry> &textures,
                               bool fastMode, bool markToConvert, bool bc7format, float bc7quality,
                               ProgressCallback callback, void *callbackHandle)
{
    PINFO("Mods conversion started...\n");

    QStringList ddsList;

    QList<BinaryMod> mods = QList<BinaryMod>();
    QList<FileMod> modFiles = QList<FileMod>();

    QString dir = DirName(memFilePath);
    if (dir != memFilePath)
        QDir().mkpath(dir);

    if (QFile(memFilePath).exists())
        QFile(memFilePath).remove();

    Misc::startTimer();

    FileStream outFs = FileStream(memFilePath, FileMode::Create, FileAccess::WriteOnly);
    outFs.WriteUInt32(TextureModTag);
    outFs.WriteUInt32(TextureModVersion);
    outFs.WriteInt64(0); // filled later

    int lastProgress = -1;
    for (int n = 0; n < files.count(); n++)
    {
#ifdef GUI
        QApplication::processEvents();
#endif
        foreach (BinaryMod mod, mods)
        {
            mod.data.Free();
        }
        mods.clear();

        QString file = files[n].absoluteFilePath();
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + BaseName(file));
            ConsoleSync();
        }
        else
        {
            PINFO(QString("File: ") + BaseName(file) + "\n");
        }
        int newProgress = (n * 100) / files.count();
        if (lastProgress != newProgress)
        {
            lastProgress = newProgress;
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
                ConsoleSync();
            }
            if (callback)
            {
                callback(callbackHandle, newProgress, "Converting");
            }
        }

        if (file.endsWith(".mem", Qt::CaseInsensitive))
        {
            FileStream fs = FileStream(file, FileMode::Open, FileAccess::ReadOnly);
            if (!CheckMEMHeader(fs, file))
                continue;

            if (!CheckMEMGameVersion(fs, file, gameId))
                continue;

            int numFiles = fs.ReadInt32();
            for (int l = 0; l < numFiles; l++)
            {
#ifdef GUI
                QApplication::processEvents();
#endif
                FileMod fileMod{};
                fileMod.tag = fs.ReadUInt32();
                fs.ReadStringASCIINull(fileMod.name);
                fileMod.offset = fs.ReadInt64();
                fileMod.size = fs.ReadInt64();
                fileMod.flags = fs.ReadUInt64();
                qint64 prevPos = fs.Position();
                fs.JumpTo(fileMod.offset);
                fileMod.offset = outFs.Position();
                if (fileMod.tag == FileTextureTag ||
                    fileMod.tag == FileMovieTextureTag)
                {
                    outFs.WriteUInt32(fs.ReadUInt32()); // flags
                    outFs.WriteUInt32(fs.ReadUInt32()); // crc
                }
                else
                    CRASH();

                outFs.CopyFrom(fs, fileMod.size);
                fs.JumpTo(prevPos);
                modFiles.push_back(fileMod);
            }
        }
        else if (file.endsWith(".dds", Qt::CaseInsensitive) ||
                 file.endsWith(".png", Qt::CaseInsensitive) ||
                 file.endsWith(".bmp", Qt::CaseInsensitive) ||
                 file.endsWith(".tga", Qt::CaseInsensitive))
        {
            BinaryMod mod{};
            TextureMapEntry f;
            bool entryMarkToConvert = markToConvert;
            uint crc = scanFilenameForCRC(file);
            if (crc == 0)
                continue;

            if (bc7format || DetectMarkToConvertFromFile(file))
                entryMarkToConvert = true;

            if (DetectBc7FromFile(file))
            {
                bc7format = true;
                entryMarkToConvert = true;
            }

            bool forceHash = DetectHashFromFile(file);
            if (!forceHash)
            {
                f = FoundTextureInTheMap(textures, crc);
                if (f.crc == 0)
                {
                    PINFO(QString("Texture skipped. Texture ") + BaseName(file) +
                                 " is not present in your game setup.\n");
                    continue;
                }
            }

            Image image(file, ImageFormat::UnknownImageFormat);
            if (forceHash)
            {
                f.width = image.getMipMaps().first()->getOrigWidth();
                f.height = image.getMipMaps().first()->getOrigHeight();
            }

            if (!Misc::CheckImage(image, f, file, -1))
                continue;

            if (!forceHash)
            {
                PixelFormat newPixelFormat = f.pixfmt;
                if (entryMarkToConvert)
                {
                    newPixelFormat = changeTextureType(f.pixfmt, image.getPixelFormat(), f.type, bc7format);
                    if (f.pixfmt == newPixelFormat)
                        PINFO(QString("Warning for texture: ") + BaseName(file)  +
                              " This texture can not be converted to desired format...\n");
                }

                int numMips = Misc::GetNumberOfMipsFromMap(f);
                CorrectTexture(image, f, numMips, newPixelFormat, file, bc7quality);
            }
            else if (image.getPixelFormat() == PixelFormat::Internal)
            {
                PINFO(QString("Warning for texture: ") + BaseName(file) +
                      " This texture can not be included as non-DDS...\n");
                continue;
            }

            mod.data = image.StoreImageToDDS();
            mod.name = f.name;
            mod.movieTexture = false;
            mod.textureCrc = crc;
            mod.markConvert = entryMarkToConvert;
            mod.bc7Format = bc7format;
            mod.forceHash = forceHash;
            mods.push_back(mod);
        }
        else if (file.endsWith(".bik", Qt::CaseInsensitive))
        {
            BinaryMod mod{};
            TextureMapEntry f;

            uint crc = scanFilenameForCRC(file);
            if (crc == 0)
                continue;

            bool forceHash = DetectHashFromFile(file);
            if (!forceHash)
            {
                f = FoundTextureInTheMap(textures, crc);
                if (f.crc == 0)
                {
                    PINFO(QString("Texture skipped. Texture ") + BaseName(file) +
                                 " is not present in your game setup.\n");
                    continue;
                }
            }

            FileStream fs = FileStream(file, FileMode::Open);
            quint32 tag = fs.ReadUInt32();
            if (tag != BIK_TAG)
            {
                PINFO(QString("File mod is not supported Bik movie: ") + BaseName(file) +
                             ", skipping...\n");
                continue;
            }
            int dataSize = fs.ReadInt32() + 8;
            if (dataSize != QFile(file).size())
            {
                if (g_ipc)
                {
                    ConsoleWrite(QString("[IPC]ERROR_FILE_NOT_COMPATIBLE ") + BaseName(file));
                    ConsoleSync();
                }
                else
                {
                    PINFO(QString("Skipping movie: ") + f.name + QString::asprintf("_0x%08X", f.crc) +
                         " This texture has wrong size, skipping...");
                }
                continue;
            }
            if (!forceHash)
            {
                fs.Skip(12);
                int w = fs.ReadInt32();
                int h = fs.ReadInt32();
                if (w / h != f.width / f.height)
                {
                    if (g_ipc)
                    {
                        ConsoleWrite(QString("[IPC]ERROR_FILE_NOT_COMPATIBLE ") + BaseName(file));
                        ConsoleSync();
                    }
                    else
                    {
                        PINFO(QString("Skipping movie: ") + f.name + QString::asprintf("_0x%08X", f.crc) +
                             " This texture has wrong aspect ratio, skipping...");
                    }
                    continue;
                }
            }
            fs.SeekBegin();

            mod.data = fs.ReadToBuffer(dataSize);
            mod.name = f.name;
            mod.movieTexture = true;
            mod.textureCrc = crc;
            mod.markConvert = false;
            mod.forceHash = forceHash;
            mods.push_back(mod);
        }

        for (int l = 0; l < mods.count(); l++)
        {
#ifdef GUI
            QApplication::processEvents();
#endif
            FileMod fileMod{};
            quint32 textureFlags{};
            std::unique_ptr<Stream> dst (new MemoryStream());
            if (mods[l].movieTexture)
                fileMod.tag = FileMovieTextureTag;
            else
                fileMod.tag = FileTextureTag;
            fileMod.name = mods[l].name;

            if (mods[l].data.size() != 0)
            {
                Misc::compressData(mods[l].data, *dst, fastMode ? CompressionDataType::Zlib : CompressionDataType::LZMA);
                dst->SeekBegin();
                fileMod.offset = outFs.Position();
                fileMod.size = dst->Length();
            }
            else
            {
                fileMod.offset = mods[l].offset;
                fileMod.size = mods[l].size;
            }

            if (mods[l].markConvert)
                textureFlags |= (quint32)ModTextureFlags::MarkToConvert;
            if (mods[l].forceHash)
                textureFlags |= (quint32)ModTextureFlags::ForceHash;

            outFs.WriteUInt32(textureFlags);
            outFs.WriteUInt32(mods[l].textureCrc);
            if (mods[l].data.size() != 0)
                outFs.CopyFrom(*dst, dst->Length());
            modFiles.push_back(fileMod);
        }
    }
    foreach (BinaryMod mod, mods)
    {
        mod.data.Free();
    }
    mods.clear();

    if (modFiles.count() == 0)
    {
        outFs.Close();
        if (QFile(memFilePath).exists())
            QFile(memFilePath).remove();
        if (g_ipc)
        {
            ConsoleWrite("[IPC]ERROR_NO_BUILDABLE_FILES");
            ConsoleSync();
            return true;
        }

        PERROR("MEM file not created, nothing to build!\n");
        return false;
    }

    qint64 pos = outFs.Position();
    outFs.SeekBegin();
    outFs.WriteUInt32(TextureModTag);
    outFs.WriteUInt32(TextureModVersion);
    outFs.WriteInt64(pos);
    outFs.JumpTo(pos);
    outFs.WriteUInt32((uint)gameId);
    outFs.WriteInt32(modFiles.count());
    for (int i = 0; i < modFiles.count(); i++)
    {
        outFs.WriteUInt32(modFiles[i].tag);
        outFs.WriteStringASCIINull(modFiles[i].name);
        outFs.WriteInt64(modFiles[i].offset);
        outFs.WriteInt64(modFiles[i].size);
        outFs.WriteInt64(modFiles[i].flags);
    }

    long elapsed = Misc::elapsedTime();
    PINFO(Misc::getTimerFormat(elapsed) + "\n");

    return true;
}

bool Misc::extractMEM(MeType gameId, QFileInfoList &inputList, QString &outputDir,
                      ProgressCallback callback, void *callbackHandle)
{
    PINFO("Extract MEM files started...\n");

    outputDir = QDir::cleanPath(outputDir);
    if (outputDir.length() != 0)
    {
        QDir().mkpath(outputDir);
        outputDir += "/";
    }

    Misc::startTimer();

    int currentNumberOfTotalMods = 1;
    int totalNumberOfMods = 0;
    for (int i = 0; i < inputList.count(); i++)
    {
        FileStream fs = FileStream(inputList[i].absoluteFilePath(), FileMode::Open, FileAccess::ReadOnly);
        uint tag = fs.ReadUInt32();
        uint version = fs.ReadUInt32();
        if (tag != TextureModTag || version != TextureModVersion)
            continue;
        fs.JumpTo(fs.ReadInt64());
        fs.SkipInt32();
        totalNumberOfMods += fs.ReadInt32();
    }

    int lastProgress = -1;
    foreach (QFileInfo file, inputList)
    {
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + file.fileName());
            ConsoleSync();
        }
        else
        {
            PINFO(QString("Extract MEM: ") + file.absoluteFilePath() + "\n");
        }
        QString outputMODdir = outputDir + BaseNameWithoutExt(file.fileName());
        QDir().mkpath(outputMODdir);

        FileStream fs = FileStream(file.absoluteFilePath(), FileMode::Open, FileAccess::ReadOnly);
        if (!Misc::CheckMEMHeader(fs, file.absoluteFilePath()))
            continue;

        if (!Misc::CheckMEMGameVersion(fs, file.absoluteFilePath(), gameId))
            continue;

        int numFiles = fs.ReadInt32();
        QList<FileMod> modFiles = QList<FileMod>();
        for (int i = 0; i < numFiles; i++)
        {
            FileMod fileMod{};
            fileMod.tag = fs.ReadUInt32();
            fs.ReadStringASCIINull(fileMod.name);
            fileMod.offset = fs.ReadInt64();
            fileMod.size = fs.ReadInt64();
            fileMod.flags = fs.ReadUInt64();
            modFiles.push_back(fileMod);
        }
        numFiles = modFiles.count();

        for (int i = 0; i < numFiles; i++, currentNumberOfTotalMods++)
        {
#ifdef GUI
            QApplication::processEvents();
#endif
            long size = 0;
            quint32 flags = 0, crc = 0;
            fs.JumpTo(modFiles[i].offset);
            size = modFiles[i].size;

            if (modFiles[i].tag == FileTextureTag ||
                modFiles[i].tag == FileMovieTextureTag)
            {
                flags = fs.ReadUInt32();
                crc = fs.ReadUInt32();
            }
            else
            {
                if (g_ipc)
                {
                    ConsoleWrite(QString("[IPC]ERROR_FILE_NOT_COMPATIBLE ") + file.absoluteFilePath());
                    ConsoleSync();
                }
                PERROR(QString("Unknown tag for file: ") + QString::number(i + 1) + " of " +
                       QString::number(numFiles) + "\n");
                continue;
            }

            PINFO(QString("Processing MEM mod ") + file.fileName() +
                             " - File " + QString::number(i + 1) + " of " +
                             QString::number(numFiles) + " - " + modFiles[i].name + "\n");
            int newProgress = currentNumberOfTotalMods * 100 / totalNumberOfMods;
            if (lastProgress != newProgress)
            {
                lastProgress = newProgress;
                if (g_ipc)
                {
                    ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
                    ConsoleSync();
                }
                if (callback)
                {
                    callback(callbackHandle, newProgress, "Extracting");
                }
            }

            ByteBuffer dst = Misc::decompressData(fs, size);
            if (dst.size() == 0)
            {
                if (g_ipc)
                {
                    ConsoleWrite(QString("[IPC]ERROR_FILE_NOT_COMPATIBLE ") + file.absoluteFilePath());
                    ConsoleSync();
                }
                PERROR(QString("Failed decompress data: ") + file.absoluteFilePath() + "\n");
                PERROR("Extract MEM mod files failed.\n\n");
                return false;
            }

            if (modFiles[i].tag == FileTextureTag ||
                modFiles[i].tag == FileMovieTextureTag)
            {
                QString filename = outputMODdir + "/" + modFiles[i].name + QString::asprintf("_0x%08X", crc);
                if (flags == ModTextureFlags::ForceHash)
                    filename += "-hash";
                if (flags & ModTextureFlags::MarkToConvert)
                    filename += "-memconvert";
                if (modFiles[i].tag == FileTextureTag)
                    filename += ".dds";
                else if (modFiles[i].tag == FileMovieTextureTag)
                    filename += ".bik";
                else
                    CRASH();
                FileStream output = FileStream(filename, FileMode::Create, FileAccess::ReadWrite);
                output.WriteFromBuffer(dst);
            }
            dst.Free();
        }
    }

    long elapsed = Misc::elapsedTime();
    PINFO(Misc::getTimerFormat(elapsed) + "\n");

    PINFO("Extract MEM mod files completed.\n\n");
    return true;
}

bool Misc::compressData(ByteBuffer inputData, Stream &ouputStream, CompressionDataType compType)
{
    uint compressedSize = 0;
    uint dataBlockLeft = inputData.size();
    uint newNumBlocks = ((uint)inputData.size() + ModsDataEnums::MaxBlockSize - 1) / ModsDataEnums::MaxBlockSize;
    QList<Package::ChunkBlock> blocks{};
    {
        MemoryStream inputStream = MemoryStream(inputData);
        // skip blocks header and table - filled later
        ouputStream.JumpTo(ModsDataEnums::SizeOfChunk + ModsDataEnums::SizeOfChunkBlock * newNumBlocks);

        for (uint b = 0; b < newNumBlocks; b++)
        {
            Package::ChunkBlock block{};
            block.uncomprSize = qMin((uint)ModsDataEnums::MaxBlockSize, dataBlockLeft);
            dataBlockLeft -= block.uncomprSize;
            block.uncompressedBuffer = new quint8[block.uncomprSize];
            if (block.uncompressedBuffer == nullptr)
                CRASH_MSG((QString("Out of memory! - amount: ") +
                           QString::number(block.uncomprSize)).toStdString().c_str());
            inputStream.ReadToBuffer(block.uncompressedBuffer, block.uncomprSize);
            blocks.push_back(block);
        }
    }

    bool failed = false;
    #pragma omp parallel for
    for (int b = 0; b < blocks.count(); b++)
    {
        Package::ChunkBlock block = blocks[b];
        if (compType == CompressionDataType::Zlib)
        {
            if (ZlibCompress(block.uncompressedBuffer, block.uncomprSize, &block.compressedBuffer, &block.comprSize) == -100)
                CRASH_MSG("Out of memory!");
        }
        else if (compType == CompressionDataType::LZMA)
        {
            if (LzmaCompress(block.uncompressedBuffer, block.uncomprSize, &block.compressedBuffer, &block.comprSize) == -100)
                CRASH_MSG("Out of memory!");
        }
        else
            CRASH_MSG("Compression type not expected!");
        if (block.comprSize == 0)
        {
            failed = true;
        }
        blocks[b] = block;
    }

    foreach (Package::ChunkBlock block, blocks)
    {
        if (!failed)
        {
            ouputStream.WriteFromBuffer(block.compressedBuffer, (int)block.comprSize);
            compressedSize += block.comprSize;
        }
        delete[] block.uncompressedBuffer;
        delete[] block.compressedBuffer;
    }

    if (failed)
        return false;

    ouputStream.SeekBegin();
    ouputStream.WriteUInt32(compressedSize);
    ouputStream.WriteInt32(inputData.size());
    ouputStream.WriteUInt32((quint32)compType);
    foreach (Package::ChunkBlock block, blocks)
    {
        ouputStream.WriteUInt32(block.comprSize);
        ouputStream.WriteUInt32(block.uncomprSize);
    }

    return true;
}

ByteBuffer Misc::decompressData(Stream &stream, long compressedSize)
{
    uint compressedChunkSize = stream.ReadUInt32();
    uint uncompressedChunkSize = stream.ReadUInt32();
    auto compType = (CompressionDataType)stream.ReadUInt32();
    auto data = ByteBuffer(uncompressedChunkSize);
    uint blocksCount = (uncompressedChunkSize + ModsDataEnums::MaxBlockSize - 1) / ModsDataEnums::MaxBlockSize;
    if ((compressedChunkSize + ModsDataEnums::SizeOfChunk + ModsDataEnums::SizeOfChunkBlock * blocksCount) != (uint)compressedSize)
    {
        return ByteBuffer{};
    }

    QList<Package::ChunkBlock> blocks{};
    for (uint b = 0; b < blocksCount; b++)
    {
        Package::ChunkBlock block{};
        block.comprSize = stream.ReadUInt32();
        block.uncomprSize = stream.ReadUInt32();
        blocks.push_back(block);
    }

    for (int b = 0; b < blocks.count(); b++)
    {
        Package::ChunkBlock block = blocks[b];
        block.compressedBuffer = new quint8[block.comprSize];
        if (block.compressedBuffer == nullptr)
            CRASH_MSG((QString("Out of memory! - amount: ") +
                       QString::number(block.comprSize)).toStdString().c_str());
        stream.ReadToBuffer(block.compressedBuffer, block.comprSize);
        block.uncompressedBuffer = new quint8[ModsDataEnums::MaxBlockSize * 2];
        if (block.uncompressedBuffer == nullptr)
            CRASH_MSG((QString("Out of memory! - amount: ") +
                       QString::number(ModsDataEnums::MaxBlockSize * 2)).toStdString().c_str());
        blocks[b] = block;
    }

    bool failed = false;
    #pragma omp parallel for
    for (int b = 0; b < blocks.count(); b++)
    {
        uint dstLen = ModsDataEnums::MaxBlockSize * 2;
        Package::ChunkBlock block = blocks[b];
        if (compType == CompressionDataType::Zlib)
        {
            if (ZlibDecompress(block.compressedBuffer, block.comprSize, block.uncompressedBuffer, &dstLen) == -100)
                CRASH_MSG("Out of memory!");
        }
        else if (compType == CompressionDataType::LZMA)
        {
            dstLen = block.uncomprSize;
            if (LzmaDecompress(block.compressedBuffer, block.comprSize, block.uncompressedBuffer, &dstLen) != 0)
                failed = true;
        }
        else
            CRASH_MSG("Compression type not expected!");
        if (dstLen != block.uncomprSize)
        {
            failed = true;
        }
    }

    int dstPos = 0;
    foreach (Package::ChunkBlock block, blocks)
    {
        if (!failed)
        {
            memcpy(data.ptr() + dstPos, block.uncompressedBuffer, block.uncomprSize);
            dstPos += block.uncomprSize;
        }
        delete[] block.uncompressedBuffer;
        delete[] block.compressedBuffer;
    }

    if (failed)
        return ByteBuffer{};

    return data;
}
