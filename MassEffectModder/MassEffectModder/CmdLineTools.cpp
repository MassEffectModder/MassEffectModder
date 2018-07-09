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

#include "Exceptions/SignalHandler.h"
#include "Helpers/Misc.h"

#include "CmdLineTools.h"
#include "ConfigIni.h"
#include "TreeScan.h"
#include "GameData.h"
#include "MemTypes.h"

static QList<FoundTexture> *textures = nullptr;
static TreeScan *treeScan = nullptr;

static bool CheckGamePath()
{
    if (g_GameData->GamePath() == "" || !QDir(g_GameData->GamePath()).exists())
    {
        ConsoleWrite("Error: Could not found the game!");
        return false;
    }

    return true;
}

int ScanTextures(MeType gameId, bool ipc)
{
    int errorCode;

    treeScan = new TreeScan();
    auto *configIni = new ConfigIni();
    auto *gameData = g_GameData;
    gameData->Init(gameId, configIni);

    if (!CheckGamePath())
        return -1;

    if (ipc)
    {
    }

    gameData->getPackages();

    if (gameId != MeType::ME1_TYPE)
        gameData->getTfcTextures();

    ConsoleWrite("Scan textures started...");

    errorCode = treeScan->PrepareListOfTextures(gameId, ipc);
    textures = treeScan->treeScan;
    treeScan->ReleaseTreeScan(textures);
    delete textures;

    ConsoleWrite("Scan textures finished.\n");

    return errorCode;
}
