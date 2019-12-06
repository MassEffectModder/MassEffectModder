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

int Misc::ParseLegacyMe3xScriptMod(QList<TextureMapEntry> &textures, QString &script, QString &textureName)
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
                    if (AsciiStringMatchCaseIgnore(textures[i].name, textureName))
                    {
                        for (int l = 0; l < textures[i].list.count(); l++)
                        {
                            if (textures[i].list[l].path.length() == 0)
                                continue;
                            if (textures[i].list[l].exportID == exportId)
                            {
                                QString pkg = textures[i].list[l].path.split(QChar('/')).last().split(QChar('.')).first().toLower();
                                if (AsciiStringMatch(pkg, packageName))
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
                            if (AsciiStringMatch(pkg, packageName))
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

bool Misc::ParseBinaryModFileName(const QString &file, QString &pkgName, QString &dlcName, int &exportId)
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
    exportId = tmpExp.midRef(0).toInt() - 1;

    return true;
}

static uint GetCrcFromTpfList(QStringList &ddsList, const QString &filename)
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

bool Misc::DetectHashFromFile(const QString &file)
{
    int idx = file.indexOf("-hash", Qt::CaseInsensitive);
    return idx > 0;
}

bool Misc::convertDataModtoMem(QFileInfoList &files, QString &memFilePath,
                               MeType gameId, QList<TextureMapEntry> &textures, bool markToConvert,
                               ProgressCallback callback, void *callbackHandle)
{
    PINFO("Mods conversion started...\n");

    int result;
    QString fileName;
    quint64 dstLen = 0;
    QStringList ddsList;
    int numEntries = 0;

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
#ifdef GUI
                QApplication::processEvents();
#endif
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
                    TextureMapEntry f = textures[index];
                    mod.textureCrc = f.crc;
                    mod.textureName = f.name;
                    mod.binaryModType = 0;
                    mod.data = fs.ReadToBuffer(fs.ReadInt32());

                    Image image = Image(mod.data, ImageFormat::DDS);
                    if (!CheckImage(image, f, file, i))
                        continue;

                    PixelFormat newPixelFormat = f.pixfmt;
                    if (markToConvert)
                    {
                        newPixelFormat = changeTextureType(gameId, f.pixfmt, image.getPixelFormat(), f.flags);
                        if (f.pixfmt == newPixelFormat)
                            PINFO(QString("Warning for texture: ") + mod.textureName +
                                  " This texture can not be converted to desired format...\n");
                    }

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
#ifdef GUI
                QApplication::processEvents();
#endif
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
                            QString::number(i + 1) + " - mod: " + BaseName(file) + "\n");
                    }
                    ZipGoToNextFile(handle);
                    continue;
                }

                TextureMapEntry f = FoundTextureInTheMap(textures, crc);
                if (f.crc == 0)
                {
                    PINFO(QString("Texture skipped. File ") + fileName + QString().sprintf("_0x%08X", crc) +
                        " is not present in your game setup - mod: " + BaseName(file) + "\n");
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
                            ", skipping texture, entry: " + QString::number(i + 1) + " - mod: " + BaseName(file) + "\n");
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
                {
                    newPixelFormat = changeTextureType(gameId, f.pixfmt, image.getPixelFormat(), f.flags);
                    if (f.pixfmt == newPixelFormat)
                        PINFO(QString("Warning for texture: ") + mod.textureName +
                              " This texture can not be converted to desire format...\n");
                }

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
            TextureMapEntry f;
            bool entryMarkToConvert = markToConvert;
            uint crc = scanFilenameForCRC(file);
            if (crc == 0)
                continue;

            if (DetectMarkToConvertFromFile(file))
                entryMarkToConvert = true;

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
                QString filename = BaseName(file);
                int idx = filename.indexOf("0x");
                if (idx > 1)
                    f.name = filename.left(idx - 1);
                f.name += "-hash";
            }

            if (!Misc::CheckImage(image, f, file, -1))
                continue;

            if (!forceHash)
            {
                PixelFormat newPixelFormat = f.pixfmt;
                if (entryMarkToConvert)
                {
                    newPixelFormat = changeTextureType(gameId, f.pixfmt, image.getPixelFormat(), f.flags);
                    if (f.pixfmt == newPixelFormat)
                        PINFO(QString("Warning for texture: ") + mod.textureName +
                              " This texture can not be converted to desired format...\n");
                }

                int numMips = Misc::GetNumberOfMipsFromMap(f);
                CorrectTexture(image, f, numMips, entryMarkToConvert,
                               f.pixfmt, newPixelFormat, file);
            }

            mod.data = image.StoreImageToDDS();
            mod.textureName = f.name;
            mod.binaryModType = 0;
            mod.textureCrc = crc;
            mod.markConvert = entryMarkToConvert;
            mods.push_back(mod);
        }

        for (int l = 0; l < mods.count(); l++)
        {
#ifdef GUI
            QApplication::processEvents();
#endif
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
            int idx = fileMod.name.indexOf("-hash");
            if (idx != -1)
                fileMod.name = fileMod.name.left(idx);
            fileMod.offset = fs.ReadInt64();
            fileMod.size = fs.ReadInt64();
            modFiles.push_back(fileMod);
        }
        numFiles = modFiles.count();

        for (int i = 0; i < numFiles; i++, currentNumberOfTotalMods++)
        {
#ifdef GUI
            QApplication::processEvents();
#endif
            QString name;
            uint crc = 0;
            long size = 0;
            int exportId = -1;
            QString pkgPath;
            fs.JumpTo(modFiles[i].offset);
            size = modFiles[i].size;
            if (modFiles[i].tag == FileTextureTag || modFiles[i].tag == FileTextureTag2)
            {
                fs.ReadStringASCIINull(name);
                crc = fs.ReadUInt32();
            }
            else if (modFiles[i].tag == FileBinaryTag || modFiles[i].tag == FileXdeltaTag)
            {
                name = modFiles[i].name;
                exportId = fs.ReadInt32();
                fs.ReadStringASCIINull(pkgPath);
                pkgPath.replace('\\', '/');
            }

            PINFO(QString("Processing MEM mod ") + file.fileName() +
                             " - File " + QString::number(i + 1) + " of " +
                             QString::number(numFiles) + " - " + name + "\n");
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

            ByteBuffer dst = MipMaps::decompressData(fs, size);
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

            if (modFiles[i].tag == FileTextureTag || modFiles[i].tag == FileTextureTag2)
            {
                int idx = name.indexOf("-hash");
                if (idx != -1)
                    name = name.left(idx);
                QString filename = outputMODdir + "/" +
                        BaseName(name + QString().sprintf("_0x%08X", crc));
                if (idx != -1)
                    filename += "-hash";
                if (modFiles[i].tag == FileTextureTag)
                    filename += ".dds";
                else
                    filename += "-memconvert.dds";
                FileStream output = FileStream(filename, FileMode::Create, FileAccess::ReadWrite);
                output.WriteFromBuffer(dst);
            }
            else if (modFiles[i].tag == FileBinaryTag || modFiles[i].tag == FileXdeltaTag)
            {
                const QString& path = pkgPath;
                QString newFilename;
                if (path.contains("/DLC/"))
                {
                    QString dlcName = path.split('/')[3];
                    newFilename = "D" + QString::number(dlcName.size()) + "-" + dlcName + "-";
                }
                else
                {
                    newFilename = "B";
                }
                newFilename += QString::number(BaseName(path).size()) +
                        "-" + BaseName(path) + "-E" + QString::number(exportId + 1);
                if (modFiles[i].tag == FileBinaryTag)
                    newFilename += ".bin";
                else
                    newFilename += ".xdelta";
                newFilename = outputMODdir + "/" + newFilename;
                FileStream output = FileStream(newFilename, FileMode::Create, FileAccess::WriteOnly);
                output.WriteFromBuffer(dst);
            }
            else
            {
                if (g_ipc)
                {
                    ConsoleWrite(QString("[IPC]ERROR_FILE_NOT_COMPATIBLE ") + file.absoluteFilePath());
                    ConsoleSync();
                }
                PERROR(QString("Unknown tag for file: ") + name + "\n");
            }
            dst.Free();
        }
    }

    long elapsed = Misc::elapsedTime();
    PINFO(Misc::getTimerFormat(elapsed) + "\n");

    PINFO("Extract MEM mod files completed.\n\n");
    return true;
}

bool Misc::extractMOD(QFileInfoList &list, QList<TextureMapEntry> &textures, QString &outputDir)
{
    PINFO("Extract MOD files started...\n");

    bool status = true;

    outputDir = QDir::cleanPath(outputDir);
    if (outputDir.length() != 0)
    {
        QDir().mkpath(outputDir);
        outputDir += "/";
    }

    foreach (QFileInfo file, list)
    {
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + file.fileName());
            ConsoleSync();
        }
        PINFO(QString("Extract MOD: ") + file.fileName() + "\n");
        QString outputMODdir = outputDir + BaseNameWithoutExt(file.fileName());
        QDir().mkpath(outputMODdir);

        FileStream fs = FileStream(file.absoluteFilePath(), FileMode::Open, FileAccess::ReadOnly);
        uint numEntries = Misc::ReadModHeader(fs);
        for (uint i = 0; i < numEntries; i++)
        {
            QString scriptLegacy;
            bool binary;
            QString textureName;
            Misc::ReadModEntryHeader(fs, scriptLegacy, binary, textureName);

            if (binary)
            {
                int exportId = -1;
                QString path;
                QString package;
                Misc::ParseME3xBinaryScriptMod(scriptLegacy, package, exportId, path);
                if (exportId == -1 || package.length() == 0 || path.length() == 0)
                {
                    fs.Skip(fs.ReadInt32());
                    PERROR(QString("Skipping not compatible content, entry: ") +
                                 QString::number(i + 1) + " - mod: " + file.fileName() + "\n");
                    status = false;
                    continue;
                }
                path += "/" + package;
                QString newFilename;
                if (path.contains("/DLC/"))
                {
                    QString dlcName = path.split(QChar('/'))[3];
                    newFilename = "D" + QString::number(dlcName.size()) + "-" + dlcName + "-";
                }
                else
                {
                    newFilename = "B";
                }
                newFilename += QString::number(BaseName(path).size()) + "-" +
                        BaseName(path) + "-E" + QString::number(exportId + 1) + ".bin";
                QString outputFile = outputMODdir + "/" + newFilename;
                if (QFile(outputFile).exists())
                    QFile(outputFile).remove();
                FileStream fs2 = FileStream(outputFile, FileMode::Create);
                fs2.CopyFrom(fs, fs.ReadInt32());
                PINFO(outputFile);
            }
            else
            {
                int index = Misc::ParseLegacyMe3xScriptMod(textures, scriptLegacy, textureName);
                if (index == -1)
                {
                    fs.Skip(fs.ReadInt32());
                    PERROR(QString("Skipping not compatible content, entry: ") +
                                 QString::number(i + 1) + " - mod: " + file.fileName() + "\n");
                    status = false;
                    continue;
                }
                QString newFile = outputMODdir + "/" + textures[index].name +
                        QString().sprintf("_0x%08X", textures[index].crc) + ".dds";
                if (QFile(newFile).exists())
                    QFile(newFile).remove();
                FileStream fs2 = FileStream(newFile, FileMode::Create);
                fs2.CopyFrom(fs, fs.ReadInt32());
                PINFO(newFile + "\n");
            }
        }
    }

    PINFO("Extract MOD files completed.\n\n");
    return status;
}

bool Misc::extractTPF(QFileInfoList &list, QString &outputDir)
{
    PINFO("Extract TPF files started...\n");

    bool status = true;
    int result;
    QString fileName;
    quint64 dstLen = 0;
    int numEntries = 0;

    outputDir = QDir::cleanPath(outputDir);
    if (outputDir.length() != 0)
    {
        QDir().mkpath(outputDir);
        outputDir += "/";
    }

    foreach (QFileInfo file, list)
    {
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + file.fileName());
            ConsoleSync();
        }
        else
        {
            PINFO(QString("Extract TPF: ") + file.fileName() + "\n");
        }
        QString outputTPFdir = outputDir + BaseNameWithoutExt(file.fileName());
        QDir().mkpath(outputTPFdir);

#if defined(_WIN32)
        auto str = file.absoluteFilePath().toStdWString();
#else
        auto str = file.absoluteFilePath().toStdString();
#endif
        auto name = str.c_str();
        void *handle = ZipOpenFromFile(name, &numEntries, 1);
        if (handle == nullptr)
            goto failed;

        for (int i = 0; i < numEntries; i++)
        {
            if (!Misc::TpfGetCurrentFileInfo(handle, fileName, dstLen))
                goto failed;
            if (fileName.endsWith(".def", Qt::CaseInsensitive) ||
                fileName.endsWith(".log", Qt::CaseInsensitive))
            {
                ZipGoToNextFile(handle);
                continue;
            }

            auto data = ByteBuffer(dstLen);
            result = ZipReadCurrentFile(handle, data.ptr(), dstLen, nullptr);
            if (result != 0)
            {
                ZipGoToNextFile(handle);
                data.Free();
                goto failed;
            }

            QString outputPath = outputTPFdir + "/" + fileName;
            if (QFile(outputPath).exists())
                QFile(outputPath).remove();
            FileStream fs = FileStream(outputPath, FileMode::Create);
            fs.WriteFromBuffer(data);
            data.Free();
            PINFO(outputPath + "\n");
            ZipGoToNextFile(handle);
        }
        ZipClose(handle);
        handle = nullptr;
        continue;

failed:

        PERROR(QString("TPF file is damaged: ") + file.fileName() + "\n");
        if (handle != nullptr)
            ZipClose(handle);
        handle = nullptr;
    }

    PINFO("Extract TPF files completed.\n\n");
    return status;
}
