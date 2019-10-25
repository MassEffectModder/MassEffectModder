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

#ifndef LOD_SETTINGS_H
#define LOD_SETTINGS_H

#include <Program/ConfigIni.h>
#include <Types/MemTypes.h>

class LODSettings
{

public:

    static void readLOD(MeType gameId, ConfigIni &engineConf, QString &log);
    static void readLODIpc(MeType gameId, ConfigIni &engineConf);
    static void updateLOD(MeType gameId, ConfigIni &engineConf, bool limit2k);
    static void removeLOD(MeType gameId, ConfigIni &engineConf);
    static void updateGFXSettings(MeType gameId, ConfigIni &engineConf, bool softShadowsME1, bool meuitmMode);
};

#endif
