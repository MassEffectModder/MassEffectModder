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

#ifndef MISC_H
#define MISC_H

#include <Helpers/ByteBuffer.h>
#include <Image/Image.h>
#include <Resources/Resources.h>
#include <Texture/Texture.h>
#include <Texture/TextureScan.h>
#include <Types/MemTypes.h>

class MipMaps;

struct MD5ModFileEntry
{
    const char *path;
    const quint8 md5[16];
    const char *modName;
};

struct BinaryMod
{
    QString packagePath;
    int exportId;
    ByteBuffer data;
    bool movieTexture;
    QString textureName;
    uint textureCrc;
    bool markConvert;
    bool forceHash;
    long offset;
    long size;
};

class Misc
{
public:

    enum ModsDataEnums
    {
        SizeOfChunkBlock = 8,
        SizeOfChunk = 8,
        MaxBlockSize = 0x40000, // 256KB
    };

    typedef void (*ProgressCallback)(void *handle, int progress, const QString &stage);

private:

    static bool checkGameFilesSub(FileStream *fs, QStringList &files, QList<MD5FileEntry> &entries,
                                  int &lastProgress, int &progress, int allFilesCount,
                                  QString &errors, QStringList &mods,
                                  ProgressCallback callback, void *callbackHandle);
public:

    static bool SetGameDataPath(MeType gameId, const QString &path);
    static bool SetGameUserPath(const QString &path);
    static bool ConvertEndLines(const QString &path, bool unixMode);
    static bool checkWriteAccessDir(const QString &path);
    static bool checkWriteAccessFile(QString &path);
    static bool isRunAsAdministrator();
    static bool CheckAndCorrectAccessToGame();
    static quint64 getDiskFreeSpace(QString &path);
    static quint64 getDirectorySize(QString &dir);
    static QString getBytesFormat(quint64 size);
    static void startTimer();
    static long elapsedTime();
    static void startStageTimer();
    static void restartStageTimer();
    static long elapsedStageTime();
    static QString getTimerFormat(long time);
    static bool CheckGamePath();
    static bool applyModTag(int MeuitmV, int AlotV);
    static PixelFormat changeTextureType(PixelFormat gamePixelFormat,
                                         PixelFormat texturePixelFormat,
                                         TextureType flags);
    static uint scanFilenameForCRC(const QString &inputFile);
    static uint GetCRCFromTextureMap(QList<TextureMapEntry> &textures, int exportId,
                                     const QString &path);
    static TextureMapEntry FoundTextureInTheMap(QList<TextureMapEntry> &textures, uint crc);
    static TextureMapEntry FoundTextureInTheInternalMap(MeType gameId, uint crc);
    static bool compareFileInfoPath(const QFileInfo &e1, const QFileInfo &e2);
    static bool convertDataModtoMem(QFileInfoList &files, QString &memFilePath,
                                    MeType gameId, QList<TextureMapEntry> &textures, bool markToConvert,
                                    ProgressCallback callback, void *callbackHandle);
    static bool InstallMods(MeType gameId, Resources &resources, QStringList &modFiles, bool guiInstallerMode, bool alotInstallerMode,
                           bool skipMarkers, bool verify, int cacheAmount,
                           ProgressCallback callback, void *callbackHandle);

    static bool extractMEM(MeType gameId, QFileInfoList &inputList, QString &outputDir,
                           ProgressCallback callback, void *callbackHandle);
    static bool CheckForMarkers(ProgressCallback callback, void *callbackHandle);
    static bool MarkersPresent(ProgressCallback callback, void *callbackHandle);
    static void AddMarkers(QStringList &pkgsToMarker,
                           ProgressCallback callback, void *callbackHandle);
    static bool ReportBadMods();
    static bool ReportMods();
    static bool applyMods(QStringList &files, QList<TextureMapEntry> &textures, QStringList &pkgsToMarker,
                          MipMaps &mipMaps, bool alotMode, bool verify, int cacheAmount,
                          ProgressCallback callback, void *callbackHandle);
    static QString CorrectTexture(Image *image, Texture &texture, PixelFormat newPixelFormat,
                                  const QString &textureName);
    static bool CorrectTexture(Image &image, TextureMapEntry &f, int numMips,
                              PixelFormat newPixelFormat, const QString &file);
    static bool CheckMEMHeader(FileStream &fs, const QString &file);
    static bool CheckMEMGameVersion(FileStream &fs, const QString &file, int gameId);
    static bool CheckImage(Image &image, TextureMapEntry &f, const QString &file, int index);
    static bool CheckImage(Image &image, Texture &texture, const QString &textureName);
    static bool DetectMarkToConvertFromFile(const QString &file);
    static bool DetectHashFromFile(const QString &file);
    static int GetNumberOfMipsFromMap(TextureMapEntry &f);
    static QByteArray calculateMD5(const QString &filePath);
    static void detectMods(QStringList &mods);
    static bool detectMod();
    static void detectBrokenMod(QStringList &mods);
    static bool CheckGameDataAndMods(MeType gameId, Resources &resources);
    static bool ApplyPostInstall(MeType gameId);
    static bool checkGameFiles(MeType gameType, Resources &resources, QString &errors,
                               QStringList &mods, ProgressCallback callback,
                               void *callbackHandle);
    static bool compressData(ByteBuffer inputData, Stream &ouputStream, CompressionDataType compType = CompressionDataType::LZMA);
    static ByteBuffer decompressData(Stream &stream, long compressedSize);
};

#endif
