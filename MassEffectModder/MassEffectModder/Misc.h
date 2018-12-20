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

#ifndef MISC_H
#define MISC_H

#include "Helpers/ByteBuffer.h"
#include "MemTypes.h"
#include "Texture.h"
#include "TreeScan.h"
#include "Resources.h"

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

    ~BinaryMod() { data.Free(); }
};

class Misc
{
public:

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
    static bool convertDataModtoMem(QString &inputDir, QString &memFilePath,
                                    MeType gameId, QList<FoundTexture> &textures, bool markToConvert,
                                    bool onlyIndividual, bool ipc);
    static QByteArray calculateMD5(const QString &filePath);
    static void detectMods(QStringList &mods);
    static void detectBrokenMod(QStringList &mods);
    static bool unpackSFARisNeeded();
    static bool checkGameFiles(MeType gameType, Resources &resources, QString &errors, QStringList &mods, bool ipc);
};

#endif
