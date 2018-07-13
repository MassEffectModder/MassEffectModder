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

#include <QCoreApplication>

#include "Exceptions/SignalHandler.h"
#include "Helpers/MiscHelpers.h"

#include "CmdLineParams.h"
#include "CmdLineTools.h"
#include "ConfigIni.h"
#include "GameData.h"
#include "MemTypes.h"
#include "Version.h"

void DisplayHelp()
{
    ConsoleWrite("\nHelp:");
    ConsoleWrite("  --help");
    ConsoleWrite("     This help");
    ConsoleWrite("");
    ConsoleWrite("  --scan --gameid <game id> [--ipc]");
    ConsoleWrite("     Scan game data for textures.");
    ConsoleWrite("");
}

bool hasValue(const QStringList &args, int curPos)
{
    return args.count() >= (curPos + 2) && !args[curPos + 1].contains("--");
}

int ProcessArguments()
{
    int errorCode = 0;
    int cmd = CmdType::UNKNOWN;
    MeType gameId = MeType::UNKNOWN_TYPE;
    bool ipc = false;

    const QStringList args = QCoreApplication::arguments();
    for (int l = 0; l < args.count(); l++)
    {
        const QString arg = args[l].toLower();
        if (arg == "--help")
            cmd = CmdType::HELP;
        if (arg == "--scan")
            cmd = CmdType::SCAN;
        else if (arg == "--gameid" && hasValue(args, l))
        {
            bool ok;
            int id = args[l + 1].toInt(&ok);
            if (ok && id >= 1 && id <= 3)
            {
                gameId = (MeType)id;
            }
        }
        else if (arg == "--ipc")
            ipc = true;
    }

    switch (cmd)
    {
        case CmdType::HELP:
            DisplayHelp();
            return 0;
        case CmdType::UNKNOWN:
            ConsoleWrite("Wrong command!");
            DisplayHelp();
            return -1;
    }

    switch (cmd)
    {
        case CmdType::SCAN:
            if (gameId == MeType::UNKNOWN_TYPE)
            {
                ConsoleWrite("Wrong game id!");
                errorCode = -1;
                break;
            }
            errorCode = ScanTextures(gameId, ipc);
            break;
    }

    if (errorCode != 0)
    {
        DisplayHelp();
        return errorCode;
    }

    return errorCode;
}
