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

#include <CmdLine/CmdLineTools.h>
#include <GameData/GameData.h>
#include <GameData/LODSettings.h>
#include <GameData/TOCFile.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <Misc/Misc.h>
#include <MipMaps/MipMaps.h>
#include <Program/ConfigIni.h>
#include <Resources/Resources.h>
#include <Wrappers.h>
#include <Texture/Texture.h>
#include <Texture/TextureMovie.h>
#include <Texture/TextureScan.h>
#include <Types/MemTypes.h>

int CmdLineTools::scan(MeType gameId)
{
    int errorCode = 0;

    auto configIni = ConfigIni{};
    g_GameData->Init(gameId, configIni);
    if (!Misc::CheckGamePath())
        return 1;

    Misc::startTimer();
    PINFO("Scan started...\n");

    int totalPackages = g_GameData->packageFiles.count();
    for (int i = 0; i < totalPackages; i++)
    {
        PINFO(QString("Package ") + QString::number(i + 1) + "/" +
                             QString::number(totalPackages) + " : " +
                             g_GameData->packageFiles[i] + "\n");
        Package package;
        int status = package.Open(g_GameData->GamePath() + g_GameData->packageFiles[i]);
        if (status != 0)
        {
            PERROR(QString("ERROR: Issue opening package file: ") + g_GameData->packageFiles[i] + "\n");
            return 1;
        }

        for (int e = 0; e < package.exportsTable.count(); e++)
        {
            ByteBuffer exportData = package.getExportData(e);
            if (exportData.ptr() == nullptr)
            {
                PERROR(QString("Error: broken export data in package: " +
                                 g_GameData->packageFiles[i] +"\nExport Id: " + QString::number(e + 1) + "\nSkipping...\n"));
                continue;
            }
            auto properties = new Properties(package, exportData);
            exportData.Free();
            auto serialize = properties->toArray();
            delete properties;
            serialize.Free();
        }
    }

    long elapsed = Misc::elapsedTime();
    PINFO(Misc::getTimerFormat(elapsed) + "\n");

    PINFO("Scan finished.\n\n");

    return errorCode;
}

int CmdLineTools::scanTextures(MeType gameId)
{
    int errorCode;

    auto configIni = ConfigIni{};
    g_GameData->Init(gameId, configIni);
    if (!Misc::CheckGamePath())
        return 1;

    PINFO("Scan textures started...\n");

    QList<TextureMapEntry> textures;
    Resources resources;

    resources.loadMD5Tables();
    Misc::startTimer();
    errorCode = TreeScan::PrepareListOfTextures(gameId, resources, textures, true, nullptr, nullptr);
    long elapsed = Misc::elapsedTime();
    PINFO(Misc::getTimerFormat(elapsed) + "\n");

    PINFO("Scan textures finished.\n\n");

    return errorCode;
}

bool CmdLineTools::updateTOCs(MeType gameId)
{
    ConfigIni configIni = ConfigIni();
    g_GameData->Init(gameId, configIni);
    if (!Misc::CheckGamePath())
        return false;

    TOCBinFile::UpdateAllTOCBinFiles(gameId);

    return true;
}

bool CmdLineTools::GetGamePaths()
{
    ConfigIni configIni = ConfigIni();
    g_GameData->Init(MeType::ME1_TYPE, configIni);

    QString pathData = g_GameData->GamePath();

    if (g_ipc)
    {
        ConsoleWrite(QString("[IPC]GAMEPATH ") + pathData);
        ConsoleSync();
    }
    else
    {
        ConsoleWrite(QString("Game data path - MELE: ") + pathData);
    }

    return true;
}

bool CmdLineTools::unpackArchive(const QString &inputFile, QString &outputDir,
                                 QString &filterWithExt, bool flattenPath)
{
    outputDir = QDir::cleanPath(outputDir);
    if (outputDir != "")
        QDir().mkpath(outputDir);
#if defined(_WIN32)
    auto strFile = inputFile.toStdWString();
    auto strOut = outputDir.toStdWString();
    auto strFilter = filterWithExt.toStdWString();
    auto fileName = strFile.c_str();
    auto outPath = strOut.c_str();
    auto filter = strFilter.c_str();
#else
    auto strFile = inputFile.toUtf8();
    auto strOut = outputDir.toUtf8();
    auto strFilter = filterWithExt.toUtf8();
    auto fileName = strFile.constData();
    auto outPath = strOut.constData();
    auto filter = strFilter.constData();
#endif

    if (outputDir.isEmpty())
        PINFO(QString("Unpacking archive: " + inputFile + "\n\n"));
    else
        PINFO(QString("Unpacking archive: " + inputFile + " to folder: " + outputDir + "\n\n"));

    int result = 1;
    if (inputFile.endsWith(".zip", Qt::CaseInsensitive))
        result = ZipUnpack(fileName, outPath, filter, !flattenPath, g_ipc);
    else if (inputFile.endsWith(".7z", Qt::CaseInsensitive))
        result = SevenZipUnpack(fileName, outPath, filter, !flattenPath, g_ipc);
    else if (inputFile.endsWith(".rar", Qt::CaseInsensitive))
        result = RarUnpack(fileName, outPath, filter, !flattenPath, g_ipc);
    else
    {
        PINFO("Archive type is not supported.\n\n");
        return false;
    }

    if (result == 0)
        PINFO("\nUnpacking completed successfully.\n\n");
    else
        PINFO("\nUnpacking failed.\n\n");

    return result == 0;
}

bool CmdLineTools::listArchive(const QString &inputFile)
{
#if defined(_WIN32)
    auto strFile = inputFile.toStdWString();
    auto fileName = strFile.c_str();
#else
    auto strFile = inputFile.toUtf8();
    auto fileName = strFile.constData();
#endif

    PINFO(QString("Listing archive: " + inputFile + "\n\n"));

    int result = 1;
    if (inputFile.endsWith(".zip", Qt::CaseInsensitive))
        result = ZipList(fileName, g_ipc);
    else if (inputFile.endsWith(".7z", Qt::CaseInsensitive))
        result = SevenZipList(fileName, g_ipc);
    else if (inputFile.endsWith(".rar", Qt::CaseInsensitive))
        result = RarList(fileName, g_ipc);
    else
    {
        PINFO("Archive type is not supported.\n\n");
        return false;
    }

    if (result == 0)
        PINFO("\nListing completed successfully.\n\n");
    else
        PINFO("\nListing failed.\n\n");

    return result == 0;
}

bool CmdLineTools::ConvertToMEM(MeType gameId, QString &inputDir, QString &memFile, bool markToConvert, bool bc7format)
{
    QList<TextureMapEntry> textures;
    Resources resources;
    resources.loadMD5Tables();
    TreeScan::loadTexturesMap(gameId, resources, textures);

    QFileInfoList list;
    QFileInfoList list2;
    list = QDir(inputDir, "*.mem", QDir::SortFlag::IgnoreCase | QDir::SortFlag::Name, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    list2 += QDir(inputDir, "*.dds", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    list2 += QDir(inputDir, "*.png", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    list2 += QDir(inputDir, "*.bmp", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    list2 += QDir(inputDir, "*.tga", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    list2 += QDir(inputDir, "*.bik", QDir::SortFlag::Unsorted, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    std::sort(list2.begin(), list2.end(), Misc::compareFileInfoPath);
    list.append(list2);

    return Misc::convertDataModtoMem(list, memFile, gameId, textures, markToConvert, bc7format, nullptr, nullptr);
}

bool CmdLineTools::convertGameTexture(const QString &inputFile,
                                      QString &outputFile, QList<TextureMapEntry> &textures,
                                      bool markToConvert)
{
    uint crc = Misc::scanFilenameForCRC(inputFile);
    if (crc == 0)
        return false;

    TextureMapEntry foundTex = Misc::FoundTextureInTheMap(textures, crc);
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
    {
        newPixelFormat = Misc::changeTextureType(foundTex.pixfmt, image.getPixelFormat(), foundTex.type);
        if (foundTex.pixfmt == newPixelFormat)
            PINFO(QString("Warning for texture: ") + foundTex.name +
                  " This texture can not be converted to desired format...\n");
    }

    bool dxt1HasAlpha = false;
    quint8 dxt1Threshold = 128;
    if (foundTex.type == TextureType::OneBitAlpha)
    {
        dxt1HasAlpha = true;
        if (image.getPixelFormat() == PixelFormat::ARGB ||
            image.getPixelFormat() == PixelFormat::DXT3 ||
            image.getPixelFormat() == PixelFormat::DXT5 ||
            image.getPixelFormat() == PixelFormat::BC7)
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
    QList<TextureMapEntry> textures;
    Resources resources;
    resources.loadMD5Tables();

    TreeScan::loadTexturesMap(gameId, resources, textures);
    return convertGameTexture(inputFile, outputFile, textures, markToConvert);
}

bool CmdLineTools::convertGameImages(MeType gameId, QString &inputDir, QString &outputDir, bool markToConvert)
{
    QList<TextureMapEntry> textures;
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
        if (!convertGameTexture(file.absoluteFilePath(), outputFile, textures, markToConvert))
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
    else if (format == "rgba")
        pixFmt = PixelFormat::RGBA;
    else if (format == "g8")
        pixFmt = PixelFormat::G8;
    else if (format == "bc5")
        pixFmt = PixelFormat::BC5;
    else if (format == "bc7")
        pixFmt = PixelFormat::BC7;
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

bool CmdLineTools::extractMEM(MeType gameId, QString &inputDir, QString &outputDir)
{
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

    return Misc::extractMEM(gameId, list, outputDir, nullptr, nullptr);
}

bool CmdLineTools::ApplyLODAndGfxSettings(MeType gameId)
{
    ConfigIni configIni{};
    g_GameData->Init(gameId, configIni);
    QString path = g_GameData->EngineConfigIniPath(gameId);
#if defined(_WIN32)
    ConfigIni engineConf = ConfigIni(path);
#else
    ConfigIni engineConf = ConfigIni(path, true);
#endif
    LODSettings::updateGFXSettings(gameId, engineConf);

    return true;
}

bool CmdLineTools::PrintLODSettings(MeType gameId)
{
    ConfigIni configIni{};
    g_GameData->Init(gameId, configIni);
    QString path = g_GameData->EngineConfigIniPath(gameId);
    bool exist = QFile(path).exists();
    if (!exist)
        return true;
#if defined(_WIN32)
    ConfigIni engineConf = ConfigIni(path);
#else
    ConfigIni engineConf = ConfigIni(path, true);
#endif
    if (g_ipc)
    {
        LODSettings::readLODIpc(gameId, engineConf);
    }

    return true;
}

bool CmdLineTools::CheckGameDataAndMods(MeType gameId)
{
    ConfigIni configIni{};
    g_GameData->Init(gameId, configIni);
    if (!Misc::CheckGamePath())
        return false;

    Resources resources;
    resources.loadMD5Tables();

    return Misc::CheckGameDataAndMods(gameId, resources);
}

bool CmdLineTools::CheckForMarkers(MeType gameId)
{
    ConfigIni configIni{};
    g_GameData->Init(gameId, configIni);
    if (!Misc::CheckGamePath())
        return false;

    return Misc::CheckForMarkers(nullptr, nullptr);
}

bool CmdLineTools::DetectBadMods(MeType gameId)
{
    ConfigIni configIni{};
    g_GameData->Init(gameId, configIni);
    if (!Misc::CheckGamePath())
        return false;

    return Misc::ReportBadMods();
}

bool CmdLineTools::DetectMods(MeType gameId)
{
    ConfigIni configIni{};
    g_GameData->Init(gameId, configIni);
    if (!Misc::CheckGamePath())
        return false;

    return Misc::ReportMods();
}

bool CmdLineTools::InstallMods(MeType gameId, QString &inputDir,
                               bool alotMode, bool skipMarkers,
                               bool verify, int cacheAmount)
{
    Resources resources;
    resources.loadMD5Tables();
    ConfigIni configIni = ConfigIni();
    g_GameData->Init(gameId, configIni);
    if (!Misc::CheckGamePath())
        return false;

    auto files = QDir(inputDir, "*.mem",
                      QDir::SortFlag::Name | QDir::SortFlag::IgnoreCase,
                      QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks).entryInfoList();
    QStringList modFiles;
    foreach (QFileInfo file, files)
    {
        modFiles.push_back(file.absoluteFilePath());
    }

    return Misc::InstallMods(gameId, resources, modFiles,
                             false, alotMode, skipMarkers, verify, cacheAmount,
                             nullptr, nullptr);
}

bool CmdLineTools::extractAllTextures(MeType gameId, QString &outputDir, QString &inputFile,
                                      bool png, bool pccOnly, bool tfcOnly, bool mapCrc,
                                      QString &textureTfcFilter, bool clearAlpha)
{
    Resources resources;
    resources.loadMD5Tables();
    ConfigIni configIni = ConfigIni();
    g_GameData->Init(gameId, configIni);
    if (!Misc::CheckGamePath())
        return false;

    QList<TextureMapEntry> textures;
    if (mapCrc)
        TreeScan::loadTexturesMap(gameId, resources, textures);

    QStringList packages;
    if (inputFile != "")
    {
        inputFile = QDir::cleanPath(inputFile);
        QString relPath = g_GameData->RelativeGameData(inputFile);
        if (inputFile == relPath)
        {
            PERROR(QString("ERROR: Input package path: ") + inputFile + " is not part of game!\n");
            return false;
        }
        packages.append(relPath);
    }
    else
    {
        packages = g_GameData->packageFiles;
    }

    QDir().mkpath(outputDir);

    for (int p = 0; p < packages.count(); p++)
    {
        PINFO(QString("Package ") + QString::number(p + 1) + "/" +
                             QString::number(packages.count()) + " : " +
                             packages[p] + "\n");

        Package package;
        if (package.Open(g_GameData->GamePath() + packages[p]) != 0)
        {
            PERROR(QString("ERROR: Issue opening package file: ") + packages[p] + "\n");
            continue;
        }

        for (int e = 0; e < package.exportsTable.count(); e++)
        {
            Package::ExportEntry& exp = package.exportsTable[e];
            int id = package.getClassNameId(exp.getClassId());
            if (id == package.nameIdTexture2D ||
                id == package.nameIdLightMapTexture2D ||
                id == package.nameIdShadowMapTexture2D ||
                id == package.nameIdTextureFlipBook)
            {
                ByteBuffer exportData = package.getExportData(e);
                if (exportData.ptr() == nullptr)
                {
                    PERROR(QString("Error: Texture ") + exp.objectName +
                                 " has broken export data in package: " +
                                 packages[p] +"\nExport Id: " + QString::number(e + 1) + "\nSkipping...\n");
                    continue;
                }
                Texture texture(package, e, exportData);
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
                    QString archive = texture.getProperties().getProperty("TextureFileCacheName").getValueName();
                    if (archive != textureTfcFilter ||
                        !texture.HasExternalMips())
                    {
                        continue;
                    }
                }
                QString name = exp.objectName;
                uint crc = 0;
                if (mapCrc)
                    crc = Misc::GetCRCFromTextureMap(textures, e, packages[p]);
                if (crc == 0)
                    crc = texture.getCrcTopMipmap();
                if (crc == 0)
                {
                    PERROR(QString("Error: Texture ") + name + " is broken in package: " +
                                 packages[p] +"\nExport Id: " + QString::number(e + 1) + "\nSkipping...\n");
                    continue;
                }
                QString outputFile = outputDir + "/" +  name + QString::asprintf("_0x%08X", crc);
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
                PixelFormat pixelFormat = Image::getPixelFormatType(texture.getProperties().getProperty("Format").getValueName());
                if (png)
                {
                    Texture::TextureMipMap mipmap = texture.getTopMipmap();
                    ByteBuffer data = texture.getTopImageData();
                    if (data.ptr() != nullptr)
                    {
                        if (QFile(outputFile).exists())
                            QFile(outputFile).remove();
                        Image::saveToPng(data.ptr(), mipmap.width, mipmap.height, pixelFormat, outputFile, clearAlpha);
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
                                     QString::asprintf("_0x%08X", crc) + " is broken in game data!\n");
                    }
                }
            }
        }
    }

    PINFO("Extracting textures completed.\n\n");
    return true;
}

bool CmdLineTools::extractAllMovieTextures(MeType gameId, QString &outputDir, QString &inputFile,
                                           bool pccOnly, bool tfcOnly, bool mapCrc,
                                           QString &textureTfcFilter)
{
    Resources resources;
    resources.loadMD5Tables();
    ConfigIni configIni = ConfigIni();
    g_GameData->Init(gameId, configIni);
    if (!Misc::CheckGamePath())
        return false;

    QList<TextureMapEntry> textures;
    if (mapCrc)
        TreeScan::loadTexturesMap(gameId, resources, textures);

    QStringList packages;
    if (inputFile != "")
    {
        inputFile = QDir::cleanPath(inputFile);
        QString relPath = g_GameData->RelativeGameData(inputFile);
        if (inputFile == relPath)
        {
            PERROR(QString("ERROR: Input package path: ") + inputFile + " is not part of game!\n");
            return false;
        }
        packages.append(relPath);
    }
    else
    {
        packages = g_GameData->packageFiles;
    }

    QDir().mkpath(outputDir);

    for (int p = 0; p < packages.count(); p++)
    {
        PINFO(QString("Package ") + QString::number(p + 1) + "/" +
                             QString::number(packages.count()) + " : " +
                             packages[p] + "\n");

        Package package;
        if (package.Open(g_GameData->GamePath() + packages[p]) != 0)
        {
            PERROR(QString("ERROR: Issue opening package file: ") + packages[p] + "\n");
            continue;
        }

        for (int e = 0; e < package.exportsTable.count(); e++)
        {
            Package::ExportEntry& exp = package.exportsTable[e];
            int id = package.getClassNameId(exp.getClassId());
            if (id == package.nameIdTextureMovie)
            {
                ByteBuffer exportData = package.getExportData(e);
                if (exportData.ptr() == nullptr)
                {
                    PERROR(QString("Error: Movie Texture ") + exp.objectName +
                                 " has broken export data in package: " +
                                 packages[p] +"\nExport Id: " + QString::number(e + 1) + "\nSkipping...\n");
                    continue;
                }
                TextureMovie textureMovie(package, e, exportData);
                exportData.Free();
                if (!textureMovie.hasTextureData())
                {
                    continue;
                }

                bool tfcPropExists = textureMovie.getProperties().exists("TextureFileCacheName");
                if ((pccOnly && tfcPropExists) ||
                    (tfcOnly && !tfcPropExists) ||
                    (tfcOnly && textureMovie.getStorageType() != StorageTypes::extUnc &&
                     textureMovie.getStorageType() != StorageTypes::extUnc2))
                {
                    continue;
                }
                if (!pccOnly && !tfcOnly && textureTfcFilter.length() != 0)
                {
                    if (!tfcPropExists)
                        continue;
                    QString archive = textureMovie.getProperties().getProperty("TextureFileCacheName").getValueName();
                    if (archive != textureTfcFilter ||
                        textureMovie.getStorageType() != StorageTypes::extUnc)
                    {
                        continue;
                    }
                }
                QString name = exp.objectName;
                uint crc = 0;
                if (mapCrc)
                    crc = Misc::GetCRCFromTextureMap(textures, e, packages[p]);
                if (crc == 0)
                    crc = textureMovie.getCrcData();
                if (crc == 0)
                {
                    PERROR(QString("Error: Movie Texture ") + name + " is broken in package: " +
                                 packages[p] +"\nExport Id: " + QString::number(e + 1) + "\nSkipping...\n");
                    continue;
                }
                QString outputFile = outputDir + "/" +  name + QString::asprintf("_0x%08X.bik", crc);
                if (QFile(outputFile).exists())
                    continue;
                auto data = textureMovie.getData();
                FileStream output = FileStream(outputFile, FileMode::Create, FileAccess::ReadWrite);
                output.WriteFromBuffer(data);
                data.Free();
            }
        }
    }

    PINFO("Extracting movie textures completed.\n\n");
    return true;
}

bool CmdLineTools::CheckTextures(MeType gameId)
{
    ConfigIni configIni = ConfigIni();
    g_GameData->Init(gameId, configIni);
    if (!Misc::CheckGamePath())
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
                texture.removeEmptyMips();
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

bool CmdLineTools::checkGameFilesAfter(MeType gameType)
{
    ConfigIni configIni = ConfigIni();
    g_GameData->Init(gameType, configIni);
    if (!Misc::CheckGamePath())
        return false;

    PINFO("Checking for vanilla files after textures installation...\n");

    QString path = g_GameData->MainData() + "/SFXTest.pcc";
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
        fs.Seek(-MEMMarkerLength, SeekOrigin::End);
        QString marker;
        fs.ReadStringASCII(marker, MEMMarkerLength);
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
    if (!Misc::CheckGamePath())
        return false;

    QString path = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation).first() +
            "/MassEffectModder";
    QString mapFile = path + QString("/mele%1map.bin").arg((int)gameType);
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
