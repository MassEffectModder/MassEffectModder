/*
 * MassEffectModder
 *
 * Copyright (C) 2018-2022 Pawel Kolodziejski
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

#include <Md5/MD5BadEntries.h>

MD5ModFileEntry badMOD[] =
{

{
"/Game/ME2/BioGame/CookedPCConsole/SFXGame.pcc",
{ 0xB3, 0x7E, 0x7A, 0xCD, 0x92, 0xD1, 0x44, 0x0E, 0x2F, 0x4D, 0x5D, 0xCE, 0x4C, 0xDA, 0x85, 0xBC, },
"ME2LE One Probe All Resources - v0.0.2",
},
{
"/Game/ME2/BioGame/DLC/DLC_MOD_GarrusUndamaged/CookedPCConsole/BioH_Garrus_02.pcc",
{ 0xEA, 0xFD, 0x80, 0x8C, 0x9E, 0xBE, 0xBA, 0xA1, 0xD8, 0x31, 0xFB, 0xDF, 0x1D, 0xED, 0xBB, 0x25, },
"ME2LE Garrus Undamaged Armor - Alt Appearance Replacer v1.0",
},
{
"/Game/ME2/BioGame/DLC/DLC_MOD_GarrusUndamaged/CookedPCConsole/BioH_Garrus_02.pcc",
{ 0x1C, 0x4D, 0xAE, 0x6E, 0x1A, 0x43, 0xEC, 0xFE, 0x78, 0xE9, 0xEC, 0xC9, 0xBC, 0xF5, 0x1B, 0x02, },
"ME2LE Garrus Undamaged Armor - Alt Appearance Replacer v1.1",
},

};

const int badMODSize = sizeof (badMOD) / sizeof (MD5ModFileEntry);
