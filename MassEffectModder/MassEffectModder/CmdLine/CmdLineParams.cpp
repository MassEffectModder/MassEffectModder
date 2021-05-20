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

#include <CmdLine/CmdLineParams.h>
#include <CmdLine/CmdLineTools.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <GameData/GameData.h>
#include <GameData/TOCFile.h>
#include <Misc/Misc.h>
#include <Program/ConfigIni.h>
#include <Types/MemTypes.h>

static bool hasValue(const QStringList &args, int curPos)
{
    return args.count() >= (curPos + 2) && !args[curPos + 1].startsWith("--");
}

int ProcessArguments()
{
    int errorCode = 0;
    int cmd = CmdType::UNKNOWN;
    MeType gameId = MeType::UNKNOWN_TYPE;
    bool markToConvert = false;
    bool alotMode = false;
    bool skipMarkers = false;
    bool pccOnly = false;
    bool tfcOnly = false;
    bool verify = false;
    bool mapCRC = false;
    bool removeEmptyMips = false;
    bool flattenPath = false;
    bool clearAlpha = false;
    int thresholdValue = 128;
    int cacheAmountValue = -1;
    QString input, output, threshold, format, tfcName;
    QString dlcName, path, cacheAmount, filter;
    CmdLineTools tools;

    QStringList args = QCoreApplication::arguments();
    if (args.count() != 0)
        args.removeFirst();

    for (int l = 0; l < args.count(); l++)
    {
        const QString arg = args[l].toLower();
        if (arg == "--help")
            cmd = CmdType::HELP;
        else if (arg == "--version")
            cmd = CmdType::VERSION;
        else if (arg == "--scan")
            cmd = CmdType::SCAN;
        else if (arg == "--scan-textures")
            cmd = CmdType::SCAN_TEXTURES;
        else if (arg == "--update-toc")
            cmd = CmdType::UPDATE_TOC;
        else if (arg == "--convert-to-mem")
            cmd = CmdType::CONVERT_TO_MEM;
        else if (arg == "--convert-game-image")
            cmd = CmdType::CONVERT_GAME_IMAGE;
        else if (arg == "--convert-game-images")
            cmd = CmdType::CONVERT_GAME_IMAGES;
        else if (arg == "--convert-image")
            cmd = CmdType::CONVERT_IMAGE;
        else if (arg == "--install-mods")
            cmd = CmdType::INSTALL_MODS;
        else if (arg == "--extract-mem")
            cmd = CmdType::EXTRACT_MEM;
        else if (arg == "--detect-mods")
            cmd = CmdType::DETECT_MODS;
        else if (arg == "--detect-bad-mods")
            cmd = CmdType::DETECT_BAD_MODS;
        else if (arg == "--apply-lods-gfx")
            cmd = CmdType::APPLY_LODS_GFX;
        else if (arg == "--print-lods")
            cmd = CmdType::PRINT_LODS;
        else if (arg == "--remove-lods")
            cmd = CmdType::REMOVE_LODS;
        else if (arg == "--check-game-data-textures")
            cmd = CmdType::CHECK_GAME_DATA_TEXTURES;
        else if (arg == "--check-game-data-mismatch")
            cmd = CmdType::CHECK_GAME_DATA_MISMATCH;
        else if (arg == "--check-game-data-after")
            cmd = CmdType::CHECK_GAME_DATA_AFTER;
        else if (arg == "--check-game-data-vanilla")
            cmd = CmdType::CHECK_GAME_DATA_VANILLA;
        else if (arg == "--check-for-markers")
            cmd = CmdType::CHECK_FOR_MARKERS;
        else if (arg == "--extract-all-dds")
            cmd = CmdType::EXTRACT_ALL_DDS;
        else if (arg == "--extract-all-png")
            cmd = CmdType::EXTRACT_ALL_PNG;
        else if (arg == "--extract-all-bik")
            cmd = CmdType::EXTRACT_ALL_BIK;
        else if (arg == "--unpack-archive")
            cmd = CmdType::UNPACK_ARCHIVE;
        else if (arg == "--list-archive")
            cmd = CmdType::LIST_ARCHIVE;
        else if (arg == "--set-game-data-path")
            cmd = CmdType::SET_GAME_DATA_PATH;
        else if (arg == "--get-game-paths")
            cmd = CmdType::GET_GAME_PATHS;
#if !defined(_WIN32)
        else if (arg == "--set-game-user-path")
            cmd = CmdType::SET_GAME_USER_PATH;
#endif
        else
            continue;
        args.removeAt(l);
        break;
    }

    switch (cmd)
    {
    case CmdType::HELP:
        DisplayHelp();
        return 0;
    case CmdType::UNKNOWN:
        PERROR("Wrong command!\n");
        DisplayHelp();
        return 1;
    }

    for (int l = 0; l < args.count(); l++)
    {
        const QString arg = args[l].toLower();
        if (arg == "--gameid" && hasValue(args, l))
        {
            bool ok;
            int id = args[l + 1].toInt(&ok);
            if (ok && id >= 1 && id <= 3)
            {
                gameId = (MeType)id;
            }
            args.removeAt(l);
            args.removeAt(l--);
        }
        else if (arg == "--ipc")
        {
            g_ipc = true;
            args.removeAt(l--);
        }
        else if (arg == "--input" && hasValue(args, l))
        {
            input = args[l + 1].replace('\\', '/');
            if (input.length() == 0)
            {
                PERROR("Input path param wrong!\n");
                return -1;
            }
            args.removeAt(l);
            args.removeAt(l--);
        }
        else if (arg == "--output" && hasValue(args, l))
        {
            output = args[l + 1].replace('\\', '/');
            if (output.length() == 0)
            {
                PERROR("Output path wrong!\n");
                return -1;
            }
            args.removeAt(l);
            args.removeAt(l--);
        }
        else if (arg == "--mark-to-convert")
        {
            markToConvert = true;
            args.removeAt(l--);
        }
        else if (arg == "--alot-mode")
        {
            alotMode = true;
            args.removeAt(l--);
        }
        else if (arg == "--meuitm-mode")
        {
            args.removeAt(l--);
        }
        else if (arg == "--skip-markers")
        {
            skipMarkers = true;
            args.removeAt(l--);
        }
        else if (arg == "--verify")
        {
            verify = true;
            args.removeAt(l--);
        }
        else if (arg == "--pcc-only")
        {
            if (tfcName != "" || tfcOnly)
            {
                PERROR("--tfc-name or --tfc-only is already enabled!\n");
                return -1;
            }
            pccOnly = true;
            args.removeAt(l--);
        }
        else if (arg == "--tfc-only")
        {
            if (tfcName != "" || pccOnly)
            {
                PERROR("--tfc-name or --pcc-only is already enabled!\n");
                return -1;
            }
            tfcOnly = true;
            args.removeAt(l--);
        }
        else if (arg == "--package-path" && hasValue(args, l))
        {
            input = args[l + 1].replace('\\', '/');
            if (input.length() == 0)
            {
                PERROR("Input package path param wrong!\n");
                return -1;
            }
            args.removeAt(l);
            args.removeAt(l--);
        }
        else if (arg == "--map-crc")
        {
            mapCRC = true;
            args.removeAt(l--);
        }
        else if (arg == "--clear-alpha")
        {
            clearAlpha = true;
            args.removeAt(l--);
        }
        else if (arg == "--threshold" && hasValue(args, l))
        {
            threshold = args[l + 1];
            if (threshold.length() != 0)
            {
                thresholdValue = threshold.toInt();
            }
            args.removeAt(l);
            args.removeAt(l--);
        }
        else if (arg == "--format" && hasValue(args, l))
        {
            format = args[l + 1];
            args.removeAt(l);
            args.removeAt(l--);
        }
        else if (arg == "--tfc-name" && hasValue(args, l))
        {
            if (pccOnly || tfcOnly)
            {
                PERROR("--pcc-only or --tfc-only is already enabled!\n");
                return -1;
            }
            tfcName = args[l + 1];
            args.removeAt(l);
            args.removeAt(l--);
        }
        else if (arg == "--dlc-name" && hasValue(args, l))
        {
            dlcName = args[l + 1];
            args.removeAt(l);
            args.removeAt(l--);
        }
        else if (arg == "--debug-logs")
        {
            g_logs->ChangeLogLevel(LOG_DEBUG);
            args.removeAt(l--);
        }
        else if (arg == "--path" && hasValue(args, l))
        {
            path = args[l + 1];
            args.removeAt(l);
            args.removeAt(l--);
        }
        else if (arg == "--cache-amount" && hasValue(args, l))
        {
            cacheAmount = args[l + 1];
            if (cacheAmount.length() != 0)
            {
                cacheAmountValue = cacheAmount.toInt();
            }
            args.removeAt(l);
            args.removeAt(l--);
        }
        else if ((arg == "--filter-with-ext" || arg == "--filter") && hasValue(args, l))
        {
            filter = args[l + 1];
            args.removeAt(l);
            args.removeAt(l--);
        }
        else if (arg == "--remove-empty-mips")
        {
            removeEmptyMips = true;
            args.removeAt(l);
            args.removeAt(l--);
        }
        else if (arg == "--flatten-archive-path")
        {
            flattenPath = true;
            args.removeAt(l--);
        }
    }
    if (args.count() != 0)
    {
        QString error = "Wrong options: ";
        for (const auto& a : qAsConst(args)) {
            error += a + " ";
        }
        PERROR(error + "\n");
        DisplayHelp();
        return 1;
    }

    switch (cmd)
    {
    case CmdType::VERSION:
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]VERSION %1\n").arg(MEM_VERSION));
        }
        else
        {
            ConsoleWrite(QString("Version: %1\n").arg(MEM_VERSION));
        }
        ConsoleSync();
        break;
    case CmdType::SCAN:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!\n");
            errorCode = 1;
            break;
        }
        errorCode = tools.scan(gameId);
        break;
    case CmdType::SCAN_TEXTURES:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!\n");
            errorCode = 1;
            break;
        }
        errorCode = tools.scanTextures(gameId, removeEmptyMips);
        break;
    case CmdType::UPDATE_TOC:
    {
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!\n");
            errorCode = 1;
            break;
        }
        if (!tools.updateTOCs(gameId))
            errorCode = 1;
        break;
    }
    case CmdType::CONVERT_TO_MEM:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!\n");
            errorCode = 1;
            break;
        }
        if (!QDir(input).exists())
        {
            PERROR("Input folder doesn't exists! " + input + "\n");
            errorCode = 1;
            break;
        }
        if (output.length() == 0)
        {
            PERROR("Output param empty!\n");
            errorCode = 1;
            break;
        }
        if (!output.endsWith(".mem", Qt::CaseInsensitive))
        {
            PERROR(QString("Error: output file is not mem: ") + output + "\n");
            errorCode = 1;
            break;
        }
        if (!tools.ConvertToMEM(gameId, input, output, markToConvert))
            errorCode = 1;
        break;
    case CmdType::CONVERT_GAME_IMAGE:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!\n");
            errorCode = 1;
            break;
        }
        if (!QFile(input).exists())
        {
            PERROR("Input file doesn't exists! " + input + "\n");
            errorCode = 1;
            break;
        }
        if (!output.endsWith(".dds", Qt::CaseInsensitive))
        {
            PERROR(QString("Error: output file is not dds: ") + output + "\n");
            errorCode = 1;
            break;
        }
        if (!tools.convertGameImage(gameId, input, output, markToConvert))
            errorCode = 1;
        break;
    case CmdType::CONVERT_IMAGE:
        if (!QFile(input).exists())
        {
            PERROR("Input file doesn't exists! " + input + "\n");
            errorCode = 1;
            break;
        }
        if (output.length() == 0)
        {
            PERROR("Output param empty!\n");
            errorCode = 1;
            break;
        }
        if (!tools.convertImage(input, output, format, thresholdValue))
            errorCode = 1;
        break;
    case CmdType::CONVERT_GAME_IMAGES:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!\n");
            errorCode = 1;
            break;
        }
        if (!QDir(input).exists())
        {
            PERROR("Input folder doesn't exists! " + input + "\n");
            errorCode = 1;
            break;
        }
        if (output.length() == 0)
        {
            PERROR("Output param empty!\n");
            errorCode = 1;
            break;
        }
        if (!tools.convertGameImages(gameId, input, output, markToConvert))
            errorCode = 1;
        break;
    case CmdType::INSTALL_MODS:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!\n");
            errorCode = 1;
            break;
        }
        if (!QDir(input).exists())
        {
            PERROR("Input folder doesn't exists! " + input + "\n");
            errorCode = 1;
            break;
        }
        if (cacheAmountValue < -1 || cacheAmountValue > 100)
        {
            PERROR("Cache amount must be in range from -1 to 100\n");
            errorCode = 1;
            break;
        }
        if (!tools.InstallMods(gameId, input, alotMode, skipMarkers, verify, cacheAmountValue))
        {
            errorCode = 1;
        }
        break;
    case CmdType::EXTRACT_MEM:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!\n");
            errorCode = 1;
            break;
        }
        if (input.endsWith(".mem", Qt::CaseInsensitive))
        {
            if (!QFile(input).exists())
            {
                PERROR("Input file doesn't exists! " + input + "\n");
                errorCode = 1;
                break;
            }
        }
        else if (!QDir(input).exists())
        {
            PERROR("Input folder doesn't exists! " + input + "\n");
            errorCode = 1;
            break;
        }
        if (!tools.extractMEM(gameId, input, output))
            errorCode = 1;
        break;
    case CmdType::DETECT_MODS:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!\n");
            errorCode = 1;
            break;
        }
        if (!tools.DetectMods(gameId))
            errorCode = 1;
        break;
    case CmdType::DETECT_BAD_MODS:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!\n");
            errorCode = 1;
            break;
        }
        if (!tools.DetectBadMods(gameId))
            errorCode = 1;
        break;
    case CmdType::APPLY_LODS_GFX:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!\n");
            errorCode = 1;
            break;
        }
        if (!tools.ApplyLODAndGfxSettings(gameId))
            errorCode = 1;
        break;
    case CmdType::PRINT_LODS:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!\n");
            errorCode = 1;
            break;
        }
        if (!tools.PrintLODSettings(gameId))
            errorCode = 1;
        break;
    case CmdType::REMOVE_LODS:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!\n");
            errorCode = 1;
            break;
        }
        if (!tools.RemoveLODSettings(gameId))
            errorCode = 1;
        break;
    case CmdType::CHECK_GAME_DATA_TEXTURES:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!\n");
            errorCode = 1;
            break;
        }
        if (!tools.CheckTextures(gameId))
            errorCode = 1;
        break;
    case CmdType::CHECK_GAME_DATA_AFTER:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!\n");
            errorCode = 1;
            break;
        }
        if (!tools.checkGameFilesAfter(gameId))
            errorCode = 1;
        break;
    case CmdType::CHECK_GAME_DATA_MISMATCH:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!\n");
            errorCode = 1;
            break;
        }
        if (!tools.detectsMismatchPackagesAfter(gameId))
            errorCode = 1;
        break;
    case CmdType::CHECK_GAME_DATA_VANILLA:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!");
            errorCode = 1;
            break;
        }
        if (!tools.CheckGameDataAndMods(gameId))
            errorCode = 1;
        break;
    case CmdType::CHECK_FOR_MARKERS:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!\n");
            errorCode = 1;
            break;
        }
        if (!tools.CheckForMarkers(gameId))
            errorCode = 1;
        break;
    case CmdType::EXTRACT_ALL_DDS:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!\n");
            errorCode = 1;
            break;
        }
        if (output.length() == 0)
        {
            PERROR("Output param empty!\n");
            errorCode = 1;
            break;
        }
        if (input.length() != 0 && !QFile(input).exists())
        {
            PERROR("Input package doesn't exists! " + input + "\n");
            errorCode = 1;
            break;
        }
        if (!tools.extractAllTextures(gameId, output, input, false, pccOnly, tfcOnly, mapCRC, tfcName))
            errorCode = 1;
        break;
    case CmdType::EXTRACT_ALL_PNG:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!\n");
            errorCode = 1;
            break;
        }
        if (output.length() == 0)
        {
            PERROR("Output param empty!\n");
            errorCode = 1;
            break;
        }
        if (input.length() != 0 && !QFile(input).exists())
        {
            PERROR("Input package doesn't exists! " + input + "\n");
            errorCode = 1;
            break;
        }
        if (!tools.extractAllTextures(gameId, output, input, true, pccOnly, tfcOnly, mapCRC, tfcName, clearAlpha))
            errorCode = 1;
        break;
    case CmdType::EXTRACT_ALL_BIK:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!\n");
            errorCode = 1;
            break;
        }
        if (output.length() == 0)
        {
            PERROR("Output param empty!\n");
            errorCode = 1;
            break;
        }
        if (input.length() != 0 && !QFile(input).exists())
        {
            PERROR("Input package doesn't exists! " + input + "\n");
            errorCode = 1;
            break;
        }
        if (!tools.extractAllMovieTextures(gameId, output, input, pccOnly, tfcOnly, mapCRC, tfcName))
            errorCode = 1;
        break;
    case CmdType::UNPACK_ARCHIVE:
        if (!QFile(input).exists())
        {
            PERROR("Input file doesn't exists!\n");
            errorCode = 1;
            break;
        }
        if (!tools.unpackArchive(input, output, filter, flattenPath))
            errorCode = 1;
        break;
    case CmdType::LIST_ARCHIVE:
        if (!QFile(input).exists())
        {
            PERROR("Input file doesn't exists!\n");
            errorCode = 1;
            break;
        }
        if (!tools.listArchive(input))
            errorCode = 1;
        break;
    case CmdType::SET_GAME_DATA_PATH:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!\n");
            errorCode = 1;
            break;
        }
        path = QDir::cleanPath(path);
        if (path.length() == 0 || !QDir(path).exists())
        {
            PERROR("Game path doesn't exists!\n");
            errorCode = 1;
            break;
        }
        if (!Misc::SetGameDataPath(gameId, path))
            errorCode = 1;
        break;
    case CmdType::GET_GAME_PATHS:
        if (!tools.GetGamePaths())
            errorCode = 1;
        break;
#if !defined(_WIN32)
    case CmdType::SET_GAME_USER_PATH:
        if (gameId == MeType::UNKNOWN_TYPE)
        {
            PERROR("Wrong game id!\n");
            errorCode = 1;
            break;
        }
        path = QDir::cleanPath(path);
        if (path.length() == 0 || !QDir(path).exists())
        {
            PERROR("Game config path doesn't exists!\n");
            errorCode = 1;
            break;
        }
        if (!Misc::SetGameUserPath(path))
            errorCode = 1;
        break;
#endif
    }

    return errorCode;
}
