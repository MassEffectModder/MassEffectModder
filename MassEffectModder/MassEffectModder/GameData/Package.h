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

#ifndef PACKAGE_H
#define PACKAGE_H

#include <Helpers/FileStream.h>
#include <Helpers/MemoryStream.h>

enum StorageFlags
{
    noFlags         = 0,
    externalFile    = 1 << 0,
    compressedZlib  = 1 << 1,
    someFlag1       = 1 << 3,
    compressedLZO   = 1 << 4,
    unused          = 1 << 5,
    compressedOodle = 1 << 12,
};

enum StorageTypes
{
    pccUnc = StorageFlags::noFlags,
    pccZlib = StorageFlags::compressedZlib,
    pccOodle = StorageFlags::compressedOodle,
    extUnc = StorageFlags::externalFile,
    extUnc2 = StorageFlags::externalFile | StorageFlags::someFlag1,
    extZlib = StorageFlags::externalFile | StorageFlags::compressedZlib,
    extOodle = StorageFlags::externalFile | StorageFlags::compressedOodle,
    empty = StorageFlags::externalFile | StorageFlags::unused,
};

class Package
{
public:

    enum PackageEnums
    {
        DataTag = 0x9E2A83C1,
        LEXTag = 0x4C43454C,
        packageFileVersion684 = 684,
        packageFileVersion685 = 685,
    };

    enum PackageHeaderEnums
    {
        packageHeaderSize684 = 117,
        packageHeaderSize685 = 126,

        sizeOfGeneration = 12,

        packageHeaderTagOffset = 0,
        packageHeaderVersionOffset = 4,
        packageHeaderFirstChunkSizeOffset = 8,
        packageHeaderNameSizeOffset = 12,

        packageHeaderNamesCountTableOffset = 0,
        packageHeaderNamesOffsetTabletsOffset = 4,
        packageHeaderExportsCountTableOffset = 8,
        packageHeaderExportsOffsetTableOffset = 12,
        packageHeaderImportsCountTableOffset = 16,
        packageHeaderImportsOffsetTableOffset = 20,
        packageHeaderDependsOffsetTableOffset = 24,
        packageHeaderGuidsOffsetTableOffset = 28,
        packageHeaderGuidsCountTableOffset = 36,
    };

    enum CompressionType
    {
        None = 0,
        Zlib = 1,
        LZO = 2,
        Oddle = 1024
    };

    enum PackageFlags
    {
        compressed = 0x02000000,
    };

    enum ChunkBlockEnums
    {
        SizeOfChunkBlock = 8,
        SizeOfChunk = 16,
        MaxBlockSize = 0x40000, // 256KB
        MaxChunkSize = 0x100000, // 1MB
    };

    enum ObjectFlags
    {
        HasStack = 1 << 57
    };

    struct ChunkBlock
    {
        uint comprSize;
        uint uncomprSize;
        quint8 *compressedBuffer;
        quint8 *uncompressedBuffer;
    };

    struct Chunk
    {
        uint uncomprOffset;
        uint uncomprSize;
        uint comprOffset;
        uint comprSize;
        QList<ChunkBlock> blocks;
    };

    struct NameEntry
    {
        QString name;
        quint64 flags;
    };

    struct ImportEntry
    {
        int packageFileId;
        QString packageFile;
        int classId;
        QString className;
        int linkId;
        int objectNameId;
        QString objectName;
        ByteBuffer raw;
    };

    struct ExportEntry
    {
        enum Offsets {
            ClassIdOffset = 0,
            SuperClassIdOffset = 4,
            LinkIdOffset = 8,
            ObjectNameIdOffset = 12,
            IndexValueOffset = 16,
            ArchetypeOffset = 20,
            ObjectFlagsOffset = 24,
            DataSizeOffset = 32,
            DataOffsetOffset = 36,
        };

        ByteBuffer raw;
        ByteBuffer newData;
        quint64 objectFlags;
        uint id;

        inline int getClassId()
        {
            return *reinterpret_cast<int *>(&raw.ptr()[ClassIdOffset]);
        }
        QString className;
        int classParentId;
        inline int getLinkId()
        {
            return *reinterpret_cast<int *>(&raw.ptr()[LinkIdOffset]);
        }
        inline int getObjectNameId()
        {
            return *reinterpret_cast<int *>(&raw.ptr()[ObjectNameIdOffset]);
        }
        QString objectName;
        int suffixNameId;
        inline uint getDataSize()
        {
            return *reinterpret_cast<int *>(&raw.ptr()[DataSizeOffset]);
        }
        inline void setDataSize(uint size)
        {
            *reinterpret_cast<int *>(&raw.ptr()[DataSizeOffset]) = size;
        }
        inline uint getDataOffset()
        {
            return *reinterpret_cast<int *>(&raw.ptr()[DataOffsetOffset]);
        }
        inline void setDataOffset(uint offset)
        {
            *reinterpret_cast<int *>(&raw.ptr()[DataOffsetOffset]) = offset;
        }
    };

    struct GuidEntry
    {
        quint8 guid[16];
        int index;
    };

    struct ExtraNameEntry
    {
        QString name;
        ByteBuffer raw;
    };

    QList<NameEntry> namesTable;
    QList<ImportEntry> importsTable;
    QList<ExportEntry> exportsTable;
    int nameIdTexture2D = -1;
    int nameIdLightMapTexture2D = -1;
    int nameIdShadowMapTexture2D = -1;
    int nameIdTextureFlipBook = -1;
    int nameIdTextureMovie = -1;
    int nameIdTextureCube = -1;

    inline bool getCompressedFlag()
    {
        return (getFlags() & PackageFlags::compressed) != 0;
    }

    inline void setCompressedFlag(bool value)
    {
        if (value)
            setFlags(getFlags() | PackageFlags::compressed);
        else
            setFlags(getFlags() & ~PackageFlags::compressed);
    }


private:

    quint8 *packageHeader = nullptr;
    uint packageHeaderSize{};
    uint packageFileVersion{};
    uint numChunks{};
    uint someTag{};
    long dataOffset{};
    uint exportsEndOffset{};
    MemoryStream *packageData{};
    QList<Chunk> chunks;
    uint chunksTableOffset{};
    uint namesTableEnd{};
    bool namesTableModified = false;
    uint importsTableEnd{};
    bool importsTableModified = false;
    QList<int> dependsTable;
    QList<GuidEntry> guidsTable;
    QList<ExtraNameEntry> extraNamesTable;
    int currentChunk = -1;
    MemoryStream *chunkCache = nullptr;
    bool modified = false;

    inline uint getTag()
    {
        return *reinterpret_cast<uint *>(&packageHeader[packageHeaderTagOffset]);
    }

    inline ushort getVersion()
    {
        return *reinterpret_cast<ushort *>(&packageHeader[packageHeaderVersionOffset]);
    }

    inline uint getEndOfTablesOffset()
    {
        return *reinterpret_cast<uint *>(&packageHeader[packageHeaderFirstChunkSizeOffset]);
    }

    inline void setEndOfTablesOffset(uint value)
    {
        *reinterpret_cast<uint *>(&packageHeader[packageHeaderFirstChunkSizeOffset]) = value;
    }

    inline int getPackageHeaderFlagsOffset()
    {
        int len = *reinterpret_cast<uint *>(&packageHeader[packageHeaderNameSizeOffset]);
        if (len < 0)
            return (len * -2) + packageHeaderNameSizeOffset + sizeof(uint); // Unicode name
        return len + packageHeaderNameSizeOffset + sizeof(uint); // Ascii name
    }

    inline uint getFlags()
    {
        return *reinterpret_cast<uint *>(&packageHeader[getPackageHeaderFlagsOffset()]);
    }

    inline void setFlags(uint value)
    {
        *reinterpret_cast<uint *>(&packageHeader[getPackageHeaderFlagsOffset()]) = value;
    }

    inline int getTablesOffset()
    {
        if (packageFileVersion == packageFileVersion685)
            return getPackageHeaderFlagsOffset() + sizeof(uint) + sizeof(uint); // additional entry in header
        return getPackageHeaderFlagsOffset() + sizeof(uint);
    }

    inline uint getNamesCount()
    {
        return *reinterpret_cast<uint *>(&packageHeader[getTablesOffset() + packageHeaderNamesCountTableOffset]);
    }

    inline void setNamesCount(uint value)
    {
        *reinterpret_cast<uint *>(&packageHeader[getTablesOffset() + packageHeaderNamesCountTableOffset]) = value;
    }

    inline uint getNamesOffset()
    {
        return *reinterpret_cast<uint *>(&packageHeader[getTablesOffset() + packageHeaderNamesOffsetTabletsOffset]);
    }

    inline void setNamesOffset(uint value)
    {
        *reinterpret_cast<uint *>(&packageHeader[getTablesOffset() + packageHeaderNamesOffsetTabletsOffset]) = value;
    }

    inline uint getExportsCount()
    {
        return *reinterpret_cast<uint *>(&packageHeader[getTablesOffset() + packageHeaderExportsCountTableOffset]);
    }

    inline void setExportsCount(uint value)
    {
        *reinterpret_cast<uint *>(&packageHeader[getTablesOffset() + packageHeaderExportsCountTableOffset]) = value;
    }

    inline uint getExportsOffset()
    {
        return *reinterpret_cast<uint *>(&packageHeader[getTablesOffset() + packageHeaderExportsOffsetTableOffset]);
    }

    inline void setExportsOffset(uint value)
    {
        *reinterpret_cast<uint *>(&packageHeader[getTablesOffset() + packageHeaderExportsOffsetTableOffset]) = value;
    }

    inline uint getImportsCount()
    {
        return *reinterpret_cast<uint *>(&packageHeader[getTablesOffset() + packageHeaderImportsCountTableOffset]);
    }

    inline void setImportsCount(uint value)
    {
        *reinterpret_cast<uint *>(&packageHeader[getTablesOffset() + packageHeaderImportsCountTableOffset]) = value;
    }

    inline uint getImportsOffset()
    {
        return *reinterpret_cast<uint *>(&packageHeader[getTablesOffset() + packageHeaderImportsOffsetTableOffset]);
    }

    inline void setImportsOffset(uint value)
    {
        *reinterpret_cast<uint *>(&packageHeader[getTablesOffset() + packageHeaderImportsOffsetTableOffset]) = value;
    }

    inline uint getDependsOffset()
    {
        return *reinterpret_cast<uint *>(&packageHeader[getTablesOffset() + packageHeaderDependsOffsetTableOffset]);
    }

    inline void setDependsOffset(uint value)
    {
        *reinterpret_cast<uint *>(&packageHeader[getTablesOffset() + packageHeaderDependsOffsetTableOffset]) = value;
    }

    inline uint getGuidsOffset()
    {
        return *reinterpret_cast<uint *>(&packageHeader[getTablesOffset() + packageHeaderGuidsOffsetTableOffset]);
    }

    inline void setGuidsOffset(uint value)
    {
        *reinterpret_cast<uint *>(&packageHeader[getTablesOffset() + packageHeaderGuidsOffsetTableOffset]) = value;
    }

    inline uint getGuidsCount()
    {
        return *reinterpret_cast<uint *>(&packageHeader[getTablesOffset() + packageHeaderGuidsCountTableOffset]);
    }

    inline void setGuidsCount(uint value)
    {
        *reinterpret_cast<uint *>(&packageHeader[getTablesOffset() + packageHeaderGuidsCountTableOffset]) = value;
    }


public:

    CompressionType compressionType = CompressionType::None;
    Stream *packageStream = nullptr;
    FileStream *packageFile = nullptr;
    QString packagePath;

    Package() = default;
    ~Package();
    int Open(const QString &filename, bool headerOnly = false, bool fullLoad = false);
    bool isName(int id);
    QString getClassName(int id);
    int getClassNameId(int id);
    QString resolvePackagePath(int id);
    bool getData(uint offset, uint length, Stream *outputStream = nullptr, quint8 *outputBuffer = nullptr);
    ByteBuffer getExportData(int id);
    void setExportData(int id, const ByteBuffer &data);
    void MoveExportDataToEnd(int id);
    void SortExportsTableByDataOffset(const QList<ExportEntry> &list, QList<ExportEntry> &sortedExports);
    bool ReserveSpaceBeforeExportData(int space);
    static const QString StorageTypeToString(StorageTypes type);
    int getPropertiesOffset(int exportIndex);
    int getNameId(const QString &name);
    bool existsNameId(const QString &name);
    QString getName(int id);
    int addName(const QString &name);
    void loadNames(Stream &input);
    void saveNames(Stream &output);
    void loadExtraNames(Stream &input, bool rawMode = true);
    void saveExtraNames(Stream &output, bool rawMode = true);
    void loadImports(Stream &input);
    void loadImportsNames();
    void saveImports(Stream &output);
    void loadExports(Stream &input);
    void loadExportsNames();
    void saveExports(Stream &output);
    void loadDepends(Stream &input);
    void saveDepends(Stream &output);
    void loadGuids(Stream &input);
    void saveGuids(Stream &output);
    bool SaveToFile(bool forceCompressed = false, bool forceDecompressed = false, bool appendMarker = true);
    static const ByteBuffer compressData(const ByteBuffer &inputData, StorageTypes type,
                                         bool maxCompress = true);
    static const ByteBuffer decompressData(Stream &stream, StorageTypes type,
                                           int uncompressedSize, int compressedSize);
    void DisposeCache();
    void ReleaseChunks();
};

#endif
