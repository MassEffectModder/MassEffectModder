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

#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <Wrappers.h>
#include <GameData/GameData.h>
#include <GameData/Package.h>
#include <Program/ConfigIni.h>
#include <Types/MemTypes.h>

Package::~Package()
{
    for (int i = 0; i < exportsTable.count(); i++)
    {
        exportsTable[i].raw.Free();
        exportsTable[i].newData.Free();
    }
    for (int i = 0; i < importsTable.count(); i++)
    {
        importsTable[i].raw.Free();
    }
    for (int i = 0; i < extraNamesTable.count(); i++)
    {
        extraNamesTable[i].raw.Free();
    }

    DisposeCache();
    ReleaseChunks();
    delete[] packageHeader;
    delete packageData;
    delete packageStream;
}

bool Package::isName(int id)
{
    return id >= 0 && id < namesTable.count();
}

QString Package::getClassName(int id)
{
    if (id > 0 && id < exportsTable.count())
        return exportsTable[id - 1].objectName;
    if (id < 0 && -id < importsTable.count())
        return importsTable[-id - 1].objectName;
    return "Class";
}

int Package::getClassNameId(int id)
{
    if (id > 0 && id < exportsTable.count())
        return exportsTable[id - 1].getObjectNameId();
    if (id < 0 && -id < importsTable.count())
        return importsTable[-id - 1].objectNameId;
    return 0;
}

QString Package::resolvePackagePath(int id)
{
    QString s = "";
    if (id > 0 && id < exportsTable.count())
    {
        s += resolvePackagePath(exportsTable[id - 1].getLinkId());
        if (s.length() != 0)
            s += ".";
        s += exportsTable[id - 1].objectName;
    }
    else if (id < 0 && -id < importsTable.count())
    {
        s += resolvePackagePath(importsTable[-id - 1].linkId);
        if (s.length() != 0)
            s += ".";
        s += importsTable[-id - 1].objectName;
    }
    return s;
}

int Package::Open(const QString &filename, bool headerOnly, bool fullLoad)
{
    packagePath = g_GameData->RelativeGameData(filename);

    if (!QFile(filename).exists())
    {
        PERROR(QString("Package not found: %1\n").arg(filename));
        return -1;
    }
    if (QFileInfo(filename).size() == 0)
    {
        PERROR(QString("Package file has 0 length: %1\n").arg(filename));
        return -1;
    }
    if (QFileInfo(filename).size() < packageHeaderSize684)
    {
        PERROR(QString("Broken package header in: %1\n").arg(filename));
        return -1;
    }

    packageStream = new FileStream(filename, FileMode::Open, FileAccess::ReadOnly);
    if (packageStream->ReadUInt32() != DataTag)
    {
        delete packageStream;
        packageStream = nullptr;
        PERROR(QString("Wrong PCC tag: %1\n").arg(filename));
        return -1;
    }
    ushort ver = packageStream->ReadUInt16();
    if (ver == packageFileVersion684)
    {
        packageHeaderSize = packageHeaderSize684;
        packageFileVersion = packageFileVersion684;
    }
    else if (ver == packageFileVersion685)
    {
        packageHeaderSize = packageHeaderSize685;
        packageFileVersion = packageFileVersion685;
    }
    else
    {
        delete packageStream;
        packageStream = nullptr;
        PERROR(QString("Wrong PCC version in file: %1\n").arg(filename));
        return -1;
    }

    packageHeader = new quint8[packageHeaderSize];
    if (packageHeader == nullptr)
        CRASH_MSG((QString("Out of memory! - amount: ") + QString::number(packageHeaderSize)).toStdString().c_str());
    packageStream->SeekBegin();
    packageStream->ReadToBuffer(packageHeader, packageHeaderSize);

    compressionType = (CompressionType)packageStream->ReadUInt32();

    if (headerOnly)
        return 0;

    numChunks = packageStream->ReadUInt32();

    chunksTableOffset = packageStream->Position();

    if (getCompressedFlag())
    {
        for (uint i = 0; i < numChunks; i++)
        {
            Chunk chunk{};
            chunk.uncomprOffset = packageStream->ReadUInt32();
            chunk.uncomprSize = packageStream->ReadUInt32();
            chunk.comprOffset = packageStream->ReadUInt32();
            chunk.comprSize = packageStream->ReadUInt32();
            chunks.push_back(chunk);
        }
    }
    long afterChunksTable = packageStream->Position();
    someTag = packageStream->ReadUInt32();

    loadExtraNames(*packageStream);

    dataOffset = chunksTableOffset + (packageStream->Position() - afterChunksTable);

    if (getCompressedFlag())
    {
        if (packageStream->Position() != chunks[0].comprOffset)
            CRASH();

        if ((uint)dataOffset != chunks[0].uncomprOffset)
            CRASH();

        uint length = getEndOfTablesOffset() - (uint)dataOffset;
        packageData = new MemoryStream();
        packageData->JumpTo(dataOffset);
        if (!getData((uint)dataOffset, length, packageData))
        {
            PERROR(QString("Failed get data! %1\n").arg(filename));
            return -1;
        }
    }

    if (getCompressedFlag())
        loadNames(*packageData);
    else
        loadNames(*packageStream);

    if (getEndOfTablesOffset() < getNamesOffset())
    {
        if (getCompressedFlag()) // allowed only uncompressed
            CRASH();
    }

    if (getCompressedFlag())
        loadImports(*packageData);
    else
        loadImports(*packageStream);

    if (getEndOfTablesOffset() < getImportsOffset())
    {
        if (getCompressedFlag()) // allowed only uncompressed
            CRASH();
    }

    if (getCompressedFlag())
        loadExports(*packageData);
    else
        loadExports(*packageStream);

    if (getCompressedFlag())
        loadDepends(*packageData);
    else
        loadDepends(*packageStream);

    if (getCompressedFlag())
        loadGuids(*packageData);
    else
        loadGuids(*packageStream);

    if (fullLoad)
    {
        loadImportsNames();
        loadExportsNames();
    }

    return 0;
}

bool Package::getData(uint offset, uint length, Stream *outputStream, quint8 *outputBuffer)
{
    if (getCompressedFlag())
    {
        uint pos = 0;
        uint bytesLeft = length;
        for (int c = 0; c < chunks.count(); c++)
        {
            Chunk chunk = chunks[c];
            if (chunk.uncomprOffset + chunk.uncomprSize <= offset)
                continue;
            uint startInChunk;
            if (offset < chunk.uncomprOffset)
                startInChunk = 0;
            else
                startInChunk = offset - chunk.uncomprOffset;

            uint bytesLeftInChunk = qMin(chunk.uncomprSize - startInChunk, bytesLeft);
            if (currentChunk != c)
            {
                delete chunkCache;
                chunkCache = new MemoryStream();
                currentChunk = c;
                packageStream->JumpTo(chunk.comprOffset);
                uint blockTag = packageStream->ReadUInt32(); // block tag
                if (blockTag != DataTag)
                    CRASH();
                uint blockSize = packageStream->ReadUInt32(); // max block size
                if (blockSize != MaxBlockSize)
                    CRASH();
                uint compressedChunkSize = packageStream->ReadUInt32(); // compressed chunk size
                uint uncompressedChunkSize = packageStream->ReadUInt32();
                if (uncompressedChunkSize != chunk.uncomprSize)
                    CRASH();

                uint blocksCount = (uncompressedChunkSize + MaxBlockSize - 1) / MaxBlockSize;
                if ((compressedChunkSize + SizeOfChunk + SizeOfChunkBlock * blocksCount) != chunk.comprSize)
                    CRASH();

                QList<ChunkBlock> blocks;
                for (uint b = 0; b < blocksCount; b++)
                {
                    ChunkBlock block{};
                    block.comprSize = packageStream->ReadUInt32();
                    block.uncomprSize = packageStream->ReadUInt32();
                    blocks.push_back(block);
                }
                chunk.blocks = blocks;

                for (int b = 0; b < blocks.count(); b++)
                {
                    ChunkBlock block = blocks[b];
                    block.compressedBuffer = new quint8[block.comprSize];
                    if (block.compressedBuffer == nullptr)
                        CRASH_MSG((QString("Out of memory! - amount: ") +
                                   QString::number(block.comprSize)).toStdString().c_str());
                    packageStream->ReadToBuffer(block.compressedBuffer, block.comprSize);
                    block.uncompressedBuffer = new quint8[MaxBlockSize * 2];
                    if (block.uncompressedBuffer == nullptr)
                        CRASH_MSG((QString("Out of memory! - amount: ") +
                                   QString::number(MaxBlockSize * 2)).toStdString().c_str());
                    blocks.replace(b, block);
                }

                bool failed = false;
                if (compressionType == CompressionType::LZO)
                {
                    for (int b = 0; b < blocks.count(); b++)
                    {
                        const ChunkBlock& block = blocks[b];
                        uint dstLen = MaxBlockSize * 2;
                        LzoDecompress(block.compressedBuffer, block.comprSize, block.uncompressedBuffer, &dstLen);
                        if (dstLen != block.uncomprSize)
                            failed = true;
                    }
                }
                else if (compressionType == CompressionType::Zlib)
                {
                    #pragma omp parallel for
                    for (int b = 0; b < blocks.count(); b++)
                    {
                        const ChunkBlock& block = blocks[b];
                        uint dstLen = MaxBlockSize * 2;
                        if (ZlibDecompress(block.compressedBuffer, block.comprSize, block.uncompressedBuffer, &dstLen) == -100)
                            CRASH_MSG("Out of memory!");
                        if (dstLen != block.uncomprSize)
                            failed = true;
                    }
                }
                else if (compressionType == CompressionType::Oddle)
                {
                    #pragma omp parallel for
                    for (int b = 0; b < blocks.count(); b++)
                    {
                        const ChunkBlock& block = blocks[b];
                        if (OodleDecompress(block.compressedBuffer, block.comprSize, block.uncompressedBuffer, block.uncomprSize) != 0)
                            failed = true;
                    }
                }
                else
                    CRASH_MSG("Compression type not expected!");

                for (int b = 0; b < blocks.count(); b++)
                {
                    ChunkBlock block = blocks[b];
                    if (!failed)
                    {
                        chunkCache->WriteFromBuffer(block.uncompressedBuffer, block.uncomprSize);
                        blocks[b] = block;
                    }
                    delete[] block.compressedBuffer;
                    delete[] block.uncompressedBuffer;
                }
                if (failed)
                    return false;
            }
            chunkCache->JumpTo(startInChunk);
            if (outputStream)
                outputStream->CopyFrom(*chunkCache, bytesLeftInChunk);
            if (outputBuffer)
                chunkCache->ReadToBuffer(outputBuffer + pos, bytesLeftInChunk);
            pos += bytesLeftInChunk;
            bytesLeft -= bytesLeftInChunk;
            if (bytesLeft == 0)
                break;
        }
    }
    else
    {
        packageStream->JumpTo(offset);
        if (outputStream)
            outputStream->CopyFrom(*packageStream, length);
        if (outputBuffer)
            packageStream->ReadToBuffer(outputBuffer, length);
    }

    return true;
}

ByteBuffer Package::getExportData(int id)
{
    ExportEntry& exp = exportsTable[id];
    uint length = exp.getDataSize();
    auto data = ByteBuffer(length);
    if (exp.newData.ptr() != nullptr)
        memcpy(data.ptr(), exp.newData.ptr(), length);
    else
    {
        if (!getData(exp.getDataOffset(), exp.getDataSize(), nullptr, data.ptr()))
        {
            data.Free();
            return {};
        }
    }

    return data;
}

void Package::setExportData(int id, const ByteBuffer &data)
{
    ExportEntry exp = exportsTable[id];
    if (data.size() > exp.getDataSize())
    {
        exp.setDataOffset(exportsEndOffset);
        exportsEndOffset = exp.getDataOffset() + data.size();
    }
    exp.setDataSize(data.size());
    exp.newData.Free();
    exp.newData = ByteBuffer(data.ptr(), data.size());
    exportsTable.replace(id, exp);
    modified = true;
}

void Package::MoveExportDataToEnd(int id)
{
    ByteBuffer data = getExportData(id);
    ExportEntry exp = exportsTable[id];
    exp.setDataOffset(exportsEndOffset);
    exportsEndOffset = exp.getDataOffset() + exp.getDataSize();

    exp.newData.Free();
    exp.newData = data;
    exportsTable.replace(id, exp);
    modified = true;
}

static bool compareExportsDataOffset(Package::ExportEntry &e1, Package::ExportEntry &e2)
{
    return e1.getDataOffset() < e2.getDataOffset();
}

void Package::SortExportsTableByDataOffset(const QList<ExportEntry> &list, QList<ExportEntry> &sortedExports)
{
    sortedExports = list;
    std::sort(sortedExports.begin(), sortedExports.end(), compareExportsDataOffset);
}

bool Package::ReserveSpaceBeforeExportData(int space)
{
    QList<ExportEntry> sortedExports;
    SortExportsTableByDataOffset(exportsTable, sortedExports);
    if (getEndOfTablesOffset() > sortedExports.first().getDataOffset())
        CRASH();
    int expandDataSize = sortedExports.first().getDataOffset() - getEndOfTablesOffset();
    if (expandDataSize >= space)
        return true;
    bool dryRun = true;
    for (int i = 0; i < sortedExports.count(); i++)
    {
        if (sortedExports[i].objectName == "SeekFreeShaderCache" &&
            getClassName(sortedExports[i].getClassId()) == "ShaderCache")
        {
            return false;
        }
        expandDataSize += sortedExports[i].getDataSize();
        if (!dryRun)
            MoveExportDataToEnd((int)sortedExports[i].id);
        if (expandDataSize >= space)
        {
            if (!dryRun)
                return true;
            expandDataSize = sortedExports.first().getDataOffset() - getEndOfTablesOffset();
            i = -1;
            dryRun = false;
        }
    }

    return false;
}

const QString Package::StorageTypeToString(StorageTypes type)
{
    switch (type)
    {
    case StorageTypes::pccUnc:
        return "pccUnc";
    case StorageTypes::pccLZO:
        return "pccLZO";
    case StorageTypes::pccZlib:
        return "pccZlib";
    case StorageTypes::pccOodle:
        return "pccOodle";
    case StorageTypes::extUnc:
        return "extUnc";
    case StorageTypes::extLZO:
        return "extLZO";
    case StorageTypes::extZlib:
        return "extZlib";
    case StorageTypes::extOodle:
        return "extOodle";
    case StorageTypes::empty:
        return "empty";
    default:
        CRASH();
    }
}

int Package::getNameId(const QString &name)
{
    for (int i = 0; i < namesTable.count(); i++)
    {
        if (namesTable[i].name == name)
            return i;
    }
    CRASH();
}

bool Package::existsNameId(const QString &name)
{
    for (int i = 0; i < namesTable.count(); i++)
    {
        if (namesTable[i].name == name)
            return true;
    }
    return false;
}

QString Package::getName(int id)
{
    if (id >= namesTable.count())
        CRASH();
    return namesTable[id].name;
}

int Package::addName(const QString &name)
{
    if (existsNameId(name))
        CRASH();

    NameEntry entry{};
    entry.name = name;
    namesTable.push_back(entry);
    setNamesCount(namesTable.count());
    namesTableModified = true;
    modified = true;
    return namesTable.count() - 1;
}

void Package::loadNames(Stream &input)
{
    input.JumpTo(getNamesOffset());
    for (uint i = 0; i < getNamesCount(); i++)
    {
        NameEntry entry{};
        int len = input.ReadInt32();
        if (len < 0) // unicode
        {
            entry.name = "";
            if (packageFileVersion == packageFileVersion685)
            {
                for (int n = 0; n < -len; n++)
                {
                    quint16 c = input.ReadUInt16();
                    entry.name += QChar(static_cast<ushort>(c));
                }
            }
            else
            {
                for (int n = 0; n < -len; n++)
                {
                    quint8 c = input.ReadByte();
                    input.ReadByte();
                    entry.name += (char)c;
                }
            }
        }
        else
        {
            input.ReadStringASCII(entry.name, len);
        }
        if (entry.name.endsWith(QChar('\0')))
            entry.name.chop(1);

        if (nameIdTexture2D == -1 && entry.name == "Texture2D")
            nameIdTexture2D = i;
        else if (nameIdLightMapTexture2D == -1 && entry.name == "LightMapTexture2D")
            nameIdLightMapTexture2D = i;
        else if (nameIdShadowMapTexture2D == -1 && entry.name == "ShadowMapTexture2D")
            nameIdShadowMapTexture2D = i;
        else if (nameIdTextureFlipBook == -1 && entry.name == "TextureFlipBook")
            nameIdTextureFlipBook = i;
        else if (nameIdTextureFlipBook == -1 && entry.name == "TextureMovie")
            nameIdTextureMovie = i;

        namesTable.push_back(entry);
    }
    namesTableEnd = input.Position();
}

void Package::saveNames(Stream &output)
{
    if (!namesTableModified)
    {
        if (getCompressedFlag())
        {
            packageData->JumpTo(getNamesOffset());
            output.CopyFrom(*packageData, namesTableEnd - getNamesOffset());
        }
        else
        {
            packageStream->JumpTo(getNamesOffset());
            output.CopyFrom(*packageStream, namesTableEnd - getNamesOffset());
        }
    }
    else
    {
        for (int i = 0; i < namesTable.count(); i++)
        {
            NameEntry entry = namesTable[i];
            if (entry.name.length() == 0)
            {
                output.WriteInt32(0);
            }
            else if (packageFileVersion == packageFileVersion685)
            {
                output.WriteInt32(-(entry.name.length() + 1));
                output.WriteStringUnicode16Null(entry.name);
            }
            else
            {
                output.WriteInt32(entry.name.length() + 1);
                output.WriteStringASCIINull(entry.name);
            }
        }
    }
}

void Package::loadExtraNames(Stream &input, bool rawMode)
{
    ExtraNameEntry entry{};
    uint extraNamesCount = input.ReadUInt32();
    for (uint c = 0; c < extraNamesCount; c++)
    {
        int len = input.ReadInt32();
        if (rawMode)
        {
            if (len < 0)
            {
                entry.raw = ByteBuffer(-len * 2);
                input.ReadToBuffer(entry.raw.ptr(), entry.raw.size());
            }
            else
            {
                entry.raw = ByteBuffer(len);
                input.ReadToBuffer(entry.raw.ptr(), len);
            }
        }
        else
        {
            QString name;
            if (len < 0)
            {
                input.ReadStringUnicode16(name, -len * 2);
            }
            else
            {
                input.ReadStringASCII(name, len);
            }
            entry.name = name;
            if (entry.name.endsWith(QChar('\0')))
                entry.name.chop(1);
        }
        extraNamesTable.push_back(entry);
    }
}

void Package::saveExtraNames(Stream &output, bool rawMode)
{
    output.WriteInt32(extraNamesTable.count());
    for (int c = 0; c < extraNamesTable.count(); c++)
    {
        if (rawMode)
        {
            if (packageFileVersion == packageFileVersion685)
                output.WriteInt32(-(extraNamesTable[c].raw.size() / 2));
            else
                output.WriteInt32(extraNamesTable[c].raw.size());
            output.WriteFromBuffer(extraNamesTable[c].raw.ptr(), extraNamesTable[c].raw.size());
        }
        else
        {
            if (extraNamesTable[c].name.length() == 0)
            {
                output.WriteInt32(0);
            }
            else if (packageFileVersion == packageFileVersion685)
            {
                output.WriteInt32(-(extraNamesTable[c].name.length() + 1));
                output.WriteStringUnicode16Null(extraNamesTable[c].name);
            }
            else
            {
                output.WriteInt32(extraNamesTable[c].name.length() + 1);
                output.WriteStringASCIINull(extraNamesTable[c].name);
            }
        }
    }
}

void Package::loadImports(Stream &input)
{
    input.JumpTo(getImportsOffset());
    for (uint i = 0; i < getImportsCount(); i++)
    {
        ImportEntry entry{};

        long start = input.Position();
        entry.packageFileId = input.ReadInt32();
        entry.packageFile = namesTable[entry.packageFileId].name;
        input.SkipInt32(); // const 0
        entry.classId = input.ReadInt32();
        input.SkipInt32(); // const 0
        entry.linkId = input.ReadInt32();
        entry.objectNameId = input.ReadInt32();
        entry.objectName = namesTable[entry.objectNameId].name;
        input.SkipInt32();

        long len = input.Position() - start;
        input.JumpTo(start);
        entry.raw = ByteBuffer(len);
        input.ReadToBuffer(entry.raw.ptr(), len);

        importsTable.push_back(entry);
    }
    importsTableEnd = input.Position();
}

void Package::loadImportsNames()
{
    for (uint i = 0; i < getImportsCount(); i++)
    {
        ImportEntry entry = importsTable[i];
        entry.className = getClassName(entry.classId);
        importsTable[i] = entry;
    }
}

void Package::saveImports(Stream &output)
{
    if (!importsTableModified)
    {
        if (getCompressedFlag())
        {
            packageData->JumpTo(getImportsOffset());
            output.CopyFrom(*packageData, importsTableEnd - getImportsOffset());
        }
        else
        {
            packageStream->JumpTo(getImportsOffset());
            output.CopyFrom(*packageStream, importsTableEnd - getImportsOffset());
        }
    }
    else
    {
        for (int i = 0; i < importsTable.count(); i++)
        {
            output.WriteFromBuffer(importsTable[i].raw.ptr(), importsTable[i].raw.size());
        }
    }
}

void Package::loadExports(Stream &input)
{
    input.JumpTo(getExportsOffset());
    for (uint i = 0; i < getExportsCount(); i++)
    {
        ExportEntry entry{};

        long start = input.Position();
        input.Skip(entry.DataOffsetOffset + 4);
        input.SkipInt32();
        input.Skip(input.ReadUInt32() * 4 + 16 + 4); // skip entries + skip guid + some

        long len = input.Position() - start;
        input.JumpTo(start);
        entry.raw = ByteBuffer(len);
        entry.newData = ByteBuffer();
        input.ReadToBuffer(entry.raw.ptr(), len);

        if ((entry.getDataOffset() + entry.getDataSize()) > exportsEndOffset)
            exportsEndOffset = entry.getDataOffset() + entry.getDataSize();

        entry.objectName = namesTable[entry.getObjectNameId()].name;
        entry.id = i;
        exportsTable.push_back(entry);
    }
}

void Package::loadExportsNames()
{
    for (uint i = 0; i < getExportsCount(); i++)
    {
        ExportEntry entry = exportsTable[i];
        entry.className = getClassName(entry.getClassId());
        exportsTable[i] = entry;
    }
}

void Package::saveExports(Stream &output)
{
    for (int i = 0; i < exportsTable.count(); i++)
    {
        output.WriteFromBuffer(exportsTable[i].raw.ptr(), exportsTable[i].raw.size());
    }
}

void Package::loadDepends(Stream &input)
{
    input.JumpTo(getDependsOffset());
    for (uint i = 0; i < getExportsCount(); i++)
    {
        dependsTable.push_back(input.ReadInt32());
    }
}

void Package::saveDepends(Stream &output)
{
    for (int i = 0; i < dependsTable.count(); i++)
        output.WriteInt32(dependsTable[i]);
}

void Package::loadGuids(Stream &input)
{
    input.JumpTo(getGuidsOffset());
    for (uint i = 0; i < getGuidsCount(); i++)
    {
        GuidEntry entry{};
        input.ReadToBuffer(entry.guid, 16);
        entry.index = input.ReadInt32();
        guidsTable.push_back(entry);
    }
}

void Package::saveGuids(Stream &output)
{
    for (int i = 0; i < guidsTable.count(); i++)
    {
        GuidEntry entry = guidsTable[i];
        output.WriteFromBuffer(entry.guid, 16);
        output.WriteInt32(entry.index);
    }
}

bool Package::SaveToFile(bool forceCompressed, bool forceDecompressed, bool appendMarker)
{
    if (!modified && !forceDecompressed && !forceCompressed)
        return false;

    if (forceCompressed && forceDecompressed)
        CRASH_MSG("force de/compression can't be both enabled!");

    CompressionType targetCompression = compressionType;
    if (forceCompressed)
        targetCompression = CompressionType::Zlib;

    if (!appendMarker)
    {
        QString marker;
        packageStream->Seek(-MEMMarkerLength, SeekOrigin::End);
        packageStream->ReadStringASCII(marker, MEMMarkerLength);
        if (marker == QString(MEMendFileMarker))
            appendMarker = true;
    }

    MemoryStream tempOutput;
    tempOutput.WriteFromBuffer(packageHeader, packageHeaderSize);
    tempOutput.WriteUInt32(targetCompression);
    tempOutput.WriteUInt32(0); // number of chunks - filled later if needed
    tempOutput.WriteUInt32(someTag);
    saveExtraNames(tempOutput);
    dataOffset = tempOutput.Position();

    QList<ExportEntry> sortedExports;
    SortExportsTableByDataOffset(exportsTable, sortedExports);

    setDependsOffset(tempOutput.Position());
    saveDepends(tempOutput);
    if (tempOutput.Position() > sortedExports[0].getDataOffset())
        CRASH();

    setGuidsOffset(tempOutput.Position());
    saveGuids(tempOutput);
    if (tempOutput.Position() > sortedExports[0].getDataOffset())
        CRASH();

    bool spaceForNamesAvailable = true;
    bool spaceForImportsAvailable = true;
    bool spaceForExportsAvailable = true;

    setEndOfTablesOffset(tempOutput.Position());
    long namesOffsetTmp = tempOutput.Position();
    saveNames(tempOutput);
    if (tempOutput.Position() > sortedExports[0].getDataOffset())
    {
        if (ReserveSpaceBeforeExportData((int)(tempOutput.Position() - getEndOfTablesOffset())))
        {
            tempOutput.JumpTo(namesOffsetTmp);
            saveNames(tempOutput);
            SortExportsTableByDataOffset(exportsTable, sortedExports);
        }
        else
        {
            spaceForNamesAvailable = false;
        }
    }
    if (spaceForNamesAvailable)
    {
        setNamesOffset(namesOffsetTmp);

        setEndOfTablesOffset(tempOutput.Position());
        long importsOffsetTmp = tempOutput.Position();
        saveImports(tempOutput);
        if (tempOutput.Position() > sortedExports[0].getDataOffset())
        {
            if (ReserveSpaceBeforeExportData((int)(tempOutput.Position() - getEndOfTablesOffset())))
            {
                tempOutput.JumpTo(importsOffsetTmp);
                saveImports(tempOutput);
                SortExportsTableByDataOffset(exportsTable, sortedExports);
            }
            else
            {
                spaceForImportsAvailable = false;
            }
        }
        if (spaceForImportsAvailable)
        {
            setImportsOffset(importsOffsetTmp);

            setEndOfTablesOffset(tempOutput.Position());
            long exportsOffsetTmp = tempOutput.Position();
            saveExports(tempOutput);
            if (tempOutput.Position() > sortedExports[0].getDataOffset())
            {
                if (ReserveSpaceBeforeExportData((int)(tempOutput.Position() - getEndOfTablesOffset())))
                {
                    tempOutput.JumpTo(exportsOffsetTmp);
                    saveExports(tempOutput);
                }
                else
                {
                    spaceForExportsAvailable = false;
                }
            }
            if (spaceForExportsAvailable)
            {
                setExportsOffset(exportsOffsetTmp);
            }
        }
    }

    SortExportsTableByDataOffset(exportsTable, sortedExports);

    setEndOfTablesOffset(sortedExports[0].getDataOffset());

    for (uint i = 0; i < getExportsCount(); i++)
    {
        ExportEntry& exp = sortedExports[i];
        uint dataLeft;
        tempOutput.JumpTo(exp.getDataOffset());
        if (i + 1 == getExportsCount())
            dataLeft = exportsEndOffset - exp.getDataOffset() - exp.getDataSize();
        else
            dataLeft = sortedExports[i + 1].ExportEntry::getDataOffset() - exp.getDataOffset() - exp.getDataSize();
        if (exp.newData.ptr() != nullptr)
        {
            tempOutput.WriteFromBuffer(exp.newData);
        }
        else
        {
            if (!getData(exp.getDataOffset(), exp.getDataSize(), &tempOutput))
            {
                CRASH_MSG("Failed get data!");
            }
        }
        tempOutput.WriteZeros(dataLeft);
    }

    tempOutput.JumpTo(exportsEndOffset);

    if (!spaceForNamesAvailable)
    {
        long tmpPos = tempOutput.Position();
        saveNames(tempOutput);
        setNamesOffset(tmpPos);
    }

    if (!spaceForImportsAvailable)
    {
        long tmpPos = tempOutput.Position();
        saveImports(tempOutput);
        setImportsOffset(tmpPos);
    }

    if (!spaceForExportsAvailable)
    {
        setExportsOffset(tempOutput.Position());
        saveExports(tempOutput);
    }

    if ((forceDecompressed && getCompressedFlag()) ||
        !spaceForNamesAvailable ||
        !spaceForImportsAvailable ||
        !spaceForExportsAvailable)
    {
        setCompressedFlag(false);
    }

    if (forceCompressed && !getCompressedFlag())
    {
        if (spaceForNamesAvailable &&
            spaceForImportsAvailable &&
            spaceForExportsAvailable)
        {
            setCompressedFlag(true);
        }
        else
        {
            if (!modified)
                return false;
        }
    }

    tempOutput.SeekBegin();
    tempOutput.WriteFromBuffer(packageHeader, packageHeaderSize);

    packageStream->Close();

    std::unique_ptr<FileStream> fs (new FileStream(g_GameData->GamePath() + packagePath,
                                                   FileMode::Create, FileAccess::WriteOnly));
    if (fs == nullptr)
        CRASH_MSG(QString("Failed to write to file: %1").arg(packagePath).toStdString().c_str());

    if (!getCompressedFlag())
    {
        tempOutput.SeekBegin();
        fs->CopyFrom(tempOutput, tempOutput.Length());
    }
    else
    {
        ReleaseChunks();

        Chunk chunk{};
        chunk.uncomprSize = sortedExports.first().getDataOffset() - dataOffset;
        chunk.uncomprOffset = (uint)dataOffset;
        for (uint i = 0; i < getExportsCount(); i++)
        {
            ExportEntry& exp = sortedExports[i];
            uint dataSize;
            if (i + 1 == getExportsCount())
                dataSize = exportsEndOffset - exp.getDataOffset();
            else
                dataSize = sortedExports[i + 1].ExportEntry::getDataOffset() - exp.getDataOffset();
            if (chunk.uncomprSize + dataSize > MaxChunkSize)
            {
                uint offset = chunk.uncomprOffset + chunk.uncomprSize;
                chunks.push_back(chunk);
                chunk.uncomprSize = dataSize;
                chunk.uncomprOffset = offset;
            }
            else
            {
                chunk.uncomprSize += dataSize;
            }
        }
        chunks.push_back(chunk);

        fs->WriteFromBuffer(packageHeader, packageHeaderSize);
        fs->WriteUInt32(targetCompression);
        fs->WriteUInt32(chunks.count());
        fs->Skip(SizeOfChunk * chunks.count()); // skip chunks table - filled later
        fs->WriteUInt32(someTag);
        saveExtraNames(*fs);

#ifdef GUI
        QElapsedTimer timer;
        timer.start();
#endif
        for (int c = 0; c < chunks.count(); c++)
        {
#ifdef GUI
            if (timer.elapsed() > 100)
            {
                QApplication::processEvents();
                timer.restart();
            }
#endif
            chunk = chunks[c];
            chunk.comprOffset = fs->Position();
            chunk.comprSize = 0; // filled later

            uint dataBlockLeft = chunk.uncomprSize;
            uint newNumBlocks = (chunk.uncomprSize + MaxBlockSize - 1) / MaxBlockSize;
            // skip blocks header and table - filled later
            fs->Seek(SizeOfChunk + SizeOfChunkBlock * newNumBlocks, SeekOrigin::Current);

            tempOutput.JumpTo(chunk.uncomprOffset);

            for (uint b = 0; b < newNumBlocks; b++)
            {
                ChunkBlock block{};
                block.uncomprSize = qMin((uint)MaxBlockSize, dataBlockLeft);
                dataBlockLeft -= block.uncomprSize;
                block.uncompressedBuffer = new quint8[block.uncomprSize];
                if (block.uncompressedBuffer == nullptr)
                    CRASH_MSG((QString("Out of memory! - amount: ") +
                               QString::number(block.uncomprSize)).toStdString().c_str());
                tempOutput.ReadToBuffer(block.uncompressedBuffer, block.uncomprSize);
                chunk.blocks.push_back(block);
            }

            #pragma omp parallel for
            for (int b = 0; b < chunk.blocks.count(); b++)
            {
                ChunkBlock block = chunk.blocks[b];
                if (targetCompression == CompressionType::LZO)
                {
                    if (LzoCompress(block.uncompressedBuffer, block.uncomprSize, &block.compressedBuffer, &block.comprSize) == -100)
                        CRASH_MSG("Out of memory!");
                }
                else if (targetCompression == CompressionType::Zlib)
                {
                    if (ZlibCompress(block.uncompressedBuffer, block.uncomprSize, &block.compressedBuffer, &block.comprSize,
                                     forceCompressed ? 9 : 1) == -100)
                        CRASH_MSG("Out of memory!");
                }
                else if (targetCompression == CompressionType::Oddle)
                {
                    if (OodleCompress(block.uncompressedBuffer, block.uncomprSize, &block.compressedBuffer, &block.comprSize) != 0)
                        CRASH_MSG("Compression failed!");
                }
                else
                    CRASH_MSG("Compression type not expected!");
                if (block.comprSize == 0)
                    CRASH_MSG("Compression failed!");
                chunk.blocks.replace(b, block);
            }

            for (uint b = 0; b < newNumBlocks; b++)
            {
                ChunkBlock block = chunk.blocks[b];
                fs->WriteFromBuffer(block.compressedBuffer, block.comprSize);
                chunk.comprSize += block.comprSize;
            }
            chunks[c] = chunk;
        }

        for (int c = 0; c < chunks.count(); c++)
        {
            const Chunk& chunk = chunks[c];
            fs->JumpTo(chunksTableOffset + c * SizeOfChunk); // jump to chunks table
            fs->WriteUInt32(chunk.uncomprOffset);
            fs->WriteUInt32(chunk.uncomprSize);
            fs->WriteUInt32(chunk.comprOffset);
            fs->WriteUInt32(chunk.comprSize + SizeOfChunk + SizeOfChunkBlock * chunk.blocks.count());
            fs->JumpTo(chunk.comprOffset); // jump to blocks header
            fs->WriteUInt32(DataTag);
            fs->WriteUInt32(MaxBlockSize);
            fs->WriteUInt32(chunk.comprSize);
            fs->WriteUInt32(chunk.uncomprSize);
            for (int b = 0; b < chunk.blocks.count(); b++)
            {
                const ChunkBlock& block = chunk.blocks[b];
                fs->WriteUInt32(block.comprSize);
                fs->WriteUInt32(block.uncomprSize);
                delete[] block.compressedBuffer;
                delete[] block.uncompressedBuffer;
            }
        }
        chunks.clear();
    }

    if (appendMarker)
    {
        fs->SeekEnd();
        QString str(MEMendFileMarker);
        fs->WriteStringASCII(str);
    }

    return true;
}

void Package::ReleaseChunks()
{
    for (int c = 0; c < chunks.count(); c++)
    {
        const Chunk& chunk = chunks[c];
        for (int b = 0; b < chunk.blocks.count(); b++)
        {
            const ChunkBlock& block = chunk.blocks[b];
            delete[] block.compressedBuffer;
            delete[] block.uncompressedBuffer;
        }
    }
    chunks.clear();
}

const ByteBuffer Package::compressData(const ByteBuffer &inputData, StorageTypes type, bool maxCompress)
{
    MemoryStream ouputStream;
    qint64 compressedSize = 0;
    uint dataBlockLeft = inputData.size();
    uint newNumBlocks = (inputData.size() + MaxBlockSize - 1) / MaxBlockSize;
    QList<Package::ChunkBlock> blocks{};
    {
        auto inputStream = MemoryStream(inputData);
        // skip blocks header and table - filled later
        ouputStream.Seek(SizeOfChunk + SizeOfChunkBlock * newNumBlocks, SeekOrigin::Begin);

        for (uint b = 0; b < newNumBlocks; b++)
        {
            Package::ChunkBlock block{};
            block.uncomprSize = qMin((uint)MaxBlockSize, dataBlockLeft);
            dataBlockLeft -= block.uncomprSize;
            block.uncompressedBuffer = new quint8[block.uncomprSize];
            if (block.uncompressedBuffer == nullptr)
                CRASH_MSG((QString("Out of memory! - amount: ") +
                           QString::number(block.uncomprSize)).toStdString().c_str());
            inputStream.ReadToBuffer(block.uncompressedBuffer, block.uncomprSize);
            blocks.push_back(block);
        }
    }

    #pragma omp parallel for
    for (int b = 0; b < blocks.count(); b++)
    {
        Package::ChunkBlock block = blocks[b];
        if (type == StorageTypes::extLZO || type == StorageTypes::pccLZO)
        {
            if (LzoCompress(block.uncompressedBuffer, block.uncomprSize, &block.compressedBuffer, &block.comprSize) == -100)
                CRASH_MSG("Out of memory!");
        }
        else if (type == StorageTypes::extZlib || type == StorageTypes::pccZlib)
        {
            if (ZlibCompress(block.uncompressedBuffer, block.uncomprSize, &block.compressedBuffer, &block.comprSize,
                             maxCompress ? 9 : 1) == -100)
                CRASH_MSG("Out of memory!");
        }
        else if (type == StorageTypes::extOodle || type == StorageTypes::pccOodle)
        {
            if (OodleCompress(block.uncompressedBuffer, block.uncomprSize, &block.compressedBuffer, &block.comprSize) != 0)
                CRASH_MSG("Compression failed!");
        }
        else
            CRASH_MSG("Compression type not expected!");
        if (block.comprSize == 0)
            CRASH_MSG("Compression failed!");
        blocks[b] = block;
    };

    for (int b = 0; b < blocks.count(); b++)
    {
        const Package::ChunkBlock& block = blocks[b];
        ouputStream.WriteFromBuffer(block.compressedBuffer, block.comprSize);
        compressedSize += block.comprSize;
    }

    ouputStream.SeekBegin();
    ouputStream.WriteUInt32(DataTag);
    ouputStream.WriteUInt32(MaxBlockSize);
    ouputStream.WriteUInt32(compressedSize);
    ouputStream.WriteInt32(inputData.size());
    for (int b = 0; b < blocks.count(); b++)
    {
        const Package::ChunkBlock& block = blocks[b];
        ouputStream.WriteUInt32(block.comprSize);
        ouputStream.WriteUInt32(block.uncomprSize);
        delete[] block.compressedBuffer;
        delete[] block.uncompressedBuffer;
    }

    return ouputStream.ToArray();
}

const ByteBuffer Package::decompressData(Stream &stream, StorageTypes type,
                                         int uncompressedSize, int compressedSize)
{
    auto data = ByteBuffer(uncompressedSize);
    uint blockTag = stream.ReadUInt32();
    if (blockTag != DataTag)
    {
        PERROR(QString("Data tag wrong!\n"));
        return ByteBuffer();
    }
    uint blockSize = stream.ReadUInt32();
    if (blockSize != MaxBlockSize)
    {
        PERROR(QString("Data block size is wrong!\n"));
        return ByteBuffer();
    }
    uint compressedChunkSize = stream.ReadUInt32();
    uint uncompressedChunkSize = stream.ReadUInt32();
    if (uncompressedChunkSize != (uint)uncompressedSize)
    {
        PERROR(QString("Data uncompressed size diffrent than expected!\n"));
        return ByteBuffer();
    }

    uint blocksCount = (uncompressedChunkSize + MaxBlockSize - 1) / MaxBlockSize;
    if ((compressedChunkSize + SizeOfChunk + SizeOfChunkBlock * blocksCount) != (uint)compressedSize)
    {
        PERROR(QString("Data compressed size diffrent than expected!\n"));
        return ByteBuffer();
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
        block.compressedBuffer = new quint8[blocks[b].comprSize];
        if (block.compressedBuffer == nullptr)
            CRASH_MSG((QString("Out of memory! - amount: ") +
                       QString::number(blocks[b].comprSize)).toStdString().c_str());
        stream.ReadToBuffer(block.compressedBuffer, blocks[b].comprSize);
        block.uncompressedBuffer = new quint8[MaxBlockSize * 2];
        if (block.uncompressedBuffer == nullptr)
            CRASH_MSG((QString("Out of memory! - amount: ") +
                       QString::number(MaxBlockSize * 2)).toStdString().c_str());
        blocks[b] = block;
    }

    bool errorFlag = false;
    if (type == StorageTypes::extLZO || type == StorageTypes::pccLZO)
    {
        for (int b = 0; b < blocks.count(); b++)
        {
            uint dstLen = MaxBlockSize * 2;
            Package::ChunkBlock block = blocks[b];
            LzoDecompress(block.compressedBuffer, block.comprSize, block.uncompressedBuffer, &dstLen);
            if (dstLen != block.uncomprSize)
                errorFlag = true;
        }
    }
    else if (type == StorageTypes::extZlib || type == StorageTypes::pccZlib)
    {
        #pragma omp parallel for
        for (int b = 0; b < blocks.count(); b++)
        {
            uint dstLen = MaxBlockSize * 2;
            Package::ChunkBlock block = blocks[b];
            if (ZlibDecompress(block.compressedBuffer, block.comprSize, block.uncompressedBuffer, &dstLen) == -100)
                CRASH_MSG("Out of memory!");
            if (dstLen != block.uncomprSize)
                errorFlag = true;
        }
    }
    else if (type == StorageTypes::extOodle || type == StorageTypes::pccOodle)
    {
        #pragma omp parallel for
        for (int b = 0; b < blocks.count(); b++)
        {
            uint dstLen = MaxBlockSize * 2;
            Package::ChunkBlock block = blocks[b];
            if (OodleDecompress(block.compressedBuffer, block.comprSize, block.uncompressedBuffer, dstLen) != 0)
                errorFlag = true;
            if (dstLen != block.uncomprSize)
                errorFlag = true;
        }
    }
    else
        CRASH_MSG("Compression type not expected!");

    if (errorFlag)
    {
        PERROR(QString("ERROR: Decompressed data size not expected!"));
        return ByteBuffer();
    }

    int dstPos = 0;
    for (int b = 0; b < blocks.count(); b++)
    {
        memcpy(data.ptr() + dstPos, blocks[b].uncompressedBuffer, blocks[b].uncomprSize);
        dstPos += blocks[b].uncomprSize;
        delete[] blocks[b].compressedBuffer;
        delete[] blocks[b].uncompressedBuffer;
    }

    return data;
}

void Package::DisposeCache()
{
    delete chunkCache;
    chunkCache = nullptr;
    currentChunk = -1;
}
