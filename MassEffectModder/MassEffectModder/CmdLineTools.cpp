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

#include "Exceptions/SignalHandler.h"
#include "Helpers/MiscHelpers.h"
#include "Helpers/Logs.h"

#include "CmdLineTools.h"
#include "ConfigIni.h"
#include "Resources.h"
#include "TreeScan.h"
#include "GameData.h"
#include "Misc.h"
#include "DLC.h"
#include "MipMaps.h"
#include "Wrappers.h"
#include "LODSettings.h"
#include "TOCFile.h"
#include "Texture.h"
#include "MemTypes.h"

static bool CheckGamePath()
{
    if (g_GameData->GamePath().length() == 0 || !QDir(g_GameData->GamePath()).exists())
    {
        PERROR("Error: Could not found the game!\n");
        return false;
    }

    return true;
}

int CmdLineTools::scanTextures(MeType gameId, bool removeEmptyMips)
{
    int errorCode;

    auto configIni = ConfigIni{};
    g_GameData->Init(gameId, configIni);
    if (!CheckGamePath())
        return 1;

    PINFO("Scan textures started...\n");

    QList<FoundTexture> textures;
    Resources resources;
    QStringList pkgsToMarkers;
    QStringList pkgsToRepack;

    resources.loadMD5Tables();
    Misc::startTimer();
    errorCode = TreeScan::PrepareListOfTextures(gameId, resources, textures, removeEmptyMips);
    long elapsed = Misc::elapsedTime();
    PINFO(Misc::getTimerFormat(elapsed) + "\n");

    PINFO("Scan textures finished.\n\n");

    return errorCode;
}

bool CmdLineTools::updateTOCs()
{
    ConfigIni configIni = ConfigIni();
    g_GameData->Init(MeType::ME3_TYPE, configIni);
    if (!CheckGamePath())
        return false;

    TOCBinFile::UpdateAllTOCBinFiles();

    return true;
}

bool CmdLineTools::unpackAllDLCs()
{
    ConfigIni configIni = ConfigIni();
    g_GameData->Init(MeType::ME3_TYPE, configIni);
    if (!CheckGamePath())
        return false;

    Misc::startTimer();
    ME3DLC::unpackAllDLC();
    long elapsed = Misc::elapsedTime();
    PINFO(Misc::getTimerFormat(elapsed) + "\n");

    return true;
}

bool CmdLineTools::repackGame(MeType gameId)
{
    ConfigIni configIni = ConfigIni();
    g_GameData->Init(gameId, configIni);
    if (!CheckGamePath())
        return false;
    Misc::startTimer();
    Repack(gameId);
    long elapsed = Misc::elapsedTime();
    PINFO(Misc::getTimerFormat(elapsed) + "\n");

    return true;
}

bool CmdLineTools::unpackArchive(const QString &inputFile, QString &outputDir)
{
    outputDir = QDir::cleanPath(outputDir);
    if (outputDir != "")
        QDir().mkpath(outputDir);
#if defined(_WIN32)
    auto strFile = inputFile.toStdWString();
    auto strOut = outputDir.toStdWString();
#else
    auto strFile = inputFile.toStdString();
    auto strOut = outputDir.toStdString();
#endif
    auto fileName = strFile.c_str();
    auto outPath = strOut.c_str();

    if (inputFile.endsWith(".zip", Qt::CaseInsensitive))
        return ZipUnpack(fileName, outPath, true) == 0;
    if (inputFile.endsWith(".7z", Qt::CaseInsensitive))
        return SevenZipUnpack(fileName, outPath, true) == 0;
    if (inputFile.endsWith(".rar", Qt::CaseInsensitive))
        return RarUnpack(fileName, outPath, true) == 0;

    return false;
}

bool CmdLineTools::applyModTag(MeType gameId, int MeuitmV, int AlotV)
{
    QString path;
    if (gameId == MeType::ME1_TYPE)
        path = "/BioGame/CookedPC/testVolumeLight_VFX.upk";
    else if (gameId == MeType::ME2_TYPE)
        path = "/BioGame/CookedPC/BIOC_Materials.pcc";
    else if (gameId == MeType::ME3_TYPE)
        path = "/BIOGame/CookedPCConsole/adv_combat_tutorial_xbox_D_Int.afc";

    FileStream fs = FileStream(g_GameData->GamePath() + path, FileMode::Open, FileAccess::ReadWrite);
    fs.Seek(-16, SeekOrigin::End);
    int prevMeuitmV = fs.ReadInt32();
    int prevAlotV = fs.ReadInt32();
    int prevProductV = fs.ReadInt32();
    uint memiTag = fs.ReadUInt32();
    if (memiTag == MEMI_TAG)
    {
        if (prevProductV < 10 || prevProductV == 4352 || prevProductV == 16777472) // default before MEM v178
            prevProductV = prevAlotV = prevMeuitmV = 0;
    }
    else
        prevProductV = prevAlotV = prevMeuitmV = 0;
    if (MeuitmV != 0)
        prevMeuitmV = MeuitmV;
    if (AlotV != 0)
        prevAlotV = AlotV;
    fs.WriteInt32(prevMeuitmV);
    fs.WriteInt32(prevAlotV);
    fs.WriteInt32((prevProductV & 0xffff0000) | QString(MEM_VERSION).toInt());
    fs.WriteUInt32(MEMI_TAG);

    return true;
}

bool CmdLineTools::ConvertToMEM(MeType gameId, QString &inputDir, QString &memFile, bool markToConvert)
{
    QList<FoundTexture> textures;
    Resources resources;
    resources.loadMD5Tables();
    TreeScan::loadTexturesMap(gameId, resources, textures);
    bool status = Misc::convertDataModtoMem(inputDir, memFile, gameId, textures, markToConvert, false);
    return status;
}

bool CmdLineTools::convertGameTexture(MeType gameId, const QString &inputFile, QString &outputFile, QList<FoundTexture> &textures,
                                      bool markToConvert)
{
    uint crc = Misc::scanFilenameForCRC(inputFile);
    if (crc == 0)
        return false;

    FoundTexture foundTex = Misc::FoundTextureInTheMap(textures, crc);
    if (foundTex.crc == 0)
    {
        PINFO(QString("Texture skipped. Texture ") + BaseName(inputFile) +
                     " is not present in your game setup.\n");
        return false;
    }

    Image image = Image(inputFile);
    if (!Misc::CheckImage(image, foundTex, inputFile, -1))
        return false;

    PixelFormat newPixelFormat = foundTex.pixfmt;
    if (markToConvert)
        newPixelFormat = Misc::changeTextureType(gameId, foundTex.pixfmt, image.getPixelFormat(), foundTex.flags);

    bool dxt1HasAlpha = false;
    quint8 dxt1Threshold = 128;
    if (foundTex.flags == TexProperty::TextureTypes::OneBitAlpha)
    {
        dxt1HasAlpha = true;
        if (image.getPixelFormat() == PixelFormat::ARGB ||
            image.getPixelFormat() == PixelFormat::DXT3 ||
            image.getPixelFormat() == PixelFormat::DXT5)
        {
            PINFO(QString("Warning for texture: ") + BaseName(inputFile) +
                         ". This texture converted from full alpha to binary alpha.\n");
        }
    }
    image.correctMips(newPixelFormat, dxt1HasAlpha, dxt1Threshold);
    if (QFile(outputFile).exists())
        QFile(outputFile).remove();
    FileStream fs = FileStream(outputFile, FileMode::Create, FileAccess::WriteOnly);
    ByteBuffer buffer = image.StoreImageToDDS();
    fs.WriteFromBuffer(buffer);
    buffer.Free();

    return true;
}

bool CmdLineTools::convertGameImage(MeType gameId, QString &inputFile, QString &outputFile, bool markToConvert)
{
    QList<FoundTexture> textures;
    Resources resources;
    resources.loadMD5Tables();

    TreeScan::loadTexturesMap(gameId, resources, textures);
    return convertGameTexture(gameId, inputFile, outputFile, textures, markToConvert);
}

bool CmdLineTools::convertGameImages(MeType gameId, QString &inputDir, QString &outputDir, bool markToConvert)
{
    QList<FoundTexture> textures;
    Resources resources;
    resources.loadMD5Tables();

    TreeScan::loadTexturesMap(gameId, resources, textures);

    inputDir = QDir::cleanPath(inputDir);
    QFileInfoList list;
    list += QDir(inputDir, "*.dds", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    list += QDir(inputDir, "*.png", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    list += QDir(inputDir, "*.bmp", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    list += QDir(inputDir, "*.tga", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();

    outputDir = QDir::cleanPath(outputDir);
    QDir().mkpath(outputDir);

    bool status = true;
    foreach (QFileInfo file, list)
    {
        QString outputFile = outputDir + "/" + BaseNameWithoutExt(file.fileName()) + ".dds";
        if (!convertGameTexture(gameId, file.absoluteFilePath(), outputFile, textures, markToConvert))
            status = false;
    }

    return status;
}

bool CmdLineTools::convertImage(QString &inputFile, QString &outputFile, QString &format, int dxt1Threshold)
{
    format = format.toLower();
    PixelFormat pixFmt;
    bool dxt1HasAlpha = false;
    if (format == "dxt1")
        pixFmt = PixelFormat::DXT1;
    else if (format == "dxt1a")
    {
        pixFmt = PixelFormat::DXT1;
        dxt1HasAlpha = true;
    }
    else if (format == "dxt1a")
        pixFmt = PixelFormat::DXT3;
    else if (format == "dxt5")
        pixFmt = PixelFormat::DXT5;
    else if (format == "ati2")
        pixFmt = PixelFormat::ATI2;
    else if (format == "v8u8")
        pixFmt = PixelFormat::V8U8;
    else if (format == "argb")
        pixFmt = PixelFormat::ARGB;
    else if (format == "rgb")
        pixFmt = PixelFormat::RGB;
    else if (format == "g8")
        pixFmt = PixelFormat::G8;
    else
    {
        PERROR(QString("Error: not supported format: ") + format + "\n");
        return false;
    }

    Image image = Image(inputFile);
    if (image.getMipMaps().count() == 0)
    {
        PERROR("Texture not compatible!\n");
        return false;
    }
    if (QFile(outputFile).exists())
        QFile(outputFile).remove();
    image.correctMips(pixFmt, dxt1HasAlpha, dxt1Threshold);
    FileStream fs = FileStream(outputFile, FileMode::Create, FileAccess::WriteOnly);
    ByteBuffer buffer = image.StoreImageToDDS();
    fs.WriteFromBuffer(buffer);
    buffer.Free();

    return true;
}

bool CmdLineTools::extractTPF(QString &inputDir, QString &outputDir)
{
    PINFO("Extract TPF files started...\n");

    bool status = true;
    int result;
    QString fileName;
    quint64 dstLen = 0;
    int numEntries = 0;

    inputDir = QDir::cleanPath(inputDir);
    QFileInfoList list;
    if (inputDir.endsWith(".tpf", Qt::CaseInsensitive))
    {
        list.push_back(QFileInfo(inputDir));
    }
    else
    {
        list = QDir(inputDir, "*.tpf",
                     QDir::SortFlag::Name | QDir::SortFlag::IgnoreCase,
                     QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    }

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

bool CmdLineTools::extractMOD(MeType gameId, QString &inputDir, QString &outputDir)
{
    QList<FoundTexture> textures;
    Resources resources;
    resources.loadMD5Tables();

    TreeScan::loadTexturesMap(gameId, resources, textures);

    PINFO("Extract MOD files started...\n");

    bool status = true;

    inputDir = QDir::cleanPath(inputDir);
    QFileInfoList list;
    if (inputDir.endsWith(".mod", Qt::CaseInsensitive))
    {
        list.push_back(QFileInfo(inputDir));
    }
    else
    {
        list = QDir(inputDir, "*.mod",
                     QDir::SortFlag::Name | QDir::SortFlag::IgnoreCase,
                     QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    }

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
                        BaseName(path) + "-E" + QString::number(exportId) + ".bin";
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

bool CmdLineTools::extractMEM(MeType gameId, QString &inputDir, QString &outputDir)
{
    QList<FoundTexture> textures;
    Resources resources;
    resources.loadMD5Tables();

    TreeScan::loadTexturesMap(gameId, resources, textures);

    PINFO("Extract MEM files started...\n");

    inputDir = QDir::cleanPath(inputDir);
    QFileInfoList list;
    if (inputDir.endsWith(".mem", Qt::CaseInsensitive))
    {
        list.push_back(QFileInfo(inputDir));
    }
    else
    {
        list = QDir(inputDir, "*.mem",
                                 QDir::SortFlag::Name | QDir::SortFlag::IgnoreCase,
                                 QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    }

    outputDir = QDir::cleanPath(outputDir);
    if (outputDir.length() != 0)
    {
        QDir().mkpath(outputDir);
        outputDir += "/";
    }

    Misc::startTimer();

    int currentNumberOfTotalMods = 1;
    int totalNumberOfMods = 0;
    for (int i = 0; i < list.count(); i++)
    {
        FileStream fs = FileStream(list[i].absoluteFilePath(), FileMode::Open, FileAccess::ReadOnly);
        uint tag = fs.ReadUInt32();
        uint version = fs.ReadUInt32();
        if (tag != TextureModTag || version != TextureModVersion)
            continue;
        fs.JumpTo(fs.ReadInt64());
        fs.SkipInt32();
        totalNumberOfMods += fs.ReadInt32();
    }

    int lastProgress = -1;
    foreach (QFileInfo file, list)
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
            if (g_ipc && lastProgress != newProgress)
            {
                ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
                ConsoleSync();
                lastProgress = newProgress;
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
                        "-" + BaseName(path) + "-E" + QString::number(exportId);
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

bool CmdLineTools::ApplyME1LAAPatch()
{
    ConfigIni configIni{};
    g_GameData->Init(MeType::ME1_TYPE, configIni);
    if (!CheckGamePath())
        return false;

    if (!Misc::ApplyLAAForME1Exe())
        return false;
    if (!Misc::ChangeProductNameForME1Exe())
        return false;

    return true;
}

bool CmdLineTools::ApplyLODAndGfxSettings(MeType gameId, bool softShadowsME1, bool meuitmMode, bool limit2k)
{
    g_GameData->Init(gameId);
    if (g_GameData->ConfigIniPath().length() == 0)
    {
        PERROR("Game User path is not defined.\n");
        return false;
    }
    QString path = g_GameData->EngineConfigIniPath();
    QDir().mkpath(DirName(path));
#if !defined(_WIN32)
    if (QFile(path).exists())
    {
        if (!Misc::ConvertEndLines(path, true))
            return false;
    }
#endif
    ConfigIni engineConf = ConfigIni(path);
    LODSettings::updateLOD(gameId, engineConf, limit2k);
    LODSettings::updateGFXSettings(gameId, engineConf, softShadowsME1, meuitmMode);
#if !defined(_WIN32)
    Misc::ConvertEndLines(path, false);
#endif

    return true;
}

bool CmdLineTools::RemoveLODSettings(MeType gameId)
{
    g_GameData->Init(gameId);
    if (g_GameData->ConfigIniPath().length() == 0)
    {
        PERROR("Game User path is not defined.\n");
        return false;
    }
    QString path = g_GameData->EngineConfigIniPath();
    if (!QFile(path).exists())
        return true;
#if !defined(_WIN32)
    if (!Misc::ConvertEndLines(path, true))
        return false;
#endif
    ConfigIni engineConf = ConfigIni(path);
    LODSettings::removeLOD(gameId, engineConf);
#if !defined(_WIN32)
    Misc::ConvertEndLines(path, false);
#endif

    return true;
}

bool CmdLineTools::PrintLODSettings(MeType gameId)
{
    g_GameData->Init(gameId);
    if (g_GameData->ConfigIniPath().length() == 0)
    {
        PERROR("Game User path is not defined.\n");
        return false;
    }
    QString path = g_GameData->EngineConfigIniPath();
    bool exist = QFile(path).exists();
    if (!exist)
        return true;
#if !defined(_WIN32)
    Misc::ConvertEndLines(path, true);
#endif
    ConfigIni engineConf = ConfigIni(path);
    if (g_ipc)
    {
        LODSettings::readLODIpc(gameId, engineConf);
    }
    else
    {
        QString log;
        LODSettings::readLOD(gameId, engineConf, log);
        PINFO(log);
    }
#if !defined(_WIN32)
    Misc::ConvertEndLines(path, false);
#endif

    return true;
}

bool CmdLineTools::CheckGameData(MeType gameId)
{
    ConfigIni configIni{};
    g_GameData->Init(gameId, configIni);
    if (!CheckGamePath())
        return false;

    QString errors;
    QStringList modList;
    Resources resources;

    resources.loadMD5Tables();

    bool vanilla = Misc::checkGameFiles(gameId, resources, errors, modList);

    if (!g_ipc)
    {
        PERROR(errors);
        if (modList.count() != 0)
        {
            PERROR("\n------- Detected mods --------\n");
            for (int l = 0; l < modList.count(); l++)
            {
                PERROR(modList[l] + "\n");
            }
            PERROR("------------------------------\n\n");
        }
    }

    if (!vanilla && !g_ipc)
    {
        PERROR("===========================================================================\n");
        PERROR("WARNING: looks like the following file(s) are not vanilla or not recognized\n");
        PERROR("===========================================================================\n\n");
        PERROR(errors);
    }

    return vanilla;
}

bool CmdLineTools::CheckForMarkers(MeType gameId)
{
    ConfigIni configIni{};
    g_GameData->Init(gameId, configIni);
    if (!CheckGamePath())
        return false;

    QString path;
    if (GameData::gameType == MeType::ME1_TYPE)
        path = "/BioGame/CookedPC/testVolumeLight_VFX.upk";
    else if (GameData::gameType == MeType::ME2_TYPE)
        path = "/BioGame/CookedPC/BIOC_Materials.pcc";

    QStringList packages;
    for (int i = 0; i < g_GameData->packageFiles.count(); i++)
    {
        if (path.length() != 0 && g_GameData->packageFiles[i].compare(path, Qt::CaseInsensitive) == 0)
            continue;
        packages.push_back(g_GameData->packageFiles[i]);
    }

    int lastProgress = -1;
    for (int i = 0; i < packages.count(); i++)
    {
        int newProgress = (i + 1) * 100 / packages.count();
        if (g_ipc && lastProgress != newProgress)
        {
            ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
            ConsoleSync();
            lastProgress = newProgress;
        }

        FileStream fs = FileStream(g_GameData->GamePath() + packages[i], FileMode::Open, FileAccess::ReadOnly);
        fs.Seek(-MEMMarkerLenght, SeekOrigin::End);
        QString marker;
        fs.ReadStringASCII(marker, MEMMarkerLenght);
        if (marker == QString(MEMendFileMarker))
        {
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR_FILEMARKER_FOUND ") + packages[i]);
                ConsoleSync();
            }
            else
            {
                PERROR(QString("Error: detected marker: ") + packages[i] + "\n");
            }
        }
    }

    return true;
}

bool CmdLineTools::DetectBadMods(MeType gameId)
{
    ConfigIni configIni{};
    g_GameData->Init(gameId, configIni);
    if (!CheckGamePath())
        return false;

    QStringList badMods;
    Misc::detectBrokenMod(badMods);
    if (badMods.count() != 0)
    {
        if (!g_ipc)
            PERROR("Error: Detected not compatible mods:\n");
        for (int l = 0; l < badMods.count(); l++)
        {
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR ") + badMods[l]);
                ConsoleSync();
            }
            else
            {
                PERROR(badMods[l] + "\n");
            }
        }
    }

    return true;
}

bool CmdLineTools::DetectMods(MeType gameId)
{
    ConfigIni configIni{};
    g_GameData->Init(gameId, configIni);
    if (!CheckGamePath())
        return false;

    QStringList mods;
    Misc::detectMods(mods);
    if (mods.count() != 0)
    {
        if (!g_ipc)
            PINFO("Detected mods:\n");
        for (int l = 0; l < mods.count(); l++)
        {
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]MOD ") + mods[l]);
                ConsoleSync();
            }
            else
            {
                PINFO(mods[l] + "\n");
            }
        }
    }

    return true;
}

bool CmdLineTools::detectMod(MeType gameId)
{
    QString path;
    if (gameId == MeType::ME1_TYPE)
        path = "/BioGame/CookedPC/testVolumeLight_VFX.upk";
    else if (gameId == MeType::ME2_TYPE)
        path = "/BioGame/CookedPC/BIOC_Materials.pcc";
    else
        path = "/BIOGame/CookedPCConsole/adv_combat_tutorial_xbox_D_Int.afc";

    FileStream fs = FileStream(g_GameData->GamePath() + path, FileMode::Open, FileAccess::ReadOnly);
    fs.Seek(-4, SeekOrigin::End);
    auto tag = fs.ReadUInt32();
    return tag == MEMI_TAG;
}

void CmdLineTools::AddMarkers()
{
    PINFO("Adding markers started...\n");
    if (g_ipc)
    {
        ConsoleWrite("[IPC]STAGE_CONTEXT STAGE_MARKERS");
        ConsoleSync();
    }
    int lastProgress = -1;
    for (int i = 0; i < pkgsToMarker.count(); i++)
    {
        int newProgress = (i + 1) * 100 / pkgsToMarker.count();
        if (g_ipc && lastProgress != newProgress)
        {
            ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
            ConsoleSync();
            lastProgress = newProgress;
        }
        FileStream fs = FileStream(g_GameData->GamePath() + pkgsToMarker[i], FileMode::Open, FileAccess::ReadWrite);
        fs.Seek(-MEMMarkerLenght, SeekOrigin::End);
        QString marker;
        fs.ReadStringASCII(marker, MEMMarkerLenght);
        QString str(MEMendFileMarker);
        if (marker != str)
        {
            fs.SeekEnd();
            fs.WriteStringASCII(str);
        }
    }
    PINFO("Adding markers finished.\n\n");
}

bool CmdLineTools::ScanTextures(MeType gameId, Resources &resources, QList<FoundTexture> &textures)
{
    PINFO("Scan textures started...\n");
    TreeScan::PrepareListOfTextures(gameId, resources, textures);
    PINFO("Scan textures finished.\n\n");

    return true;
}

bool CmdLineTools::RemoveMipmaps(MipMaps &mipMaps, QList<FoundTexture> &textures,
                                 QStringList &pkgsToMarker, QStringList &pkgsToRepack,
                                 bool repack, bool appendMarker)
{
    PINFO("Remove empty mipmaps started...\n");
    if (g_ipc)
    {
        ConsoleWrite("[IPC]STAGE_CONTEXT STAGE_REMOVEMIPMAPS");
        ConsoleSync();
    }

    mipMaps.removeMipMaps(1, textures, pkgsToMarker, pkgsToRepack, repack, appendMarker);
    if (GameData::gameType == MeType::ME1_TYPE)
        mipMaps.removeMipMaps(2, textures, pkgsToMarker, pkgsToRepack, repack, appendMarker);

    PINFO("Remove empty mipmaps finished.\n\n");

    return true;
}

void CmdLineTools::Repack(MeType gameId)
{
    for (int i = 0; i < g_GameData->packageFiles.count(); i++)
    {
        pkgsToRepack.push_back(g_GameData->packageFiles[i]);
    }
    RepackME23(gameId, false);
    if (GameData::gameType == MeType::ME3_TYPE)
        TOCBinFile::UpdateAllTOCBinFiles();
}

void CmdLineTools::RepackME23(MeType gameId, bool appendMarker)
{
    PINFO("Repack started...\n");
    if (g_ipc)
    {
        ConsoleWrite("[IPC]STAGE_CONTEXT STAGE_REPACK");
        ConsoleSync();
    }

    if (gameId == MeType::ME2_TYPE)
        pkgsToRepack.removeOne("/BioGame/CookedPC/BIOC_Materials.pcc");
    int lastProgress = -1;
    for (int i = 0; i < pkgsToRepack.count(); i++)
    {
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + pkgsToRepack[i]);
            ConsoleSync();
        }
        else
        {
            PINFO(QString("Repack " + QString::number(i + 1) + "/" +
                                 QString::number(pkgsToRepack.count()) +
                                 " ") + pkgsToRepack[i] + "\n");
        }
        int newProgress = (i * 100 / pkgsToRepack.count());
        if (g_ipc && lastProgress != newProgress)
        {
            ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
            ConsoleSync();
            lastProgress = newProgress;
        }
        auto package = new Package();
        package->Open(g_GameData->GamePath() + pkgsToRepack[i], true);
        if (!package->getCompressedFlag() || (package->getCompressedFlag() &&
             package->compressionType != Package::CompressionType::Zlib))
        {
            delete package;
            package = new Package();
            package->Open(g_GameData->GamePath() + pkgsToRepack[i]);
            package->SaveToFile(true, false, appendMarker);
        }
        delete package;
    }
    PINFO("Repack finished.\n\n");
}

bool CmdLineTools::InstallMods(MeType gameId, QString &inputDir, bool repack,
                               bool guiInstaller, bool limit2k, bool verify, int cacheAmount)
{
    Resources resources;
    MipMaps mipMaps;
    resources.loadMD5Tables();
    ConfigIni configIni = ConfigIni();
    g_GameData->Init(gameId, configIni);
    if (!CheckGamePath())
        return false;

    if (g_GameData->ConfigIniPath().length() == 0)
    {
        PERROR("Game User path is not defined.\n");
        return false;
    }

    if (!guiInstaller)
    {
        if (gameId == MeType::ME1_TYPE && !QFile(g_GameData->EngineConfigIniPath()).exists())
        {
            PERROR("Error: Missing game configuration file.\nYou need atleast once launch the game first.\n");
            return false;
        }
        bool writeAccess = Misc::CheckAndCorrectAccessToGame();
        if (!writeAccess)
        {
            PERROR("Error: Detected no write access to game folders.\n");
            return false;
        }

        QStringList badMods;
        Misc::detectBrokenMod(badMods);
        if (badMods.count() != 0)
        {
            PERROR("Error: Detected not compatible mods:\n");
            for (int l = 0; l < badMods.count(); l++)
            {
                PERROR(badMods[l] + "\n");
            }
            return false;
        }
    }

    Misc::startTimer();

    if (gameId == MeType::ME1_TYPE)
        repack = false;

    bool modded = detectMod(gameId);
    bool unpackNeeded = false;
    if (gameId == MeType::ME3_TYPE && !modded)
        unpackNeeded = Misc::unpackSFARisNeeded();
    if (g_ipc)
    {
        if (!modded)
        {
            if (gameId == MeType::ME3_TYPE && unpackNeeded)
                ConsoleWrite("[IPC]STAGE_ADD STAGE_UNPACKDLC");
            ConsoleWrite("[IPC]STAGE_ADD STAGE_PRESCAN");
            ConsoleWrite("[IPC]STAGE_ADD STAGE_SCAN");
        }
        ConsoleWrite("[IPC]STAGE_ADD STAGE_INSTALLTEXTURES");
        if (!modded)
            ConsoleWrite("[IPC]STAGE_ADD STAGE_REMOVEMIPMAPS");
        if (repack)
            ConsoleWrite("[IPC]STAGE_ADD STAGE_REPACK");
        if (!modded)
            ConsoleWrite("[IPC]STAGE_ADD STAGE_MARKERS");
        ConsoleSync();
    }

    if (gameId == MeType::ME3_TYPE && !modded && unpackNeeded)
    {
        PINFO("Unpacking DLCs started...\n");
        if (g_ipc)
        {
            ConsoleWrite("[IPC]STAGE_CONTEXT STAGE_UNPACKDLC");
            ConsoleSync();
        }

        ME3DLC::unpackAllDLC();

        g_GameData->Init(gameId, configIni, true);

        PINFO("Unpacking DLCs finished.\n\n");
    }

    if (repack)
    {
        for (int i = 0; i < g_GameData->packageFiles.count(); i++)
        {
            pkgsToRepack.push_back(g_GameData->packageFiles[i]);
        }
        if (GameData::gameType == MeType::ME1_TYPE)
            pkgsToRepack.removeOne("/BioGame/CookedPC/testVolumeLight_VFX.upk");
        else if (GameData::gameType == MeType::ME2_TYPE)
            pkgsToRepack.removeOne("/BioGame/CookedPC/BIOC_Materials.pcc");
    }

    QList<FoundTexture> textures;

    if (!modded)
    {
        for (int i = 0; i < g_GameData->packageFiles.count(); i++)
        {
            pkgsToMarker.push_back(g_GameData->packageFiles[i]);
        }
        if (GameData::gameType == MeType::ME1_TYPE)
            pkgsToMarker.removeOne("/BioGame/CookedPC/testVolumeLight_VFX.upk");
        else if (GameData::gameType == MeType::ME2_TYPE)
            pkgsToMarker.removeOne("/BioGame/CookedPC/BIOC_Materials.pcc");

        ScanTextures(gameId, resources, textures);
    }


    PINFO("Process textures started...\n");
    if (g_ipc)
    {
        ConsoleWrite("[IPC]STAGE_CONTEXT STAGE_INSTALLTEXTURES");
        ConsoleSync();
    }
    auto files = QDir(inputDir, "*.mem",
                      QDir::SortFlag::Name | QDir::SortFlag::IgnoreCase,
                      QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    QStringList modFiles;
    foreach (QFileInfo file, files)
    {
        modFiles.push_back(file.absoluteFilePath());
    }
    if (modded)
    {
        QString path = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation).first() +
                "/MassEffectModder";
        QString mapFile = path + QString("/me%1map.bin").arg((int)gameId);
        if (!TreeScan::loadTexturesMapFile(mapFile, textures))
            return false;
    }

    QString tfcName;
    QByteArray guid;
    applyMods(modFiles, textures, mipMaps, repack, modded, tfcName, guid, false, verify, false, cacheAmount);


    if (!modded)
        RemoveMipmaps(mipMaps, textures, pkgsToMarker, pkgsToRepack, repack, true);


    if (repack)
        RepackME23(gameId, !modded);


    if (!modded)
        AddMarkers();


    if (!guiInstaller)
    {
        if (!applyModTag(gameId, 0, 0))
            PERROR("Failed applying stamp for installation!\n");
        PINFO("Updating LODs and other settings started...\n");
        QString path = g_GameData->EngineConfigIniPath();
        QDir().mkpath(DirName(path));
#if !defined(_WIN32)
        if (QFile(path).exists())
        {
            if (!Misc::ConvertEndLines(path, true))
                return false;
        }
#endif
        ConfigIni engineConf = ConfigIni(path);
        LODSettings::updateLOD(gameId, engineConf, limit2k);
        LODSettings::updateGFXSettings(gameId, engineConf, false, false);
#if !defined(_WIN32)
        if (!Misc::ConvertEndLines(path, false))
            return false;
#endif
        PINFO("Updating LODs and other settings finished.\n\n");
    }

    if (gameId == MeType::ME3_TYPE)
        TOCBinFile::UpdateAllTOCBinFiles();

    if (g_ipc)
    {
        ConsoleWrite("[IPC]STAGE_CONTEXT STAGE_DONE");
        ConsoleSync();
    }

    long elapsed = Misc::elapsedTime();
    PINFO(Misc::getTimerFormat(elapsed) + "\n");

    PINFO("\nInstallation finished.\n\n");

    return true;
}

bool CmdLineTools::applyMEMSpecialModME3(MeType gameId, QString &memFile,
                                           QString &tfcName, QByteArray &guid,
                                           bool appendTfc, bool verify)
{
    Resources resources;
    MipMaps mipMaps;
    resources.loadMD5Tables();
    ConfigIni configIni = ConfigIni();
    g_GameData->Init(gameId, configIni);
    if (!CheckGamePath())
        return false;

    QList<FoundTexture> textures;

    QString path = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation).first() +
            "/MassEffectModder";
    QString mapFile = path + QString("/me%1map.bin").arg((int)gameId);
    if (!TreeScan::loadTexturesMapFile(mapFile, textures))
    {
        return false;
    }

    QStringList memFiles = QStringList();
    memFiles.push_back(memFile);

    applyMods(memFiles, textures, mipMaps, false, false, tfcName, guid, appendTfc, verify, true, -1);

    return true;
}

bool CmdLineTools::applyMods(QStringList &files, QList<FoundTexture> &textures, MipMaps &mipMaps, bool repack,
                             bool modded, QString &tfcName, QByteArray &guid, bool appendTfc,
                             bool verify, bool special, int cacheAmount)
{
    bool status = true;

    int totalNumberOfMods = 0;
    int currentNumberOfTotalMods = 1;

    for (int i = 0; i < files.count(); i++)
    {
        if (QFile(files[i]).size() == 0)
        {
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR MEM mod file has 0 length: ") + files[i]);
                ConsoleSync();
            }
            else
            {
                PERROR(QString("MEM mod file has 0 length: ") + files[i] + "\n");
            }
            continue;
        }
        FileStream fs = FileStream(files[i], FileMode::Open, FileAccess::ReadOnly);
        if (!Misc::CheckMEMHeader(fs, files[i]))
            continue;
        fs.JumpTo(fs.ReadInt64());
        fs.SkipInt32();
        totalNumberOfMods += fs.ReadInt32();
    }

    for (int i = 0; i < files.count(); i++)
    {
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + files[i]);
            ConsoleSync();
        }
        else
        {
            if (special)
                PINFO(QString("Installing mod: ") + QString::number(i + 1) + " of " +
                             QString::number(files.count()) + " - " + BaseName(files[i]) + "\n");
            else
                PINFO(QString("Preparing mod: ") + QString::number(i + 1) + " of " +
                             QString::number(files.count()) + " - " + BaseName(files[i]) + "\n");
        }

        FileStream fs = FileStream(files[i], FileMode::Open, FileAccess::ReadOnly);
        if (!Misc::CheckMEMHeader(fs, files[i]))
            continue;

        if (!Misc::CheckMEMGameVersion(fs, files[i], GameData::gameType))
            continue;

        int numFiles = fs.ReadInt32();
        QList<FileMod> modFiles{};
        for (int k = 0; k < numFiles; k++)
        {
            FileMod fileMod{};
            fileMod.tag = fs.ReadUInt32();
            fs.ReadStringASCIINull(fileMod.name);
            fileMod.offset = fs.ReadInt64();
            fileMod.size = fs.ReadInt64();
            modFiles.push_back(fileMod);
        }
        numFiles = modFiles.count();
        for (int l = 0; l < numFiles; l++, currentNumberOfTotalMods++)
        {
            QString name;
            uint crc = 0;
            int exportId = -1;
            QString pkgPath;
            fs.JumpTo(modFiles[l].offset);
            long size = modFiles[l].size;
            if (modFiles[l].tag == FileTextureTag || modFiles[l].tag == FileTextureTag2)
            {
                fs.ReadStringASCIINull(name);
                crc = fs.ReadUInt32();
            }
            else if (modFiles[l].tag == FileBinaryTag || modFiles[l].tag == FileXdeltaTag)
            {
                name = modFiles[l].name;
                exportId = fs.ReadInt32();
                fs.ReadStringASCIINull(pkgPath);
                pkgPath = pkgPath.replace('\\', '/');
            }
            else
            {
                if (g_ipc)
                {
                    ConsoleWrite(QString("[IPC]ERROR Unknown tag for file: ") + name);
                    ConsoleSync();
                }
                else
                {
                    PERROR(QString("Unknown tag for file: ") + name + "\n");
                }
                continue;
            }

            if (modFiles[l].tag == FileTextureTag || modFiles[l].tag == FileTextureTag2)
            {
                FoundTexture f = Misc::FoundTextureInTheMap(textures, crc);
                if (f.crc != 0)
                {
                    if (special)
                    {
                        ByteBuffer dst = MipMaps::decompressData(fs, size);
                        if (dst.size() == 0)
                        {
                            if (g_ipc)
                            {
                                ConsoleWrite(QString("[IPC]ERROR Failed decompress data: ") + name);
                                ConsoleSync();
                            }
                            PERROR(QString("Failed decompress data: ") + name + "\n");
                            continue;
                        }
                        Image image = Image(dst, ImageFormat::DDS);
                        dst.Free();
                        replaceTextureSpecialME3Mod(image, f.list, f.name, tfcName, guid, appendTfc, verify);
                    }
                    else
                    {
                        ModEntry entry{};
                        entry.textureCrc = f.crc;
                        entry.textureName = f.name;
                        if (modFiles[l].tag == FileTextureTag2)
                            entry.markConvert = true;
                        entry.memPath = files[i];
                        entry.memEntryOffset = fs.Position();
                        entry.memEntrySize = size;
                        modsToReplace.push_back(entry);
                    }
                }
                else
                {
                    PINFO(QString("Texture skipped. Texture ") + name +
                                 QString().sprintf("_0x%08X", crc) + " is not present in your game setup.\n");
                }
            }
            else if (modFiles[l].tag == FileBinaryTag)
            {
                if (!QFile(g_GameData->GamePath() + pkgPath).exists())
                {
                    PINFO(QString("Warning: File ") + pkgPath +
                                 " not exists in your game setup.\n");
                    continue;
                }
                ModEntry entry{};
                entry.binaryModType = true;
                entry.packagePath = pkgPath;
                entry.exportId = exportId;
                entry.binaryModData = MipMaps::decompressData(fs, size);
                if (entry.binaryModData.size() == 0)
                {
                    if (g_ipc)
                    {
                        ConsoleWrite(QString("[IPC]ERROR Failed decompress data: ") + name);
                        ConsoleSync();
                    }
                    PERROR(QString("Failed decompress data: ") + name + "\n");
                    continue;
                }
                modsToReplace.push_back(entry);
            }
            else if (modFiles[l].tag == FileXdeltaTag)
            {
                QString path = g_GameData->GamePath() + pkgPath;
                if (!QFile(path).exists())
                {
                    PINFO(QString("Warning: File ") + pkgPath +
                                 " not exists in your game setup.\n");
                    continue;
                }
                ModEntry entry{};
                Package pkg;
                if (pkg.Open(path) != 0)
                {
                    PERROR(QString("Failed open package ") + pkgPath + "\n");
                    continue;
                }
                ByteBuffer src = pkg.getExportData(exportId);
                if (src.ptr() == nullptr)
                {
                    PERROR(QString("Failed get data, export id") +
                                 QString::number(exportId) + ", package: " + pkgPath + "\n");
                    continue;
                }
                ByteBuffer dst = MipMaps::decompressData(fs, size);
                if (dst.size() == 0)
                {
                    if (g_ipc)
                    {
                        ConsoleWrite(QString("[IPC]ERROR Failed decompress data: ") + name);
                        ConsoleSync();
                    }
                    PERROR(QString("Failed decompress data: ") + name + "\n");
                    continue;
                }
                auto buffer = ByteBuffer(src.size());
                uint dstLen = 0;
                int status = XDelta3Decompress(src.ptr(), src.size(), dst.ptr(), dst.size(), buffer.ptr(), &dstLen);
                src.Free();
                dst.Free();
                if (status != 0)
                {
                    PERROR(QString("Warning: Xdelta patch for ") + pkgPath + " failed to apply.\n");
                    buffer.Free();
                    continue;
                }
                entry.binaryModType = true;
                entry.packagePath = pkgPath;
                entry.exportId = exportId;
                entry.binaryModData = buffer;
                modsToReplace.push_back(entry);
            }
        }
    }

    if (!special)
        mipMaps.replaceModsFromList(textures, pkgsToMarker, pkgsToRepack, modsToReplace,
                                     repack, !modded, verify, !modded, cacheAmount);

    modsToReplace.clear();

    PINFO("Process textures finished.\n\n");

    return status;
}

void CmdLineTools::replaceTextureSpecialME3Mod(Image &image, QList<MatchedTexture> &list,
                                               QString &textureName, QString &tfcName,
                                               QByteArray &guid, bool appendTfc, bool verify)
{
    Texture *arcTexture = nullptr, *cprTexture = nullptr;

    for (int n = 0; n < list.count(); n++)
    {
        MatchedTexture nodeTexture = list[n];
        if (nodeTexture.path.length() == 0)
            continue;
        Package package;
        if (package.Open(g_GameData->GamePath() + nodeTexture.path) != 0)
        {
            PERROR(QString("Error: Failed open package: ") + nodeTexture.path + "\n");
            continue;
        }
        ByteBuffer exportData = package.getExportData(nodeTexture.exportID);
        if (exportData.ptr() == nullptr)
        {
            PERROR(QString("Error: Texture ") + textureName +
                   " has broken export data in package: " +
                   nodeTexture.path + "\nExport Id: " +
                   QString::number(nodeTexture.exportID + 1) + "\nSkipping...\n");
            continue;
        }

        auto *texture = new Texture(package, nodeTexture.exportID, exportData);
        exportData.Free();
        QString fmt = texture->getProperties().getProperty("Format").valueName;
        PixelFormat pixelFormat = Image::getPixelFormatType(fmt);
        texture->removeEmptyMips();

        if (!Misc::CheckImage(image, *texture, textureName))
            break;

        QString errors = Misc::CorrectTexture(&image, *texture, pixelFormat, pixelFormat,
                                              false, textureName);
        if (errors.length() != 0)
            PERROR(errors);

        // remove lower mipmaps from source image which not exist in game data
        MipMaps::RemoveLowerMips(&image, texture);

        // put empty mips if missing
        MipMaps::AddMissingLowerMips(&image, texture);

        bool triggerCacheArc = false, triggerCacheCpr = false;
        QString archiveFile;
        quint8 origGuid[16] = {};
        if (texture->getProperties().exists("TextureFileCacheName"))
        {
            ByteBuffer newGuid(reinterpret_cast<quint8 *>(guid.data()), 16);
            memcpy(origGuid, texture->getProperties().getProperty("TFCFileGuid").valueStruct.ptr(), 16);
            archiveFile = DirName(g_GameData->GamePath() + nodeTexture.path) + "/" + tfcName + ".tfc";
            texture->getProperties().setNameValue("TextureFileCacheName", tfcName);
            texture->getProperties().setStructValue("TFCFileGuid", "Guid", newGuid);
            if (!QFile(archiveFile).exists())
            {
                FileStream fs = FileStream(archiveFile, FileMode::Create, FileAccess::WriteOnly);
                fs.WriteFromBuffer(newGuid);
            }
            newGuid.Free();
        }

        if (verify)
            nodeTexture.crcs.clear();
        auto mipmaps = QList<Texture::TextureMipMap>();
        for (int m = 0; m < image.getMipMaps().count(); m++)
        {
            if (verify)
                nodeTexture.crcs.push_back(texture->getCrcData(image.getMipMaps()[m]->getRefData()));
            Texture::TextureMipMap mipmap;
            mipmap.width = image.getMipMaps()[m]->getOrigWidth();
            mipmap.height = image.getMipMaps()[m]->getOrigHeight();
            if (texture->existMipmap(mipmap.width, mipmap.height))
                mipmap.storageType = texture->getMipmap(mipmap.width, mipmap.height).storageType;
            else
            {
                mipmap.storageType = texture->getTopMipmap().storageType;
                if (texture->mipMapsList.count() > 1)
                {
                    if (texture->getProperties().exists("TextureFileCacheName"))
                    {
                        if (texture->mipMapsList.count() <= 6)
                        {
                            mipmap.storageType = Texture::StorageTypes::pccUnc;
                            texture->getProperties().setBoolValue("NeverStream", true);
                        }
                        else
                        {
                            if (GameData::gameType == MeType::ME2_TYPE)
                                mipmap.storageType = Texture::StorageTypes::extLZO;
                            else
                                mipmap.storageType = Texture::StorageTypes::extZlib;
                        }
                    }
                }
            }

            if (mipmap.storageType == Texture::StorageTypes::extLZO)
                mipmap.storageType = Texture::StorageTypes::extZlib;
            if (mipmap.storageType == Texture::StorageTypes::pccLZO)
                mipmap.storageType = Texture::StorageTypes::pccZlib;

            if (arcTexture != nullptr && mipmap.storageType != arcTexture->mipMapsList[m].storageType)
            {
                delete arcTexture;
                arcTexture = nullptr;
            }

            mipmap.uncompressedSize = image.getMipMaps()[m]->getRefData().size();
            if (mipmap.storageType == Texture::StorageTypes::extZlib ||
                mipmap.storageType == Texture::StorageTypes::extLZO)
            {
                if (cprTexture == nullptr || (cprTexture != nullptr && mipmap.storageType != cprTexture->mipMapsList[m].storageType))
                {
                    mipmap.newData = texture->compressTexture(image.getMipMaps()[m]->getRefData(), mipmap.storageType);
                    mipmap.freeNewData = true;
                    triggerCacheCpr = true;
                }
                else
                {
                    if ((mipmap.width >= 4 && cprTexture->mipMapsList[m].width != mipmap.width) ||
                        (mipmap.height >= 4 && cprTexture->mipMapsList[m].height != mipmap.height))
                    {
                        CRASH();
                    }
                    mipmap.newData = ByteBuffer(cprTexture->mipMapsList[m].newData.ptr(),
                                                cprTexture->mipMapsList[m].newData.size());
                    mipmap.freeNewData = true;
                }
                mipmap.compressedSize = mipmap.newData.size();
            }
            if (mipmap.storageType == Texture::StorageTypes::pccUnc ||
                mipmap.storageType == Texture::StorageTypes::extUnc)
            {
                mipmap.compressedSize = mipmap.uncompressedSize;
                mipmap.newData = ByteBuffer(image.getMipMaps()[m]->getRefData().ptr(),
                                            image.getMipMaps()[m]->getRefData().size());
                mipmap.freeNewData = true;
            }
            if (mipmap.storageType == Texture::StorageTypes::extZlib ||
                mipmap.storageType == Texture::StorageTypes::extLZO ||
                mipmap.storageType == Texture::StorageTypes::extUnc)
            {
                if (arcTexture == nullptr ||
                    memcmp(arcTexture->getProperties().getProperty("TFCFileGuid").valueStruct.ptr(),
                           texture->getProperties().getProperty("TFCFileGuid").valueStruct.ptr(), 16) != 0)
                {
                    triggerCacheArc = true;
                    Texture::TextureMipMap oldMipmap{};
                    auto mipMapExists = texture->existMipmap(mipmap.width, mipmap.height);
                    if (mipMapExists)
                        oldMipmap = texture->getMipmap(mipmap.width, mipmap.height);
                    if (!appendTfc &&
                        memcmp(origGuid, texture->getProperties().getProperty("TFCFileGuid").valueStruct.ptr(), 16) != 0 &&
                        mipMapExists && mipmap.newData.size() <= oldMipmap.compressedSize)
                    {
                        FileStream fs = FileStream(archiveFile, FileMode::Open, FileAccess::ReadWrite);
                        fs.JumpTo(oldMipmap.dataOffset);
                        mipmap.dataOffset = oldMipmap.dataOffset;
                        fs.WriteFromBuffer(mipmap.newData);
                    }
                    else
                    {
                        FileStream fs = FileStream(archiveFile, FileMode::Open, FileAccess::ReadWrite);
                        fs.SeekEnd();
                        mipmap.dataOffset = (uint)fs.Position();
                        fs.WriteFromBuffer(mipmap.newData);
                    }
                }
                else
                {
                    if ((mipmap.width >= 4 && arcTexture->mipMapsList[m].width != mipmap.width) ||
                        (mipmap.height >= 4 && arcTexture->mipMapsList[m].height != mipmap.height))
                    {
                        CRASH();
                    }
                    mipmap.dataOffset = arcTexture->mipMapsList[m].dataOffset;
                }
            }

            mipmap.width = image.getMipMaps()[m]->getWidth();
            mipmap.height = image.getMipMaps()[m]->getHeight();
            mipmaps.push_back(mipmap);
            if (texture->mipMapsList.count() == 1)
                break;
        }
        texture->replaceMipMaps(mipmaps);
        texture->getProperties().setIntValue("SizeX", texture->mipMapsList.first().width);
        texture->getProperties().setIntValue("SizeY", texture->mipMapsList.first().height);
        if (texture->getProperties().exists("MipTailBaseIdx"))
            texture->getProperties().setIntValue("MipTailBaseIdx", texture->mipMapsList.count() - 1);

        {
            MemoryStream newData;
            ByteBuffer buffer = texture->getProperties().toArray();
            newData.WriteFromBuffer(buffer);
            buffer.Free();
            buffer = texture->toArray(0, false); // filled later
            newData.WriteFromBuffer(buffer);
            buffer.Free();
            buffer = newData.ToArray();
            package.setExportData(nodeTexture.exportID, buffer);
            buffer.Free();
        }

        uint packageDataOffset;
        {
            MemoryStream newData;
            ByteBuffer buffer = texture->getProperties().toArray();
            newData.WriteFromBuffer(buffer);
            buffer.Free();
            packageDataOffset = package.exportsTable[nodeTexture.exportID].getDataOffset() + (uint)newData.Position();
            buffer = texture->toArray(packageDataOffset);
            newData.WriteFromBuffer(buffer);
            buffer.Free();
            buffer = newData.ToArray();
            package.setExportData(nodeTexture.exportID, buffer);
            buffer.Free();
        }

        if (triggerCacheCpr)
        {
            if (cprTexture != arcTexture || triggerCacheArc)
                delete cprTexture;
        }
        if (triggerCacheArc)
        {
            if (cprTexture != arcTexture && !triggerCacheCpr)
                delete arcTexture;
        }
        if (triggerCacheCpr)
            cprTexture = texture;
        if (triggerCacheArc)
            arcTexture = texture;

        if (!triggerCacheCpr && !triggerCacheArc)
            delete texture;

        package.SaveToFile(false, false, false);
    }
    if (cprTexture != arcTexture)
        delete cprTexture;
    delete arcTexture;
}

bool CmdLineTools::extractAllTextures(MeType gameId, QString &outputDir, bool png,
                                      bool pccOnly, bool tfcOnly, QString &textureTfcFilter)
{
    Resources resources;
    resources.loadMD5Tables();
    ConfigIni configIni = ConfigIni();
    g_GameData->Init(gameId, configIni);
    if (!CheckGamePath())
        return false;

    QDir().mkpath(outputDir);

    for (int i = 0; i < g_GameData->packageFiles.count(); i++)
    {
        PINFO(QString("Package ") + QString::number(i + 1) + "/" +
                             QString::number(g_GameData->packageFiles.count()) + " : " +
                             g_GameData->packageFiles[i] + "\n");

        Package package;
        if (package.Open(g_GameData->GamePath() + g_GameData->packageFiles[i]) != 0)
        {
            PERROR(QString("ERROR: Issue opening package file: ") + g_GameData->packageFiles[i] + "\n");
            continue;
        }

        for (int i = 0; i < package.exportsTable.count(); i++)
        {
            Package::ExportEntry& exp = package.exportsTable[i];
            int id = package.getClassNameId(exp.getClassId());
            if (id == package.nameIdTexture2D ||
                id == package.nameIdLightMapTexture2D ||
                id == package.nameIdShadowMapTexture2D ||
                id == package.nameIdTextureFlipBook)
            {
                ByteBuffer exportData = package.getExportData(i);
                if (exportData.ptr() == nullptr)
                {
                    PERROR(QString("Error: Texture ") + exp.objectName +
                                 " has broken export data in package: " +
                                 g_GameData->packageFiles[i] +"\nExport Id: " + QString::number(i + 1) + "\nSkipping...\n");
                    continue;
                }
                Texture texture(package, i, exportData);
                exportData.Free();
                if (!texture.hasImageData())
                {
                    continue;
                }

                bool tfcPropExists = texture.getProperties().exists("TextureFileCacheName");
                if ((pccOnly && tfcPropExists) ||
                    (tfcOnly && !tfcPropExists) ||
                    (tfcOnly && !texture.HasExternalMips()))
                {
                    continue;
                }
                if (!pccOnly && !tfcOnly && textureTfcFilter.length() != 0)
                {
                    if (!tfcPropExists)
                        continue;
                    QString archive = texture.getProperties().getProperty("TextureFileCacheName").valueName;
                    if (archive != textureTfcFilter ||
                        !texture.HasExternalMips())
                    {
                        continue;
                    }
                }
                QString name = exp.objectName;
                uint crc = texture.getCrcTopMipmap();
                if (crc == 0)
                {
                    PERROR(QString("Error: Texture ") + name + " is broken in package: " +
                                 g_GameData->packageFiles[i] +"\nExport Id: " + QString::number(i + 1) + "\nSkipping...\n");
                    continue;
                }
                QString outputFile = outputDir + "/" +  name +
                        QString().sprintf("_0x%08X", crc);
                if (png)
                {
                    outputFile += ".png";
                }
                else
                {
                    outputFile += ".dds";
                }
                if (QFile(outputFile).exists())
                    continue;
                PixelFormat pixelFormat = Image::getPixelFormatType(texture.getProperties().getProperty("Format").valueName);
                if (png)
                {
                    Texture::TextureMipMap mipmap = texture.getTopMipmap();
                    ByteBuffer data = texture.getTopImageData();
                    if (data.ptr() != nullptr)
                    {
                        if (QFile(outputFile).exists())
                            QFile(outputFile).remove();
                        Image::saveToPng(data.ptr(), mipmap.width, mipmap.height, pixelFormat, outputFile);
                        data.Free();
                    }
                }
                else
                {
                    texture.removeEmptyMips();
                    QList<MipMap *> mipmaps = QList<MipMap *>();
                    for (int k = 0; k < texture.mipMapsList.count(); k++)
                    {
                        ByteBuffer data = texture.getMipMapDataByIndex(k);
                        if (data.ptr() == nullptr)
                        {
                            continue;
                        }
                        mipmaps.push_back(new MipMap(data, texture.mipMapsList[k].width, texture.mipMapsList[k].height, pixelFormat));
                        data.Free();
                    }
                    Image image = Image(mipmaps, pixelFormat);
                    if (image.getMipMaps().count() != 0)
                    {
                        if (QFile(outputFile).exists())
                            QFile(outputFile).remove();
                        FileStream fs = FileStream(outputFile, FileMode::Create, FileAccess::WriteOnly);
                        image.StoreImageToDDS(fs);
                    }
                    else
                    {
                        PERROR(QString("Texture skipped. Texture ") + name +
                                     QString().sprintf("_0x%08X", crc) + " is broken in game data!\n");
                    }
                }
            }
        }
    }

    PINFO("Extracting textures completed.\n\n");
    return true;
}

bool CmdLineTools::CheckTextures(MeType gameId)
{
    ConfigIni configIni = ConfigIni();
    g_GameData->Init(gameId, configIni);
    if (!CheckGamePath())
        return false;

    PINFO("Starting checking textures...\n");

    int lastProgress = -1;
    for (int i = 0; i < g_GameData->packageFiles.count(); i++)
    {
        Package package;
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + g_GameData->packageFiles[i]);
        }
        else
        {
            PINFO(QString("Package ") + QString::number(i + 1) + " of " +
                         QString::number(g_GameData->packageFiles.count()) + " - " +
                         g_GameData->packageFiles[i] + "\n");
        }
        int newProgress = (i + 1) * 100 / g_GameData->packageFiles.count();
        if (g_ipc && lastProgress != newProgress)
        {
            ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
            ConsoleSync();
            lastProgress = newProgress;
        }
        if (package.Open(g_GameData->GamePath() + g_GameData->packageFiles[i]) != 0)
        {
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR_TEXTURE_SCAN_DIAGNOSTIC Error opening package file: ") +
                             g_GameData->packageFiles[i]);
                ConsoleSync();
            }
            else
            {
                QString err = "";
                err += "---- Start --------------------------------------------\n\n" ;
                err += "Error opening package file: " + g_GameData->packageFiles[i] + "\n\n";
                err += "---- End ----------------------------------------------\n\n";
                PERROR(err);
            }
            continue;
        }

        for (int e = 0; e < package.exportsTable.count(); e++)
        {
            int id = package.getClassNameId(package.exportsTable[e].getClassId());
            if (id == package.nameIdTexture2D ||
                id == package.nameIdLightMapTexture2D ||
                id == package.nameIdShadowMapTexture2D ||
                id == package.nameIdTextureFlipBook)
            {
                ByteBuffer exportData = package.getExportData(e);
                Texture texture(package, e, exportData);
                exportData.Free();
                if (texture.hasEmptyMips())
                {
                    if (g_ipc)
                    {
                        ConsoleWrite(QString("[IPC]ERROR_MIPMAPS_NOT_REMOVED Empty mipmap not removed in texture: ") +
                                package.exportsTable[e].objectName + ", package: " +
                                g_GameData->packageFiles[i] + ", export id: " + QString::number(e + 1));
                        ConsoleSync();
                    }
                    else
                    {
                        PERROR(QString("ERROR: Empty mipmap not removed in texture: ") +
                               package.exportsTable[e].objectName + "\nPackage: " +
                               g_GameData->packageFiles[i] + "\nExport Id: " + QString::number(e + 1) + "\n");
                    }
                    continue;
                }

                for (int m = 0; m < texture.mipMapsList.count(); m++)
                {
                    ByteBuffer data = texture.getMipMapDataByIndex(m);
                    if (data.ptr() == nullptr)
                    {
                        if (g_ipc)
                        {
                            ConsoleWrite(QString("[IPC]ERROR_TEXTURE_SCAN_DIAGNOSTIC Issue opening texture data: ") +
                                        package.exportsTable[e].objectName + ", mipmap: " + QString::number(m) + ", package: " +
                                        g_GameData->packageFiles[i] + ", export id: " + QString::number(e + 1));
                            ConsoleSync();
                        }
                        else
                        {
                            PERROR(QString("Error: Issue opening texture data: ") +
                                   package.exportsTable[e].objectName + "\nMipmap: " + QString::number(m) + "\nPackage: " +
                                   g_GameData->packageFiles[i] + "\nExport Id: " + QString::number(e + 1) + "\n");
                        }
                    }
                    data.Free();
                }
            }
        }
    }
    PINFO("Finished checking textures.\n\n");

    return true;
}

bool CmdLineTools::FixMissingPropertyInTextures(MeType gameId, const QString& filter)
{
    ConfigIni configIni = ConfigIni();
    g_GameData->Init(gameId, configIni);
    if (!CheckGamePath())
        return false;

    PINFO("Starting checking textures...\n");

    QStringList packages;
    for (int i = 0; i < g_GameData->packageFiles.count(); i++)
    {
        if (filter != "")
        {
            if (!g_GameData->packageFiles[i].contains(filter, Qt::CaseSensitive))
            {
                continue;
            }
        }
        packages += g_GameData->packageFiles[i];
    }

    int lastProgress = -1;
    for (int i = 0; i < packages.count(); i++)
    {
        Package package;
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]PROCESSING_FILE ") + packages[i]);
        }
        else
        {
            PINFO(QString("Package ") + QString::number(i + 1) + " of " +
                         QString::number(packages.count()) + " - " +
                         packages[i] + "\n");
        }
        int newProgress = (i + 1) * 100 / packages.count();
        if (g_ipc && lastProgress != newProgress)
        {
            ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
            ConsoleSync();
            lastProgress = newProgress;
        }
        if (package.Open(g_GameData->GamePath() + packages[i]) != 0)
        {
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR Error opening package file: ") +
                             packages[i]);
                ConsoleSync();
            }
            else
            {
                QString err = "";
                err += "---- Start --------------------------------------------\n\n" ;
                err += "Error opening package file: " + packages[i] + "\n\n";
                err += "---- End ----------------------------------------------\n\n";
                PERROR(err);
            }
            continue;
        }

        bool modified = false;
        for (int e = 0; e < package.exportsTable.count(); e++)
        {
            int id = package.getClassNameId(package.exportsTable[e].getClassId());
            if (id == package.nameIdTexture2D ||
                id == package.nameIdLightMapTexture2D ||
                id == package.nameIdShadowMapTexture2D ||
                id == package.nameIdTextureFlipBook)
            {
                ByteBuffer exportData = package.getExportData(e);
                Texture texture(package, e, exportData);
                exportData.Free();
                if (GameData::gameType != MeType::ME1_TYPE &&
                    texture.numNotEmptyMips() > 6 &&
                    !texture.HasExternalMips() &&
                    !texture.getProperties().exists("NeverStream"))
                {
                    PINFO(QString("Adding missing property \"NeverStream\" for ") +
                                  package.exportsTable[e].objectName + ", export id: " +
                                  QString::number(e + 1) + "\n");
                    texture.getProperties().setBoolValue("NeverStream", true);
                    modified = true;
                }
                else
                {
                    continue;
                }

                {
                    MemoryStream newData;
                    ByteBuffer buffer = texture.getProperties().toArray();
                    newData.WriteFromBuffer(buffer);
                    buffer.Free();
                    buffer = texture.toArray(0, false); // filled later
                    newData.WriteFromBuffer(buffer);
                    buffer.Free();
                    buffer = newData.ToArray();
                    package.setExportData(e, buffer);
                    buffer.Free();
                }

                uint packageDataOffset;
                {
                    MemoryStream newData;
                    ByteBuffer buffer = texture.getProperties().toArray();
                    newData.WriteFromBuffer(buffer);
                    buffer.Free();
                    packageDataOffset = package.exportsTable[e].getDataOffset() + (uint)newData.Position();
                    buffer = texture.toArray(packageDataOffset);
                    newData.WriteFromBuffer(buffer);
                    buffer.Free();
                    buffer = newData.ToArray();
                    package.setExportData(e, buffer);
                    buffer.Free();
                }
            }
        }
        if (modified)
            package.SaveToFile(false, false, false);
    }

    if (GameData::gameType == MeType::ME3_TYPE)
        TOCBinFile::UpdateAllTOCBinFiles();

    PINFO("Finished checking textures.\n\n");

    return true;
}

bool CmdLineTools::checkGameFilesAfter(MeType gameType)
{
    ConfigIni configIni = ConfigIni();
    g_GameData->Init(gameType, configIni);
    if (!CheckGamePath())
        return false;

    PINFO("Checking for vanilla files after textures installation...\n");
    QString path;
    if (GameData::gameType == MeType::ME1_TYPE)
    {
        path = "/BioGame/CookedPC/testVolumeLight_VFX.upk";
    }
    if (GameData::gameType == MeType::ME2_TYPE)
    {
        path = "/BioGame/CookedPC/BIOC_Materials.pcc";
    }
    QStringList filesToUpdate = QStringList();
    for (int i = 0; i < g_GameData->packageFiles.count(); i++)
    {
        if (path.length() != 0 && g_GameData->packageFiles[i].compare(path, Qt::CaseInsensitive) == 0)
            continue;
        filesToUpdate.push_back(g_GameData->packageFiles[i]);
    }
    int lastProgress = -1;
    for (int i = 0; i < filesToUpdate.count(); i++)
    {
        int newProgress = (i + 1) * 100 / filesToUpdate.count();
        if (g_ipc && lastProgress != newProgress)
        {
            ConsoleWrite(QString("[IPC]TASK_PROGRESS ") + QString::number(newProgress));
            ConsoleSync();
            lastProgress = newProgress;
        }
        FileStream fs = FileStream(g_GameData->GamePath() + filesToUpdate[i], FileMode::Open, FileAccess::ReadOnly);
        fs.Seek(-MEMMarkerLenght, SeekOrigin::End);
        QString marker;
        fs.ReadStringASCII(marker, MEMMarkerLenght);
        if (marker != QString(MEMendFileMarker))
        {
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR_VANILLA_MOD_FILE ") + filesToUpdate[i]);
                ConsoleSync();
            }
            else
            {
                PERROR(QString("Vanilla file: ") + filesToUpdate[i] + "\n");
            }
        }
    }

    PINFO("Finished checking for vanilla files after textures installation.\n\n");

    return true;
}

bool CmdLineTools::detectsMismatchPackagesAfter(MeType gameType)
{
    ConfigIni configIni = ConfigIni();
    g_GameData->Init(gameType, configIni);
    if (!CheckGamePath())
        return false;

    QString path = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation).first() +
            "/MassEffectModder";
    QString mapFile = path + QString("/me%1map.bin").arg((int)gameType);
    FileStream fs = FileStream(mapFile, FileMode::Open, FileAccess::ReadOnly);
    uint tag = fs.ReadUInt32();
    uint version = fs.ReadUInt32();
    if (tag != textureMapBinTag || version != textureMapBinVersion)
    {
        if (g_ipc)
        {
            ConsoleWrite("[IPC]ERROR_TEXTURE_MAP_WRONG");
            ConsoleSync();
        }
        else
        {
            PERROR("Detected wrong or old version of textures scan file!\n");
        }
        return false;
    }

    uint countTexture = fs.ReadUInt32();
    for (uint i = 0; i < countTexture; i++)
    {
        fs.Skip(fs.ReadInt32());
        fs.SkipInt32();
        uint countPackages = fs.ReadUInt32();
        for (uint k = 0; k < countPackages; k++)
        {
            fs.Skip(8);
            fs.Skip(fs.ReadInt32());
        }
    }

    QStringList packages = QStringList();
    int numPackages = fs.ReadInt32();
    for (int i = 0; i < numPackages; i++)
    {
        QString pkgPath;
        fs.ReadStringASCII(pkgPath, fs.ReadInt32());
        pkgPath.replace(QChar('\\'), QChar('/'));
        packages.push_back(pkgPath);
    }
    PINFO("Checking for removed files since last game data scan...\n");
    for (int i = 0; i < packages.count(); i++)
    {
        if (!g_GameData->packageFiles.contains(packages[i], Qt::CaseInsensitive))
        {
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR_REMOVED_FILE ") + packages[i]);
                ConsoleSync();
            }
            else
            {
                PERROR(QString("Removed: ") + packages[i] + "\n");
            }
        }
    }
    PINFO("Finished checking for removed files since last game data scan.\n\n");

    PINFO("Checking for additional files since last game data scan...\n");
    for (int i = 0; i < g_GameData->packageFiles.count(); i++)
    {
        if (!packages.contains(g_GameData->packageFiles[i], Qt::CaseInsensitive))
        {
            if (g_ipc)
            {
                ConsoleWrite(QString("[IPC]ERROR_ADDED_FILE ") + g_GameData->packageFiles[i]);
                ConsoleSync();
            }
            else
            {
                PERROR(QString("Added: ") + g_GameData->packageFiles[i] + "\n");
            }
        }
    }
    PINFO("Finished checking for additional files since last game data scan.\n\n");

    return true;
}
