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

#include <GameData/LODSettings.h>
#include <Helpers/Exception.h>
#include <Helpers/MiscHelpers.h>

void LODSettings::readLODIpc(MeType gameId, ConfigIni &engineConf)
{
    if (gameId == MeType::ME1_TYPE)
    {
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_World=" + engineConf.Read("TEXTUREGROUP_World", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_WorldNormalMap=" + engineConf.Read("TEXTUREGROUP_WorldNormalMap", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_AmbientLightMap=" + engineConf.Read("TEXTUREGROUP_AmbientLightMap", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_LightAndShadowMap=" + engineConf.Read("TEXTUREGROUP_LightAndShadowMap", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Environment_64=" + engineConf.Read("TEXTUREGROUP_Environment_64", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Environment_128=" + engineConf.Read("TEXTUREGROUP_Environment_128", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Environment_256=" + engineConf.Read("TEXTUREGROUP_Environment_256", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Environment_512=" + engineConf.Read("TEXTUREGROUP_Environment_512", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Environment_1024=" + engineConf.Read("TEXTUREGROUP_Environment_1024", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_VFX_64=" + engineConf.Read("TEXTUREGROUP_VFX_64", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_VFX_128=" + engineConf.Read("TEXTUREGROUP_VFX_128", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_VFX_256=" + engineConf.Read("TEXTUREGROUP_VFX_256", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_VFX_512" + engineConf.Read("TEXTUREGROUP_VFX_512", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_VFX_1024=" + engineConf.Read("TEXTUREGROUP_VFX_1024", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_APL_128=" + engineConf.Read("TEXTUREGROUP_APL_128", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_APL_256=" + engineConf.Read("TEXTUREGROUP_APL_256", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_APL_512=" + engineConf.Read("TEXTUREGROUP_APL_512", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_APL_1024=" + engineConf.Read("TEXTUREGROUP_APL_1024", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_GUI=" + engineConf.Read("TEXTUREGROUP_GUI", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Promotional=" + engineConf.Read("TEXTUREGROUP_Promotional", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Character_1024=" + engineConf.Read("TEXTUREGROUP_Character_1024", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Character_Diff=" + engineConf.Read("TEXTUREGROUP_Character_Diff", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Character_Norm=" + engineConf.Read("TEXTUREGROUP_Character_Norm", "TextureLODSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Character_Spec=" + engineConf.Read("TEXTUREGROUP_Character_Spec", "TextureLODSettings"));
    }
    else if (gameId == MeType::ME2_TYPE)
    {
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_World=" + engineConf.Read("TEXTUREGROUP_World", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_WorldNormalMap=" + engineConf.Read("TEXTUREGROUP_WorldNormalMap", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_AmbientLightMap=" + engineConf.Read("TEXTUREGROUP_AmbientLightMap", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_LightAndShadowMap=" + engineConf.Read("TEXTUREGROUP_LightAndShadowMap", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_RenderTarget=" + engineConf.Read("TEXTUREGROUP_RenderTarget", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Environment_64=" + engineConf.Read("TEXTUREGROUP_Environment_64", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Environment_128=" + engineConf.Read("TEXTUREGROUP_Environment_128", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Environment_256=" + engineConf.Read("TEXTUREGROUP_Environment_256", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Environment_512=" + engineConf.Read("TEXTUREGROUP_Environment_512", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Environment_1024=" + engineConf.Read("TEXTUREGROUP_Environment_1024", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_VFX_64=" + engineConf.Read("TEXTUREGROUP_VFX_64", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_VFX_128=" + engineConf.Read("TEXTUREGROUP_VFX_128", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_VFX_256=" + engineConf.Read("TEXTUREGROUP_VFX_256", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_VFX_512=" + engineConf.Read("TEXTUREGROUP_VFX_512", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_VFX_1024=" + engineConf.Read("TEXTUREGROUP_VFX_1024", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_APL_128=" + engineConf.Read("TEXTUREGROUP_APL_128", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_APL_256=" + engineConf.Read("TEXTUREGROUP_APL_256", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_APL_512=" + engineConf.Read("TEXTUREGROUP_APL_512", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_APL_1024=" + engineConf.Read("TEXTUREGROUP_APL_1024", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_UI=" + engineConf.Read("TEXTUREGROUP_UI", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Promotional=" + engineConf.Read("TEXTUREGROUP_Promotional", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Character_1024=" + engineConf.Read("TEXTUREGROUP_Character_1024", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Character_Diff=" + engineConf.Read("TEXTUREGROUP_Character_Diff", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Character_Norm=" + engineConf.Read("TEXTUREGROUP_Character_Norm", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Character_Spec=" + engineConf.Read("TEXTUREGROUP_Character_Spec", "SystemSettings"));
    }
    else if (gameId == MeType::ME3_TYPE)
    {
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_World=" + engineConf.Read("TEXTUREGROUP_World", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_WorldSpecular=" + engineConf.Read("TEXTUREGROUP_WorldSpecular", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_WorldNormalMap=" + engineConf.Read("TEXTUREGROUP_WorldNormalMap", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_AmbientLightMap=" + engineConf.Read("TEXTUREGROUP_AmbientLightMap", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_ShadowMap=" + engineConf.Read("TEXTUREGROUP_ShadowMap", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_RenderTarget=" + engineConf.Read("TEXTUREGROUP_RenderTarget", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Environment_64=" + engineConf.Read("TEXTUREGROUP_Environment_64", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Environment_128=" + engineConf.Read("TEXTUREGROUP_Environment_128", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Environment_256=" + engineConf.Read("TEXTUREGROUP_Environment_256", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Environment_512=" + engineConf.Read("TEXTUREGROUP_Environment_512", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Environment_1024=" + engineConf.Read("TEXTUREGROUP_Environment_1024", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_VFX_64=" + engineConf.Read("TEXTUREGROUP_VFX_64", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_VFX_128=" + engineConf.Read("TEXTUREGROUP_VFX_128", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_VFX_256=" + engineConf.Read("TEXTUREGROUP_VFX_256", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_VFX_512=" + engineConf.Read("TEXTUREGROUP_VFX_512", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_VFX_1024=" + engineConf.Read("TEXTUREGROUP_VFX_1024", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_APL_128=" + engineConf.Read("TEXTUREGROUP_APL_128", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_APL_256=" + engineConf.Read("TEXTUREGROUP_APL_256", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_APL_512=" + engineConf.Read("TEXTUREGROUP_APL_512", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_APL_1024=" + engineConf.Read("TEXTUREGROUP_APL_1024", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_UI=" + engineConf.Read("TEXTUREGROUP_UI", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Promotional=" + engineConf.Read("TEXTUREGROUP_Promotional", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Character_1024=" + engineConf.Read("TEXTUREGROUP_Character_1024", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Character_Diff=" + engineConf.Read("TEXTUREGROUP_Character_Diff", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Character_Norm=" + engineConf.Read("TEXTUREGROUP_Character_Norm", "SystemSettings"));
        ConsoleWrite(QString("[IPC]LODLINE ") + "TEXTUREGROUP_Character_Spec=" + engineConf.Read("TEXTUREGROUP_Character_Spec", "SystemSettings"));
    }
    else
    {
        CRASH();
    }
}

void LODSettings::updateGFXSettings(MeType gameId, ConfigIni &/*engineConf*/)
{
    if (gameId == MeType::ME1_TYPE)
    {
    }
    else if (gameId == MeType::ME2_TYPE)
    {
    }
    else if (gameId == MeType::ME3_TYPE)
    {
    }
    else
    {
        CRASH();
    }
}
