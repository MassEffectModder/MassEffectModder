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

#ifndef MISC_H
#define MISC_H

#include "Helpers/ByteBuffer.h"
#include "MemTypes.h"
#include "Texture.h"
#include "TreeScan.h"
#include "Resources.h"
#include "Image.h"

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
    int binaryModType;
    QString textureName;
    uint textureCrc;
    bool markConvert;
    long offset;
    long size;
};

class Misc
{
public:

    static bool SetGameDataPath(MeType gameId, const QString &path);
    static bool SetGameUserPath(MeType gameId, const QString &path);
    static bool ConvertEndLines(const QString &path, bool unixMode);
    static bool ApplyLAAForME1Exe();
    static bool ChangeProductNameForME1Exe();
    static bool checkWriteAccessDir(const QString &path);
    static bool checkWriteAccessFile(QString &path);
    static bool isRunAsAdministrator();
    static bool CheckAndCorrectAccessToGame();
    static long getDiskFreeSpace(QString &path);
    static long getDirectorySize(QString &dir);
    static QString getBytesFormat(long size);
    static void startTimer();
    static long elapsedTime();
    static QString getTimerFormat(long time);
    static int ParseLegacyMe3xScriptMod(QList<FoundTexture> &textures, QString &script, QString &textureName);
    static void ParseME3xBinaryScriptMod(QString &script, QString &package, int &expId, QString &path);
    static PixelFormat changeTextureType(PixelFormat gamePixelFormat, PixelFormat texturePixelFormat,
                                         TexProperty::TextureTypes flags);
    static uint scanFilenameForCRC(const QString &inputFile);
    static FoundTexture FoundTextureInTheMap(QList<FoundTexture> &textures, uint crc);
    static bool convertDataModtoMem(QString &inputDir, QString &memFilePath,
                                    MeType gameId, QList<FoundTexture> &textures, bool markToConvert,
                                    bool onlyIndividual);
    static bool CorrectTexture(Image &image, FoundTexture &f, int numMips, bool markToConvert,
                              PixelFormat pixelFormat, PixelFormat newPixelFormat,
                              const QString &file);
    static QString CorrectTexture(Image *image, Texture &texture, PixelFormat pixelFormat,
                                  PixelFormat newPixelFormat, bool markConvert, const QString &textureName);
    static bool CorrectTexture(Image &image, Texture &texture, PixelFormat pixelFormat, const QString &textureName);
    static bool CheckMEMHeader(FileStream &fs, const QString &file);
    static bool CheckMEMGameVersion(FileStream &fs, const QString &file, int gameId);
    static int ReadModHeader(FileStream &fs);
    static void ReadModEntryHeader(FileStream &fs, QString &scriptLegacy, bool &binary, QString &textureName);
    static bool CheckImage(Image &image, FoundTexture &f, const QString &file, int index);
    static bool CheckImage(Image &image, Texture &texture, const QString &textureName);
    static bool ParseBinaryModFileName(const QString &file, QString pkgName, QString dlcName, int &exportId);
    static bool TpfGetCurrentFileInfo(void *handle, QString &fileName, quint64 &lenght);
    static bool DetectMarkToConvertFromFile(const QString &file);
    static int GetNumberOfMipsFromMap(FoundTexture &f);
    static QByteArray calculateMD5(const QString &filePath);
    static void detectMods(QStringList &mods);
    static void detectBrokenMod(QStringList &mods);
    static bool unpackSFARisNeeded();
    static bool checkGameFiles(MeType gameType, Resources &resources, QString &errors, QStringList &mods);
};

#endif
