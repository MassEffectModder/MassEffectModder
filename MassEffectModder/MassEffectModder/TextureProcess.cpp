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

#include "MipMaps.h"
#include "Package.h"
#include "Texture.h"
#include "Image.h"
#include "Wrappers.h"
#include "Helpers/FileStream.h"
#include "Helpers/MemoryStream.h"

bool MipMaps::compressData(ByteBuffer inputData, Stream &ouputStream)
{
    uint compressedSize = 0;
    uint dataBlockLeft = inputData.size();
    uint newNumBlocks = ((uint)inputData.size() + maxBlockSize - 1) / maxBlockSize;
    QList<Package::ChunkBlock> blocks{};
    {
        MemoryStream inputStream = MemoryStream(inputData);
        // skip blocks header and table - filled later
        ouputStream.JumpTo(SizeOfChunk + SizeOfChunkBlock * newNumBlocks);

        for (uint b = 0; b < newNumBlocks; b++)
        {
            Package::ChunkBlock block{};
            block.uncomprSize = qMin((uint)maxBlockSize, dataBlockLeft);
            dataBlockLeft -= block.uncomprSize;
            block.uncompressedBuffer = new quint8[block.uncomprSize];
            inputStream.ReadToBuffer(block.uncompressedBuffer, block.uncomprSize);
            blocks.push_back(block);
        }
    }

    bool failed = false;
    #pragma omp parallel for
    for (int b = 0; b < blocks.count(); b++)
    {
        Package::ChunkBlock block = blocks[b];
        ZlibCompress(block.uncompressedBuffer, block.uncomprSize, &block.compressedBuffer, &block.comprSize);
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
    foreach (Package::ChunkBlock block, blocks)
    {
        ouputStream.WriteUInt32(block.comprSize);
        ouputStream.WriteUInt32(block.uncomprSize);
    }

    return true;
}

ByteBuffer MipMaps::decompressData(Stream &stream, long compressedSize)
{
    uint compressedChunkSize = stream.ReadUInt32();
    uint uncompressedChunkSize = stream.ReadUInt32();
    ByteBuffer data = ByteBuffer(uncompressedChunkSize);
    uint blocksCount = (uncompressedChunkSize + maxBlockSize - 1) / maxBlockSize;
    if ((compressedChunkSize + SizeOfChunk + SizeOfChunkBlock * blocksCount) != (uint)compressedSize)
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
        stream.ReadToBuffer(block.compressedBuffer, block.comprSize);
        block.uncompressedBuffer = new quint8[maxBlockSize * 2];
        blocks[b] = block;
    }

    bool failed = false;
    #pragma omp parallel for
    for (int b = 0; b < blocks.count(); b++)
    {
        uint dstLen = Package::maxBlockSize * 2;
        Package::ChunkBlock block = blocks[b];
        ZlibDecompress(block.compressedBuffer, block.comprSize, block.uncompressedBuffer, &dstLen);
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

void MipMaps::extractTextureToPng(QString &outputFile, QString &packagePath, int exportID)
{
    Package package = Package();
    package.Open(packagePath);
    Texture texture = Texture(package, exportID, package.getExportData(exportID));
    PixelFormat format = Image::getPixelFormatType(texture.getProperties().getProperty("Format").valueName);
    Texture::TextureMipMap mipmap = texture.getTopMipmap();
    ByteBuffer data = texture.getRefTopImageData();
    if (data.ptr() != nullptr)
    {
        if (QFile(outputFile).exists())
            QFile(outputFile).remove();
        Image::saveToPng(data.ptr(), mipmap.width, mipmap.height, format, outputFile);
        data.Free();
    }
}
