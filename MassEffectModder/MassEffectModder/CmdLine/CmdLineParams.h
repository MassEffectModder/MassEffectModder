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

#ifndef CMD_LINE_PARAMS_H
#define CMD_LINE_PARAMS_H

typedef enum : int
{
    UNKNOWN = 0,
    HELP,
    VERSION,
    SCAN,
    SCAN_TEXTURES,
    UPDATE_TOC,
    UNPACK_DLCS,
    REPACK,
    CONVERT_TO_MEM,
    CONVERT_GAME_IMAGE,
    CONVERT_GAME_IMAGES,
    CONVERT_IMAGE,
    INSTALL_MODS,
    EXTRACT_MOD,
    EXTRACT_MEM,
    EXTRACT_TPF,
    DETECT_MODS,
    DETECT_BAD_MODS,
    APPLY_LODS_GFX,
    PRINT_LODS,
    REMOVE_LODS,
    APPLY_ME1_LAA,
    CHECK_GAME_DATA_TEXTURES,
    CHECK_GAME_DATA_MISMATCH,
    CHECK_GAME_DATA_AFTER,
    CHECK_GAME_DATA_VANILLA,
    CHECK_FOR_MARKERS,
    EXTRACT_ALL_DDS,
    EXTRACT_ALL_PNG,
    EXTRACT_ALL_BIK,
    COMPACT_DLC,
    UNPACK_ARCHIVE,
    LIST_ARCHIVE,
    SET_GAME_DATA_PATH,
    GET_GAME_PATHS,
#if !defined(_WIN32)
    SET_GAME_USER_PATH,
#endif
    FIX_TEXTURES_PROPERTY,
} CmdType;

int ProcessArguments();
void DisplayHelp();

#endif
