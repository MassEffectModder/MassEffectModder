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

#include "Misc.h"
#include "GameData.h"
#include "MipMaps.h"
#include "Wrappers.h"
#include "MD5ModEntries.h"
#include "MD5BadEntries.h"
#include "Helpers/MiscHelpers.h"
#include "Helpers/Logs.h"
#include "Helpers/FileStream.h"

static bool generateModsMd5Entries = false;
static bool generateMd5Entries = false;

bool Misc::SetGameDataPath(MeType gameId, const QString &path)
{
    ConfigIni configIni;

    QString key = QString("ME%1").arg(static_cast<int>(gameId));
#if defined(_WIN32)
    configIni.Write(key, QString(path).replace(QChar('/'), QChar('\\'), Qt::CaseInsensitive), "GameDataPath");
#else
    configIni.Write(key, path, "GameDataPath");
#endif
    g_GameData->Init(gameId, configIni, true);
    if (!QFile(g_GameData->GameExePath()).exists())
    {
        PERROR("Error: Could not found the game!\n");
        return false;
    }

    PINFO("Game data found.\n");
    return true;
}

bool Misc::SetGameUserPath(MeType gameId, const QString &path)
{
    ConfigIni configIni;

    QString key = QString("ME%1").arg(static_cast<int>(gameId));
#if defined(_WIN32)
    configIni.Write(key, QString(path).replace(QChar('/'), QChar('\\'), Qt::CaseInsensitive), "GameUserPath");
#else
    configIni.Write(key, path, "GameUserPath");
#endif

    g_GameData->Init(gameId, configIni);
    QString newPath = g_GameData->GameUserPath();
    if (newPath.length() == 0 || !QDir(newPath).exists())
    {
        PERROR("Error: Could not found game user config path!\n");
        return false;
    }

    PINFO("Game user config path changed.\n");
    return true;
}

bool Misc::ConvertEndLines(const QString &path, bool unixMode)
{
    auto inputFile = new QFile(path);
    if (!inputFile->open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    auto text = inputFile->readAll();
    delete inputFile;
    QTextStream streamIn(text);

    if (QFile(path).open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        auto fs = FileStream(path, FileMode::Create);
        while (!streamIn.atEnd())
        {
            auto line = streamIn.readLine();
            fs.WriteStringASCII(line);
            if (unixMode)
                fs.WriteStringASCII("\n");
            else
                fs.WriteStringASCII("\r\n");
        }
        return true;
    }
    return false;
}

bool Misc::ApplyLAAForME1Exe()
{
    if (QFile(g_GameData->GameExePath()).exists())
    {
        FileStream fs = FileStream(g_GameData->GameExePath(), FileMode::Open, FileAccess::ReadWrite);
        {
            fs.JumpTo(0x3C); // jump to offset of COFF header
            uint offset = fs.ReadUInt32() + 4; // skip PE signature too
            fs.JumpTo(offset + 0x12); // jump to flags entry
            ushort flag = fs.ReadUInt16(); // read flags
            if ((flag & 0x20) != 0x20) // check for LAA flag
            {
                PINFO(QString("Patching ME1 for LAA: ") + g_GameData->GameExePath() + "\n");
                flag |= 0x20;
                fs.Skip(-2);
                fs.WriteUInt16(flag); // write LAA flag
            }
            else
            {
                PINFO(QString("File already has LAA flag enabled: ") + g_GameData->GameExePath() + "\n");
            }
        }
        return true;
    }

    PERROR(QString("File not found: ") + g_GameData->GameExePath() + "\n");
    return false;
}

bool Misc::ChangeProductNameForME1Exe()
{
    if (QFile(g_GameData->GameExePath()).exists())
    {
        // search for "ProductName Mass Effect"
        quint8 pattern[] = { 0x50, 0, 0x72, 0, 0x6F, 0, 0x64, 0, 0x75, 0, 0x63, 0, 0x74, 0, 0x4E, 0, 0x61, 0, 0x6D, 0, 0x65, 0, 0, 0, 0, 0,
                           0x4D, 0, 0x61, 0, 0x73, 0, 0x73, 0, 0x20, 0, 0x45, 0, 0x66, 0, 0x66, 0, 0x65, 0, 0x63, 0, 0x74, 0 };
        FileStream fs = FileStream(g_GameData->GameExePath(), FileMode::Open, FileAccess::ReadWrite);
        ByteBuffer buffer = fs.ReadAllToBuffer();
        quint8 *ptr = buffer.ptr();
        int pos = -1;
        for (int i = 0; i < buffer.size(); i++)
        {
            if (ptr[i] == pattern[0])
            {
                bool found = true;
                for (unsigned long l = 1; l < sizeof (pattern); l++)
                {
                    if (ptr[i + l] != pattern[l])
                    {
                        found = false;
                        break;
                    }
                }
                if (found)
                {
                    pos = i;
                    break;
                }
            }
        }
        if (pos != -1)
        {
            // replace to "Mass_Effect"
            fs.JumpTo(pos + 34);
            fs.WriteByte(0x5f);
            PINFO(QString("Patching ME1 for Product Name: ") + g_GameData->GameExePath() + "\n");
        }
        else
        {
            PINFO(QString("Specific Product Name not found or already changed: ") +
                         g_GameData->GameExePath() + "\n");
        }
        buffer.Free();
        return true;
    }

    PERROR(QString("File not found: ") + g_GameData->GameExePath() + "\n");
    return false;
}

bool Misc::checkWriteAccessDir(const QString &path)
{
    QFile file(path + "/test-mem-writefile");
    bool status = file.open(QIODevice::ReadWrite);
    if (status)
    {
        file.close();
        QFile(path + "/test-mem-writefile").remove();
    }
    return status;
}

bool Misc::checkWriteAccessFile(QString &path)
{
    QFile file(path);
    bool status = file.open(QIODevice::ReadWrite);
    return status;
}

bool Misc::isRunAsAdministrator()
{
    return DetectAdminRights();
}

bool Misc::CheckAndCorrectAccessToGame()
{
    if (!checkWriteAccessDir(g_GameData->MainData()))
    {
        PERROR(QString("MEM has not write access to game folders:\n") +
                     g_GameData->MainData() + "\n");
        return false;
    }

    return true;
}

long Misc::getDiskFreeSpace(QString &path)
{
    return QStorageInfo(path).bytesFree();
}

long Misc::getDirectorySize(QString &dir)
{
    return QFileInfo(dir).size();
}

QString getBytesFormat(long size)
{
    if (size / 1024 == 0)
        return QString::number(size, 'f', 2) + " Bytes";
    if (size / 1024 / 1024 == 0)
        return QString::number(size / 1024.0, 'f', 2) + " KB";
    if (size / 1024 / 1024 / 1024 == 0)
        return  QString::number(size / 1024.0 / 1024.0, 'f', 2) + " MB";
    return  QString::number(size / 1024.0 / 1024 / 1024.0, 'f', 2) + " GB";
}

static QElapsedTimer timer;
void Misc::startTimer()
{
    timer.start();
}

long Misc::elapsedTime()
{
    return timer.elapsed();
}

QString Misc::getTimerFormat(long time)
{
    if (time / 1000 == 0)
        return QString("%1 milliseconds").arg(time);
    if (time / 1000 / 60 == 0)
        return QString("%1 seconds").arg(time / 1000);
    if (time / 1000 / 60 / 60 == 0)
        return QString("%1 min - %2 sec").arg(time / 1000 / 60).arg(time / 1000 % 60);

    long hours = time / 1000 / 60 / 60;
    long minutes = (time - (hours * 1000 * 60 * 60)) / 1000 / 60;
    long seconds = (time - (hours * 1000 * 60 * 60) - (minutes * 1000 * 60)) / 1000 / 60;
    return QString("%1 hours - %2 min - %3 sec").arg(hours).arg(minutes).arg(seconds);
}

int Misc::ParseLegacyMe3xScriptMod(QList<FoundTexture> &textures, QString &script, QString &textureName)
{
    QRegularExpression regex("pccs.Add[(]\"[A-z,0-9/,..]*\"");
    auto match = regex.match(script);
    if (match.hasMatch())
    {
        QString packageName = match.captured().replace(QChar('/'), QChar('\\')).split(QChar('\"'))[1].split(QChar('\\')).last().split(QChar('.')).first().toLower();

        regex = QRegularExpression("IDs.Add[(][0-9]*[)];");
        match = regex.match(script);
        if (match.hasMatch())
        {
            int exportId = match.captured().split(QChar('('))[1].split(QChar(')')).first().toInt();
            if (exportId != 0)
            {
                textureName = textureName.toLower();
                for (int i = 0; i < textures.count(); i++)
                {
                    if (textures[i].name.toLower() == textureName)
                    {
                        for (int l = 0; l < textures[i].list.count(); l++)
                        {
                            if (textures[i].list[l].path.length() == 0)
                                continue;
                            if (textures[i].list[l].exportID == exportId)
                            {
                                QString pkg = textures[i].list[l].path.split(QChar('/')).last().split(QChar('.')).first().toLower();
                                if (pkg == packageName)
                                {
                                    return i;
                                }
                            }
                        }
                    }
                }
                // search again but without name match
                for (int i = 0; i < textures.count(); i++)
                {
                    for (int l = 0; l < textures[i].list.count(); l++)
                    {
                        if (textures[i].list[l].path.length() == 0)
                            continue;
                        if (textures[i].list[l].exportID == exportId)
                        {
                            QString pkg = textures[i].list[l].path.split(QChar('/')).last().split(QChar('.')).first().toLower();
                            if (pkg == packageName)
                            {
                                return i;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            textureName = textureName.toLower();
            for (int i = 0; i < textures.count(); i++)
            {
                if (textures[i].name.toLower() == textureName)
                {
                    for (int l = 0; l < textures[i].list.count(); l++)
                    {
                        if (textures[i].list[l].path.length() == 0)
                            continue;
                        QString pkg = textures[i].list[l].path.split(QChar('/')).last().split(QChar('.')).first().toLower();
                        if (pkg == packageName)
                        {
                            return i;
                        }
                    }
                }
            }
        }
    }

    return -1;
}

void Misc::ParseME3xBinaryScriptMod(QString &script, QString &package, int &expId, QString &path)
{
    QRegularExpression regex("int objidx = [0-9]*");
    auto match = regex.match(script);
    if (match.hasMatch())
    {
        expId = match.captured().split(QChar(' ')).last().toInt();

        regex = QRegularExpression("string filename = \"[A-z,0-9,.]*\";");
        match = regex.match(script);
        if (match.hasMatch())
        {
            package = match.captured().split(QChar('\"'))[1].replace("\\\\", "\\").replace('\\', '/');

            regex = QRegularExpression("string pathtarget = ME3Directory.cookedPath;");
            match = regex.match(script);
            if (match.hasMatch())
            {
                path = "/BIOGame/CookedPCConsole";
                return;
            }

            regex = QRegularExpression("string pathtarget = Path.GetDirectoryName[(]ME3Directory[.]cookedPath[)];");
            match = regex.match(script);
            if (match.hasMatch())
            {
                path = "/BIOGame";
                return;
            }
            regex = QRegularExpression("string pathtarget = new DirectoryInfo[(]ME3Directory[.]cookedPath[)][.]Parent.FullName [+] \"[A-z,0-9,_,.]*\";");
            match = regex.match(script);
            if (match.hasMatch())
            {
                path = QDir::cleanPath(DirName("/BIOGame/" + match.captured().split(QChar('\"'))[1].replace('\\', '/')));
            }
        }
    }
}

PixelFormat Misc::changeTextureType(PixelFormat gamePixelFormat, PixelFormat texturePixelFormat,
                                    TexProperty::TextureTypes flags)
{
    if ((gamePixelFormat == PixelFormat::DXT5 || gamePixelFormat == PixelFormat::DXT1 || gamePixelFormat == PixelFormat::ATI2) &&
        (texturePixelFormat == PixelFormat::RGB || texturePixelFormat == PixelFormat::ARGB ||
         texturePixelFormat == PixelFormat::ATI2 || texturePixelFormat == PixelFormat::V8U8))
    {
        if (texturePixelFormat == PixelFormat::ARGB && flags == TexProperty::TextureTypes::OneBitAlpha)
        {
            gamePixelFormat = PixelFormat::ARGB;
        }
        else if (texturePixelFormat == PixelFormat::ATI2 &&
            gamePixelFormat == PixelFormat::DXT1 &&
            flags == TexProperty::TextureTypes::Normalmap)
        {
            gamePixelFormat = PixelFormat::ATI2;
        }
        else if (GameData::gameType != MeType::ME3_TYPE && texturePixelFormat == PixelFormat::ARGB &&
            flags == TexProperty::TextureTypes::Normalmap)
        {
            gamePixelFormat = PixelFormat::ARGB;
        }
        else if ((gamePixelFormat == PixelFormat::DXT5 || gamePixelFormat == PixelFormat::DXT1) &&
            (texturePixelFormat == PixelFormat::ARGB || texturePixelFormat == PixelFormat::RGB) &&
            flags == TexProperty::TextureTypes::Normal)
        {
            gamePixelFormat = PixelFormat::ARGB;
        }
        else if ((gamePixelFormat == PixelFormat::DXT5 || gamePixelFormat == PixelFormat::DXT1) &&
            texturePixelFormat == PixelFormat::V8U8 && GameData::gameType == MeType::ME3_TYPE &&
            flags == TexProperty::TextureTypes::Normal)
        {
            gamePixelFormat = PixelFormat::V8U8;
        }
    }

    return gamePixelFormat;
}

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

FoundTexture Misc::FoundTextureInTheMap(QList<FoundTexture> &textures, uint crc)
{
    FoundTexture f{};
    for (int s = 0; s < textures.count(); s++)
    {
        if (textures[s].crc == crc)
        {
            f = textures[s];
            break;
        }
    }
    return f;
}

bool Misc::CorrectTexture(Image &image, FoundTexture &f, int numMips, bool markToConvert,
                          PixelFormat pixelFormat, PixelFormat newPixelFormat,
                          const QString &file)
{
    if (!image.checkDDSHaveAllMipmaps() ||
       (numMips > 1 && image.getMipMaps().count() <= 1) ||
       (markToConvert && image.getPixelFormat() != newPixelFormat) ||
       (!markToConvert && image.getPixelFormat() != pixelFormat))
    {
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]PROCESSING_FILE Converting ") + BaseName(file));
            ConsoleSync();
        }
        else
        {
            PINFO(QString("Converting/correcting texture: ") + BaseName(file) + "\n");
        }
        bool dxt1HasAlpha = false;
        quint8 dxt1Threshold = 128;
        if (f.flags == TexProperty::TextureTypes::OneBitAlpha)
        {
            dxt1HasAlpha = true;
            if (image.getPixelFormat() == PixelFormat::ARGB ||
                image.getPixelFormat() == PixelFormat::DXT3 ||
                image.getPixelFormat() == PixelFormat::DXT5)
            {
                PINFO(QString("Warning for texture: " ) + f.name +
                             ". This texture converted from full alpha to binary alpha.\n");
            }
        }
        image.correctMips(newPixelFormat, dxt1HasAlpha, dxt1Threshold);
        return true;
    }
    return false;
}

QString Misc::CorrectTexture(Image *image, Texture &texture, PixelFormat pixelFormat,
                             PixelFormat newPixelFormat, bool markConvert, const QString &textureName)
{
    QString errors;
    if (!image->checkDDSHaveAllMipmaps() ||
        (texture.mipMapsList.count() > 1 && image->getMipMaps().count() <= 1) ||
        (markConvert && image->getPixelFormat() != newPixelFormat) ||
        (!markConvert && image->getPixelFormat() != pixelFormat))
    {
        bool dxt1HasAlpha = false;
        quint8 dxt1Threshold = 128;
        if (pixelFormat == PixelFormat::DXT1 && texture.getProperties().exists("CompressionSettings"))
        {
            if (texture.getProperties().exists("CompressionSettings") &&
                texture.getProperties().getProperty("CompressionSettings").valueName == "TC_OneBitAlpha")
            {
                dxt1HasAlpha = true;
                if (image->getPixelFormat() == PixelFormat::ARGB ||
                    image->getPixelFormat() == PixelFormat::DXT3 ||
                    image->getPixelFormat() == PixelFormat::DXT5)
                {
                    errors += "Warning for texture: " + textureName +
                              ". This texture converted from full alpha to binary alpha.\n";
                }
            }
        }
        image->correctMips(pixelFormat, dxt1HasAlpha, dxt1Threshold);
    }
    return errors;
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

int Misc::ReadModHeader(FileStream &fs)
{
    QString version;
    int len = fs.ReadInt32();
    fs.ReadStringASCIINull(version);
    if (version.size() < 5) // legacy .mod
        fs.SeekBegin();
    else
    {
        fs.SeekBegin();
        len = fs.ReadInt32();
        fs.ReadStringASCII(version, len); // version
    }
    return fs.ReadUInt32();
}

void Misc::ReadModEntryHeader(FileStream &fs, QString &scriptLegacy,
                              bool &binary, QString &textureName)
{
    int len = fs.ReadInt32();
    QString desc;
    fs.ReadStringASCII(desc, len); // description
    len = fs.ReadInt32();
    fs.ReadStringASCII(scriptLegacy, len);
    binary = desc.contains("Binary Replacement", Qt::CaseInsensitive);
    if (!binary)
        textureName = desc.split(QChar(' ')).last();
}

bool Misc::CheckImage(Image &image, FoundTexture &f, const QString &file, int index)
{
    if (image.getMipMaps().count() == 0)
    {
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]ERROR_FILE_NOT_COMPATIBLE ") + BaseName(file));
            ConsoleSync();
        }
        else
        {
            if (index == -1)
            {
                PINFO(QString("Skipping texture: ") + f.name + QString().sprintf("_0x%08X", f.crc) + "\n");
            }
            else
            {
                PERROR(QString("Skipping not compatible content, entry: ") +
                             QString::number(index + 1) + " - mod: " + BaseName(file) + "\n");
            }
        }
        return false;
    }

    if (image.getMipMaps().first()->getOrigWidth() / image.getMipMaps().first()->getOrigHeight() !=
        f.width / f.height)
    {
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]ERROR_FILE_NOT_COMPATIBLE ") + BaseName(file));
            ConsoleSync();
        }
        else
        {
            if (index == -1)
            {
                PINFO(QString("Skipping texture: ") + f.name + QString().sprintf("_0x%08X", f.crc) + "\n");
            }
            else
            {
                PERROR(QString("Error in texture: ") + f.name + QString().sprintf("_0x%08X", f.crc) +
                    " This texture has wrong aspect ratio, skipping texture, entry: " + (index + 1) +
                    " - mod: " + BaseName(file) + "\n");
            }
        }
        return false;
    }

    return true;
}

bool Misc::CheckImage(Image &image, Texture &texture, const QString &textureName)
{
    if (image.getMipMaps().count() == 0)
    {
        PERROR(QString("Error in texture: ") + textureName + "\n");
        return false;
    }

    if (image.getMipMaps().first()->getOrigWidth() / image.getMipMaps().first()->getHeight() !=
        texture.mipMapsList.first().width / texture.mipMapsList.first().height)
    {
        PERROR(QString("Error in texture: ") + textureName +
                     " This texture has wrong aspect ratio, skipping texture...\n");
        return false;
    }

    return true;
}

int Misc::GetNumberOfMipsFromMap(FoundTexture &f)
{
    for (int s = 0; s < f.list.count(); s++)
    {
        if (f.list[s].path.length() != 0)
        {
            return f.list[s].numMips;
        }
    }
    return 0;
}

bool Misc::ParseBinaryModFileName(const QString &file, QString pkgName, QString dlcName, int &exportId)
{
    QString filename = BaseNameWithoutExt(file);
    int posStr = 0;
    if (filename.toUpper()[0] == 'D')
    {
        QString tmpDLC = filename.split(QChar('-'))[0];
        int lenDLC = tmpDLC.midRef(1).toInt();
        dlcName = filename.mid(tmpDLC.size() + 1, lenDLC);
        posStr += tmpDLC.size() + lenDLC + 1;
        if (filename[posStr++] != '-')
            return false;
    }
    else if (filename.toUpper()[0] == 'B')
    {
        posStr += 1;
    }
    else
        return false;
    QString tmpPkg = filename.mid(posStr).split('-')[0];
    posStr += tmpPkg.size() + 1;
    int lenPkg = tmpPkg.midRef(0).toInt();
    pkgName = filename.mid(posStr, lenPkg);
    posStr += lenPkg;
    if (filename[posStr++] != '-')
        return false;
    if (filename.toUpper()[posStr++] != 'E')
        return false;
    QString tmpExp = filename.mid(posStr);
    exportId = tmpExp.midRef(0).toInt();

    return true;
}

uint GetCrcFromTpfList(QStringList &ddsList, const QString &filename)
{
    uint crc = 0;
    foreach (QString dds, ddsList)
    {
        if (dds.length() == 0)
            continue;
        QString ddsFile = dds.split('|')[1];
        if (ddsFile != "" && ddsList.count() != 1 &&
            ddsFile.toLower() != filename.toLower())
        {
            continue;
        }
        bool ok;
        crc = dds.split('|').first().midRef(2).toUInt(&ok, 16);
        break;
    }
    return crc;
}

bool Misc::TpfGetCurrentFileInfo(void *handle, QString &fileName, quint64 &lenght)
{
    char *filetmp;
    int filetmplen = 0;
    int result = ZipGetCurrentFileInfo(handle, &filetmp, &filetmplen, &lenght);
    if (result != 0)
        return false;
    fileName = QString(filetmp);
    delete[] filetmp;
    return true;
}

bool Misc::DetectMarkToConvertFromFile(const QString &file)
{
    int idx = file.indexOf("-memconvert", Qt::CaseInsensitive);
    return idx > 0;
}

static bool compareFileInfoPath(const QFileInfo &e1, const QFileInfo &e2)
{
    return e1.absoluteFilePath().compare(e2.absoluteFilePath(), Qt::CaseInsensitive) < 0;
}

bool Misc::convertDataModtoMem(QString &inputDir, QString &memFilePath,
    MeType gameId, QList<FoundTexture> &textures, bool markToConvert, bool onlyIndividual)
{
    PINFO("Mods conversion started...\n");

    QFileInfoList list;
    QFileInfoList list2;
    if (!onlyIndividual)
    {
        list = QDir(inputDir, "*.mem", QDir::SortFlag::IgnoreCase | QDir::SortFlag::Name, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
        list2 = QDir(inputDir, "*.tpf", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
        list2 += QDir(inputDir, "*.mod", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    }
    list2 += QDir(inputDir, "*.bin", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    list2 += QDir(inputDir, "*.xdelta", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    list2 += QDir(inputDir, "*.dds", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    list2 += QDir(inputDir, "*.png", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    list2 += QDir(inputDir, "*.bmp", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    list2 += QDir(inputDir, "*.tga", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    std::sort(list2.begin(), list2.end(), compareFileInfoPath);
    list.append(list2);

    int result;
    QString fileName;
    quint64 dstLen = 0;
    QStringList ddsList;
    int numEntries = 0;

    QList<BinaryMod> mods = QList<BinaryMod>();
    QList<FileMod> modFiles = QList<FileMod>();

    QString dir = DirName(memFilePath);
    if (dir.length() != 0)
        QDir().mkpath(DirName(dir));

    if (QFile(memFilePath).exists())
        QFile(memFilePath).remove();

    Misc::startTimer();

    FileStream outFs = FileStream(memFilePath, FileMode::Create, FileAccess::WriteOnly);
    outFs.WriteUInt32(TextureModTag);
    outFs.WriteUInt32(TextureModVersion);
    outFs.WriteInt64(0); // filled later

    int lastProgress = -1;
    for (int n = 0; n < list.count(); n++)
    {
        foreach (BinaryMod mod, mods)
        {
            mod.data.Free();
        }
        mods.clear();

        QString file = list[n].absoluteFilePath();
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + BaseName(file));
            int newProgress = (n * 100) / list.count();
            if (lastProgress != newProgress)
            {
                ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
                lastProgress = newProgress;
            }
            ConsoleSync();
        }
        else
        {
            PINFO(QString("File: ") + BaseName(file) + "\n");
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
                FileMod fileMod{};
                fileMod.tag = fs.ReadUInt32();
                fs.ReadStringASCIINull(fileMod.name);
                fileMod.offset = fs.ReadInt64();
                fileMod.size = fs.ReadInt64();
                qint64 prevPos = fs.Position();
                fs.JumpTo(fileMod.offset);
                fileMod.offset = outFs.Position();
                if (fileMod.tag == FileTextureTag || fileMod.tag == FileTextureTag2)
                {
                    QString str;
                    fs.ReadStringASCIINull(str);
                    outFs.WriteStringASCIINull(str);
                    outFs.WriteUInt32(fs.ReadUInt32());
                }
                else if (fileMod.tag == FileBinaryTag || fileMod.tag == FileXdeltaTag)
                {
                    outFs.WriteInt32(fs.ReadInt32());
                    QString str;
                    fs.ReadStringASCIINull(str);
                    outFs.WriteStringASCIINull(str);
                }
                else
                    CRASH();

                outFs.CopyFrom(fs, fileMod.size);
                fs.JumpTo(prevPos);
                modFiles.push_back(fileMod);
            }
        }
        else if (file.endsWith(".mod", Qt::CaseInsensitive))
        {
            FileStream fs = FileStream(file, FileMode::Open, FileAccess::ReadOnly);
            QString package;
            int numEntries = ReadModHeader(fs);
            for (int i = 0; i < numEntries; i++)
            {
                BinaryMod mod{};

                QString scriptLegacy;
                bool binary;
                QString textureName;
                ReadModEntryHeader(fs, scriptLegacy, binary, textureName);

                if (binary)
                {
                    QString path;
                    ParseME3xBinaryScriptMod(scriptLegacy, package, mod.exportId, path);
                    if (mod.exportId == -1 || package.length() == 0 || path.length() == 0)
                    {
                        fs.Skip(fs.ReadInt32());
                        if (g_ipc)
                        {
                            ConsoleWrite(QString("[IPC]ERROR_FILE_NOT_COMPATIBLE ") + BaseName(file));
                            ConsoleSync();
                        }
                        else
                        {
                            PINFO(QString("Skipping not compatible content, entry: ") +
                                         QString::number(i + 1) + " - mod: " + BaseName(file) + "\n");
                        }
                        continue;
                    }
                    mod.packagePath = path + "/" + package;
                    mod.binaryModType = 1;
                    mod.data = fs.ReadToBuffer(fs.ReadInt32());
                }
                else
                {
                    int index = ParseLegacyMe3xScriptMod(textures, scriptLegacy, textureName);
                    if (index == -1)
                    {
                        fs.Skip(fs.ReadInt32());
                        if (g_ipc)
                        {
                            ConsoleWrite(QString("[IPC]ERROR_FILE_NOT_COMPATIBLE ") + BaseName(file));
                            ConsoleSync();
                        }
                        else
                        {
                            PINFO(QString("Skipping not compatible content, entry: ") +
                                         QString::number(i + 1) + " - mod: " + BaseName(file) + "\n");
                        }
                        continue;
                    }
                    FoundTexture f = textures[index];
                    mod.textureCrc = f.crc;
                    mod.textureName = f.name;
                    mod.binaryModType = 0;
                    mod.data = fs.ReadToBuffer(fs.ReadInt32());

                    Image image = Image(mod.data, ImageFormat::DDS);
                    if (!CheckImage(image, f, file, i))
                        continue;

                    PixelFormat newPixelFormat = f.pixfmt;
                    if (markToConvert)
                        newPixelFormat = changeTextureType(f.pixfmt, image.getPixelFormat(), f.flags);

                    int numMips = GetNumberOfMipsFromMap(f);
                    if (CorrectTexture(image, f, numMips, markToConvert,
                                       f.pixfmt, newPixelFormat, file))
                    {
                        mod.data.Free();
                        mod.data = image.StoreImageToDDS();
                    }
                }
                mod.markConvert = markToConvert;
                mods.push_back(mod);
            }
        }
        else if (file.endsWith(".bin", Qt::CaseInsensitive) ||
            file.endsWith(".xdelta", Qt::CaseInsensitive))
        {
            BinaryMod mod{};
            QString dlcName;
            QString pkgName;
            if (!ParseBinaryModFileName(file, pkgName, dlcName, mod.exportId))
            {
                if (g_ipc)
                {
                    ConsoleWrite(QString("[IPC]ERROR_FILE_NOT_COMPATIBLE ") + BaseName(file));
                    ConsoleSync();
                }
                else
                {
                    PINFO(QString("Skipping not compatible mod: ") + BaseName(file) + "\n");
                }
                continue;
            }
            if (dlcName.length() != 0)
            {
                if (gameId == MeType::ME1_TYPE)
                    mod.packagePath = QString("/DLC/") + dlcName + "/CookedPC/" + pkgName;
                else if (gameId == MeType::ME2_TYPE)
                    mod.packagePath = QString("/BioGame/DLC/") + dlcName + "/CookedPC/" + pkgName;
                else
                    mod.packagePath = QString("/BioGame/DLC/") + dlcName + "/CookedPCConsole/" + pkgName;
            }
            else
            {
                if (gameId == MeType::ME1_TYPE || gameId == MeType::ME2_TYPE)
                    mod.packagePath = QString("/BioGame/CookedPC/") + pkgName;
                else
                    mod.packagePath = QString("/BIOGame/CookedPCConsole/") + pkgName;
            }
            if (file.endsWith(".bin", Qt::CaseInsensitive))
                mod.binaryModType = 1;
            else if (file.endsWith(".xdelta", Qt::CaseInsensitive))
                mod.binaryModType = 2;
            mod.data = FileStream(file, FileMode::Open).ReadAllToBuffer();
            mods.push_back(mod);
        }
        else if (file.endsWith(".tpf", Qt::CaseInsensitive))
        {
            int indexTpf = -1;
            char *listText;
#if defined(_WIN32)
            auto str = file.toStdWString();
#else
            auto str = file.toStdString();
#endif
            auto name = str.c_str();
            void *handle = ZipOpenFromFile(name, &numEntries, 1);
            if (handle == nullptr)
                goto failed;

            for (int i = 0; i < numEntries; i++)
            {
                if (!TpfGetCurrentFileInfo(handle, fileName, dstLen))
                    goto failed;
                if (fileName.endsWith(".def", Qt::CaseInsensitive) ||
                    fileName.endsWith(".log", Qt::CaseInsensitive))
                {
                    indexTpf = (int)i;
                    break;
                }
                result = ZipGoToNextFile(handle);
                if (result != 0)
                    goto failed;
            }

            listText = new char[dstLen];
            result = ZipReadCurrentFile(handle, reinterpret_cast<quint8 *>(listText), dstLen, nullptr);
            if (result != 0)
            {
                delete[] listText;
                goto failed;
            }
            ddsList = QString(listText).remove(QChar('\r')).split(QChar('\n'));
            delete[] listText;

            result = ZipGoToFirstFile(handle);
            if (result != 0)
                goto failed;

            for (int i = 0; i < numEntries; i++)
            {
                BinaryMod mod{};

                if (i == indexTpf)
                {
                    result = ZipGoToNextFile(handle);
                    continue;
                }

                if (!TpfGetCurrentFileInfo(handle, fileName, dstLen))
                    goto failed;

                uint crc = GetCrcFromTpfList(ddsList, fileName);
                if (crc == 0)
                {
                    if (g_ipc)
                    {
                        ConsoleWrite(QString("[IPC]ERROR_FILE_NOT_COMPATIBLE ") + BaseName(file));
                        ConsoleSync();
                    }
                    else
                    {
                        PERROR(QString("Skipping file: ") + fileName + " not found in definition file, entry: " +
                            (i + 1) + " - mod: " + BaseName(file) + "\n");
                    }
                    ZipGoToNextFile(handle);
                    continue;
                }

                FoundTexture f = FoundTextureInTheMap(textures, crc);
                if (f.crc == 0)
                {
                    PINFO(QString("Texture skipped. File ") + fileName + QString().sprintf("_0x%08X", crc) +
                        " is not present in your game setup - mod: " + BaseName(file));
                    ZipGoToNextFile(handle);
                    continue;
                }

                QString textureName = f.name;
                mod.textureName = textureName;
                mod.binaryModType = 0;
                mod.textureCrc = crc;
                mod.data = ByteBuffer(dstLen);
                result = ZipReadCurrentFile(handle, mod.data.ptr(), dstLen, nullptr);
                if (result != 0)
                {
                    ZipGoToNextFile(handle);
                    if (g_ipc)
                    {
                        ConsoleWrite(QString("[IPC]ERROR_FILE_NOT_COMPATIBLE ") + BaseName(file));
                        ConsoleSync();
                    }
                    else
                    {
                        PERROR(QString("Error in texture: ") + textureName + QString().sprintf("_0x%08X", crc) +
                            ", skipping texture, entry: " + (i + 1) + " - mod: " + BaseName(file) + "\n");
                    }
                    mod.data.Free();
                    continue;
                }

                Image image = Image(mod.data, GetFileExtension(fileName));
                if (!CheckImage(image, f, file, i))
                {
                    mod.data.Free();
                    continue;
                }

                PixelFormat newPixelFormat = f.pixfmt;
                if (markToConvert)
                    newPixelFormat = changeTextureType(f.pixfmt, image.getPixelFormat(), f.flags);

                int numMips = GetNumberOfMipsFromMap(f);
                if (CorrectTexture(image, f, numMips, markToConvert,
                                   f.pixfmt, newPixelFormat, file))
                {
                    mod.data.Free();
                    mod.data = image.StoreImageToDDS();
                }
                mod.markConvert = markToConvert;
                mods.push_back(mod);
                ZipGoToNextFile(handle);
            }
            goto end;
failed:
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR_FILE_NOT_COMPATIBLE ") + BaseName(file));
                ConsoleSync();
            }
            else
            {
                PERROR(QString("Mod is not compatible: ") + BaseName(file) + "\n");
            }
end:
            if (handle != nullptr)
                ZipClose(handle);
            handle = nullptr;
        }
        else if (file.endsWith(".dds", Qt::CaseInsensitive) ||
                 file.endsWith(".png", Qt::CaseInsensitive) ||
                 file.endsWith(".bmp", Qt::CaseInsensitive) ||
                 file.endsWith(".tga", Qt::CaseInsensitive))
        {
            BinaryMod mod{};
            uint crc = scanFilenameForCRC(file);
            if (crc == 0)
                continue;

            FoundTexture f = FoundTextureInTheMap(textures, crc);
            if (f.crc == 0)
            {
                PINFO(QString("Texture skipped. Texture ") + BaseName(file) +
                             " is not present in your game setup.\n");
                continue;
            }

            markToConvert = DetectMarkToConvertFromFile(file);

            Image image(file, ImageFormat::UnknownImageFormat);
            if (!Misc::CheckImage(image, f, file, -1))
                continue;

            PixelFormat newPixelFormat = f.pixfmt;
            if (markToConvert)
                newPixelFormat = changeTextureType(f.pixfmt, image.getPixelFormat(), f.flags);

            int numMips = Misc::GetNumberOfMipsFromMap(f);
            CorrectTexture(image, f, numMips, markToConvert,
                           f.pixfmt, newPixelFormat, file);

            mod.data = image.StoreImageToDDS();
            mod.textureName = f.name;
            mod.binaryModType = 0;
            mod.textureCrc = crc;
            mod.markConvert = markToConvert;
            mods.push_back(mod);
        }


        for (int l = 0; l < mods.count(); l++)
        {
            FileMod fileMod{};
            std::unique_ptr<Stream> dst (new MemoryStream());
            if (mods[l].data.size() != 0)
            {
                MipMaps::compressData(mods[l].data, *dst);
                dst->SeekBegin();
                fileMod.offset = outFs.Position();
                fileMod.size = dst->Length();
            }
            else
            {
                fileMod.offset = mods[l].offset;
                fileMod.size = mods[l].size;
            }

            if (mods[l].binaryModType == 1)
            {
                fileMod.tag = FileBinaryTag;
                if (mods[l].packagePath.contains("/DLC/", Qt::CaseInsensitive))
                {
                    QString dlcName = mods[l].packagePath.split(QChar('/'))[3];
                    fileMod.name = "D" + QString::number(dlcName.size()) + "-" + dlcName + "-";
                }
                else
                {
                    fileMod.name = "B";
                }
                fileMod.name += QString::number(BaseName(mods[l].packagePath).size()) +
                        "-" + BaseName(mods[l].packagePath) +
                        "-E" + QString::number(mods[l].exportId) + ".bin";

                outFs.WriteInt32(mods[l].exportId);
                outFs.WriteStringASCIINull(mods[l].packagePath.replace('/', '\\'));
            }
            else if (mods[l].binaryModType == 2)
            {
                fileMod.tag = FileXdeltaTag;
                if (mods[l].packagePath.contains("/DLC/", Qt::CaseInsensitive))
                {
                    QString dlcName = mods[l].packagePath.split(QChar('/'))[3];
                    fileMod.name = "D" + QString::number(dlcName.size()) + "-" + dlcName + "-";
                }
                else
                {
                    fileMod.name = "B";
                }
                fileMod.name += QString::number(BaseName(mods[l].packagePath).size()) +
                        "-" + BaseName(mods[l].packagePath) +
                        "-E" + QString::number(mods[l].exportId) + ".xdelta";

                outFs.WriteInt32(mods[l].exportId);
                outFs.WriteStringASCIINull(mods[l].packagePath.replace('/', '\\'));
            }
            else
            {
                if (mods[l].markConvert)
                    fileMod.tag = FileTextureTag2;
                else
                    fileMod.tag = FileTextureTag;
                fileMod.name = mods[l].textureName + QString().sprintf("_0x%08X", mods[l].textureCrc) + ".dds";
                outFs.WriteStringASCIINull(mods[l].textureName);
                outFs.WriteUInt32(mods[l].textureCrc);
            }
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
        }
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
    }

    long elapsed = Misc::elapsedTime();
    PINFO(Misc::getTimerFormat(elapsed) + "\n");

    return true;
}

QByteArray Misc::calculateMD5(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly))
        return QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5);
    return QByteArray(16, 0);
}

void Misc::detectMods(QStringList &mods)
{
    for (int l = 0; l < modsEntriesSize; l++)
    {
        QString path = g_GameData->GamePath() + modsEntries[l].path;
        if (!QFile(path).exists())
            continue;
        QByteArray md5 = calculateMD5(path);
        if (memcmp(md5.data(), modsEntries[l].md5, 16) == 0)
        {
            bool found = false;
            for (int s = 0; s < mods.count(); s++)
            {
                if (mods[s].compare(modsEntries[l].modName) == 0)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                mods.push_back(modsEntries[l].modName);
        }
    }
}

void Misc::detectBrokenMod(QStringList &mods)
{
    for (int l = 0; l < badMODSize; l++)
    {
        QString path = g_GameData->GamePath() + badMOD[l].path;
        if (!QFile(path).exists())
            continue;
        QByteArray md5 = calculateMD5(path);
        if (memcmp(md5.data(), badMOD[l].md5, 16) == 0)
        {
            bool found = false;
            for (int s = 0; s < mods.count(); s++)
            {
                if (mods[s].compare(badMOD[l].modName) == 0)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                mods.push_back(badMOD[l].modName);
        }
    }
}

bool Misc::unpackSFARisNeeded()
{
    if (QDir(g_GameData->DLCData()).exists())
    {
        QStringList DLCs = QDir(g_GameData->DLCData(), "DLC_*", QDir::NoSort, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryList();
        foreach (QString DLCDir, DLCs)
        {
            QStringList packages;
            QDirIterator iterator(g_GameData->DLCData() + "/" + DLCDir + g_GameData->DLCDataSuffix(), QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
            bool isValid = false;
            bool unpacked = false;
            while (iterator.hasNext())
            {
                iterator.next();
                if (iterator.filePath().endsWith("Default.sfar", Qt::CaseInsensitive))
                    isValid = true;
                if (iterator.filePath().endsWith("Mount.dlc", Qt::CaseInsensitive))
                    unpacked = true;
            }
            if (isValid && !unpacked)
                return true;
        }
    }

    return false;
}

bool Misc::checkGameFiles(MeType gameType, Resources &resources, QString &errors, QStringList &mods)
{
    bool vanilla = true;
    QList<MD5FileEntry> entries;

    if (gameType == MeType::ME1_TYPE)
    {
        entries += resources.entriesME1;
    }
    else if (gameType == MeType::ME2_TYPE)
    {
        entries += resources.entriesME2;
    }
    else if (gameType == MeType::ME3_TYPE)
    {
        entries += resources.entriesME3;
    }

    int progress = 0;
    int allFilesCount = g_GameData->packageFiles.count();
    allFilesCount += g_GameData->sfarFiles.count();
    allFilesCount += g_GameData->tfcFiles.count();

    mods.clear();
    FileStream *fs;
    if (generateModsMd5Entries)
        fs = new FileStream("MD5ModEntries.cpp", FileMode::Create, FileAccess::WriteOnly);
    if (generateMd5Entries)
        fs = new FileStream("MD5FileEntry" + QString::number((int)gameType) + ".cpp", FileMode::Create, FileAccess::WriteOnly);

    int lastProgress = -1;
    for (int l = 0; l < g_GameData->packageFiles.count(); l++)
    {
        int newProgress = (l + progress) * 100 / allFilesCount;
        if (g_ipc)
        {
            if (lastProgress != newProgress)
            {
                ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
                ConsoleSync();
                lastProgress = newProgress;
            }
        }
        else
        {
            PINFO("Checking: " + g_GameData->packageFiles[l] + "\n");
        }
        QByteArray md5 = calculateMD5(g_GameData->GamePath() + g_GameData->packageFiles[l]);
        bool found = false;
        for (int p = 0; p < entries.count(); p++)
        {
            if (memcmp(md5.data(), entries[p].md5, 16) == 0)
            {
                found = true;
                break;
            }
        }
        if (found)
            continue;

        found = false;
        for (int p = 0; p < modsEntriesSize; p++)
        {
            if (memcmp(md5.data(), modsEntries[p].md5, 16) == 0)
            {
                found = true;
                bool found2 = false;
                for (int s = 0; s < mods.count(); s++)
                {
                    if (mods[s].compare(modsEntries[p].modName) == 0)
                    {
                        found2 = true;
                        break;
                    }
                }
                if (!found2)
                    mods.push_back(modsEntries[p].modName);
                break;
            }
        }
        if (found)
            continue;

        found = false;
        for (int p = 0; p < badMODSize; p++)
        {
            if (memcmp(md5.data(), badMOD[p].md5, 16) == 0)
            {
                found = true;
                break;
            }
        }
        if (found)
            continue;

        bool foundPkg = false;
        quint8 md5Entry[16];
        QString package = g_GameData->packageFiles[l].toLower();
        auto range = std::equal_range(entries.begin(), entries.end(),
                                      package, Resources::ComparePath());
        for (auto it = range.first; it != range.second; it++)
        {
            if (it->path.compare(package, Qt::CaseSensitive) != 0)
                break;
            if (generateMd5Entries)
            {
                if (memcmp(md5.data(), it->md5, 16) == 0)
                {
                    foundPkg = true;
                    break;
                }
            }
            else
            {
                foundPkg = true;
                memcpy(md5Entry, it->md5, 16);
                break;
            }
        }
        if (!generateMd5Entries && !foundPkg)
            continue;
        if (generateMd5Entries && foundPkg)
            continue;

        vanilla = false;

        if (generateModsMd5Entries)
        {
            fs->WriteStringASCII(QString("{\"") + g_GameData->packageFiles[l] + "\",\n{ ");
            for (int i = 0; i < md5.count(); i++)
            {
                fs->WriteStringASCII(QString::number(md5[i], 16));
            }
            fs->WriteStringASCII("},\n\"\",\n},\n");
        }
        if (generateMd5Entries)
        {
            fs->WriteStringASCII(QString("{\"") + g_GameData->packageFiles[l] + "\",\n{ ");
            for (int i = 0; i < md5.count(); i++)
            {
                fs->WriteStringASCII(QString::number(md5[i], 16));
            }
            fs->WriteStringASCII(QString("},\n") + QString::number(QFile(g_GameData->packageFiles[l]).size()) + ",\n},\n");
        }

        if (!generateMd5Entries && !generateModsMd5Entries)
        {
            errors += "File " + g_GameData->packageFiles[l] + " has wrong MD5 checksum: ";
            for (int i = 0; i < md5.count(); i++)
            {
                errors += QString::number(md5[i], 16);
            }
            errors += "\n, expected: ";
            for (unsigned char i : md5Entry)
            {
                errors += QString::number(i, 16);
            }
            errors += "\n";
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR ") + g_GameData->packageFiles[l]);
                ConsoleSync();
            }
        }
    }
    progress += g_GameData->packageFiles.count();

    for (int l = 0; l < g_GameData->sfarFiles.count(); l++)
    {
        if (g_ipc)
        {
            int newProgress = (l + progress) * 100 / allFilesCount;
            if (lastProgress != newProgress)
            {
                ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
                ConsoleSync();
                lastProgress = newProgress;
            }
        }
        else
        {
            PINFO("Checking: " + g_GameData->sfarFiles[l] + "\n");
        }
        QByteArray md5 = calculateMD5(g_GameData->GamePath() + g_GameData->sfarFiles[l]);
        bool found = false;
        for (int p = 0; p < entries.count(); p++)
        {
            if (memcmp(md5.data(), entries[p].md5, 16) == 0)
            {
                found = true;
                break;
            }
        }
        if (found)
            continue;
        int index = -1;
        for (int p = 0; p < entries.count(); p++)
        {
            if (g_GameData->sfarFiles[l].compare(entries[p].path, Qt::CaseInsensitive) == 0)
            {
                index = p;
                break;
            }
        }
        if (index == -1)
            continue;

        vanilla = false;

        if (!generateMd5Entries && !generateModsMd5Entries)
        {
            errors += "File " + g_GameData->sfarFiles[l] + " has wrong MD5 checksum: ";
            for (int i = 0; i < md5.count(); i++)
            {
                errors += QString::number(md5[i], 16);
            }
            errors += "\n, expected: ";
            for (unsigned char i : entries[index].md5)
            {
                errors += QString::number(i, 16);
            }
            errors += "\n";

            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR ") + g_GameData->sfarFiles[l]);
                ConsoleSync();
            }
        }
    }
    progress += g_GameData->sfarFiles.count();

    for (int l = 0; l < g_GameData->tfcFiles.count(); l++)
    {
        if (g_ipc)
        {
            int newProgress = (l + progress) * 100 / allFilesCount;
            if (lastProgress != newProgress)
            {
                ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
                ConsoleSync();
                lastProgress = newProgress;
            }
        }
        else
        {
            PINFO("Checking: " + g_GameData->tfcFiles[l] + "\n");
        }
        QByteArray md5 = calculateMD5(g_GameData->GamePath() + g_GameData->tfcFiles[l]);
        bool found = false;
        for (int p = 0; p < entries.count(); p++)
        {
            if (memcmp(md5.data(), entries[p].md5, 16) == 0)
            {
                found = true;
                break;
            }
        }
        if (found)
            continue;
        int index = -1;
        for (int p = 0; p < entries.count(); p++)
        {
            if (g_GameData->tfcFiles[l].compare(entries[p].path, Qt::CaseInsensitive) == 0)
            {
                index = p;
                break;
            }
        }
        if (index == -1)
            continue;

        vanilla = false;

        if (!generateMd5Entries && !generateModsMd5Entries)
        {
            errors += "File " + g_GameData->tfcFiles[l] + " has wrong MD5 checksum: ";
            for (int i = 0; i < md5.count(); i++)
            {
                errors += QString::number(md5[i], 16);
            }
            errors += "\n, expected: ";
            for (unsigned char i : entries[index].md5)
            {
                errors += QString::number(i, 16);
            }
            errors += "\n";
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR ") + g_GameData->tfcFiles[l]);
                ConsoleSync();
            }
        }
    }
    progress += g_GameData->tfcFiles.count();
    if (generateModsMd5Entries || generateMd5Entries)
    {
        fs->Close();
        delete fs;
    }

    return vanilla;
}
