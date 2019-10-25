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

#include <GameData/LODSettings.h>
#include <Helpers/MiscHelpers.h>

void LODSettings::readLOD(MeType gameId, ConfigIni &engineConf, QString &log)
{
    if (gameId == MeType::ME1_TYPE)
    {
        log += "TEXTUREGROUP_World=" + engineConf.Read("TEXTUREGROUP_World", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_WorldNormalMap=" + engineConf.Read("TEXTUREGROUP_WorldNormalMap", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_AmbientLightMap=" + engineConf.Read("TEXTUREGROUP_AmbientLightMap", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_LightAndShadowMap=" + engineConf.Read("TEXTUREGROUP_LightAndShadowMap", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_Environment_64=" + engineConf.Read("TEXTUREGROUP_Environment_64", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_Environment_128=" + engineConf.Read("TEXTUREGROUP_Environment_128", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_Environment_256=" + engineConf.Read("TEXTUREGROUP_Environment_256", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_Environment_512=" + engineConf.Read("TEXTUREGROUP_Environment_512", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_Environment_1024=" + engineConf.Read("TEXTUREGROUP_Environment_1024", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_VFX_64=" + engineConf.Read("TEXTUREGROUP_VFX_64", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_VFX_128=" + engineConf.Read("TEXTUREGROUP_VFX_128", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_VFX_256=" + engineConf.Read("TEXTUREGROUP_VFX_256", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_VFX_512" + engineConf.Read("TEXTUREGROUP_VFX_512", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_VFX_1024=" + engineConf.Read("TEXTUREGROUP_VFX_1024", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_APL_128=" + engineConf.Read("TEXTUREGROUP_APL_128", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_APL_256=" + engineConf.Read("TEXTUREGROUP_APL_256", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_APL_512=" + engineConf.Read("TEXTUREGROUP_APL_512", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_APL_1024=" + engineConf.Read("TEXTUREGROUP_APL_1024", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_GUI=" + engineConf.Read("TEXTUREGROUP_GUI", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_Promotional=" + engineConf.Read("TEXTUREGROUP_Promotional", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_Character_1024=" + engineConf.Read("TEXTUREGROUP_Character_1024", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_Character_Diff=" + engineConf.Read("TEXTUREGROUP_Character_Diff", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_Character_Norm=" + engineConf.Read("TEXTUREGROUP_Character_Norm", "TextureLODSettings") + "\n";
        log += "TEXTUREGROUP_Character_Spec=" + engineConf.Read("TEXTUREGROUP_Character_Spec", "TextureLODSettings") + "\n";
    }
    else if (gameId == MeType::ME2_TYPE)
    {
        log += "TEXTUREGROUP_World=" + engineConf.Read("TEXTUREGROUP_World", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_WorldNormalMap=" + engineConf.Read("TEXTUREGROUP_WorldNormalMap", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_AmbientLightMap=" + engineConf.Read("TEXTUREGROUP_AmbientLightMap", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_LightAndShadowMap=" + engineConf.Read("TEXTUREGROUP_LightAndShadowMap", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_RenderTarget=" + engineConf.Read("TEXTUREGROUP_RenderTarget", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_Environment_64=" + engineConf.Read("TEXTUREGROUP_Environment_64", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_Environment_128=" + engineConf.Read("TEXTUREGROUP_Environment_128", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_Environment_256=" + engineConf.Read("TEXTUREGROUP_Environment_256", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_Environment_512=" + engineConf.Read("TEXTUREGROUP_Environment_512", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_Environment_1024=" + engineConf.Read("TEXTUREGROUP_Environment_1024", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_VFX_64=" + engineConf.Read("TEXTUREGROUP_VFX_64", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_VFX_128=" + engineConf.Read("TEXTUREGROUP_VFX_128", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_VFX_256=" + engineConf.Read("TEXTUREGROUP_VFX_256", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_VFX_512=" + engineConf.Read("TEXTUREGROUP_VFX_512", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_VFX_1024=" + engineConf.Read("TEXTUREGROUP_VFX_1024", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_APL_128=" + engineConf.Read("TEXTUREGROUP_APL_128", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_APL_256=" + engineConf.Read("TEXTUREGROUP_APL_256", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_APL_512=" + engineConf.Read("TEXTUREGROUP_APL_512", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_APL_1024=" + engineConf.Read("TEXTUREGROUP_APL_1024", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_UI=" + engineConf.Read("TEXTUREGROUP_UI", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_Promotional=" + engineConf.Read("TEXTUREGROUP_Promotional", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_Character_1024=" + engineConf.Read("TEXTUREGROUP_Character_1024", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_Character_Diff=" + engineConf.Read("TEXTUREGROUP_Character_Diff", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_Character_Norm=" + engineConf.Read("TEXTUREGROUP_Character_Norm", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_Character_Spec=" + engineConf.Read("TEXTUREGROUP_Character_Spec", "SystemSettings") + "\n";
    }
    else if (gameId == MeType::ME3_TYPE)
    {
        log += "TEXTUREGROUP_World=" + engineConf.Read("TEXTUREGROUP_World", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_WorldSpecular=" + engineConf.Read("TEXTUREGROUP_WorldSpecular", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_WorldNormalMap=" + engineConf.Read("TEXTUREGROUP_WorldNormalMap", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_AmbientLightMap=" + engineConf.Read("TEXTUREGROUP_AmbientLightMap", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_ShadowMap=" + engineConf.Read("TEXTUREGROUP_ShadowMap", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_RenderTarget=" + engineConf.Read("TEXTUREGROUP_RenderTarget", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_Environment_64=" + engineConf.Read("TEXTUREGROUP_Environment_64", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_Environment_128=" + engineConf.Read("TEXTUREGROUP_Environment_128", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_Environment_256=" + engineConf.Read("TEXTUREGROUP_Environment_256", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_Environment_512=" + engineConf.Read("TEXTUREGROUP_Environment_512", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_Environment_1024=" + engineConf.Read("TEXTUREGROUP_Environment_1024", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_VFX_64=" + engineConf.Read("TEXTUREGROUP_VFX_64", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_VFX_128=" + engineConf.Read("TEXTUREGROUP_VFX_128", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_VFX_256=" + engineConf.Read("TEXTUREGROUP_VFX_256", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_VFX_512=" + engineConf.Read("TEXTUREGROUP_VFX_512", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_VFX_1024=" + engineConf.Read("TEXTUREGROUP_VFX_1024", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_APL_128=" + engineConf.Read("TEXTUREGROUP_APL_128", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_APL_256=" + engineConf.Read("TEXTUREGROUP_APL_256", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_APL_512=" + engineConf.Read("TEXTUREGROUP_APL_512", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_APL_1024=" + engineConf.Read("TEXTUREGROUP_APL_1024", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_UI=" + engineConf.Read("TEXTUREGROUP_UI", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_Promotional=" + engineConf.Read("TEXTUREGROUP_Promotional", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_Character_1024=" + engineConf.Read("TEXTUREGROUP_Character_1024", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_Character_Diff=" + engineConf.Read("TEXTUREGROUP_Character_Diff", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_Character_Norm=" + engineConf.Read("TEXTUREGROUP_Character_Norm", "SystemSettings") + "\n";
        log += "TEXTUREGROUP_Character_Spec=" + engineConf.Read("TEXTUREGROUP_Character_Spec", "SystemSettings") + "\n";
    }
    else
    {
        CRASH();
    }
}

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

void LODSettings::updateLOD(MeType gameId, ConfigIni &engineConf, bool limit2k)
{
    if (gameId == MeType::ME1_TYPE)
    {
        if (limit2k)
        {
            engineConf.Write("TEXTUREGROUP_World", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_WorldNormalMap", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_AmbientLightMap", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_LightAndShadowMap", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_Environment_64", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_Environment_128", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_Environment_256", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_Environment_512", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_Environment_1024", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_VFX_64", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_VFX_128", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_VFX_256", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_VFX_512", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_VFX_1024", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_APL_128", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_APL_256", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_APL_512", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_APL_1024", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_GUI", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_Promotional", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_Character_1024", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_Character_Diff", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_Character_Norm", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_Character_Spec", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
        }
        else
        {
            engineConf.Write("TEXTUREGROUP_World", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_WorldNormalMap", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_AmbientLightMap", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_LightAndShadowMap", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_Environment_64", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_Environment_128", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_Environment_256", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_Environment_512", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_Environment_1024", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_VFX_64", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_VFX_128", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_VFX_256", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_VFX_512", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_VFX_1024", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_APL_128", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_APL_256", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_APL_512", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_APL_1024", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_GUI", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_Promotional", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_Character_1024", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_Character_Diff", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_Character_Norm", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
            engineConf.Write("TEXTUREGROUP_Character_Spec", "(MinLODSize=4096,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
       }
    }
    else if (gameId == MeType::ME2_TYPE)
    {
        engineConf.Write("TEXTUREGROUP_World", "(MinLODSize=256,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_WorldNormalMap", "(MinLODSize=256,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_AmbientLightMap", "(MinLODSize=32,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_LightAndShadowMap", "(MinLODSize=1024,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_RenderTarget", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_Environment_64", "(MinLODSize=128,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_Environment_128", "(MinLODSize=256,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_Environment_256", "(MinLODSize=512,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_Environment_512", "(MinLODSize=1024,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_Environment_1024", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_VFX_64", "(MinLODSize=32,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_VFX_128", "(MinLODSize=32,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_VFX_256", "(MinLODSize=32,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_VFX_512", "(MinLODSize=32,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_VFX_1024", "(MinLODSize=32,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_APL_128", "(MinLODSize=256,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_APL_256", "(MinLODSize=512,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_APL_512", "(MinLODSize=1024,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_APL_1024", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_UI", "(MinLODSize=64,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_Promotional", "(MinLODSize=256,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_Character_1024", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_Character_Diff", "(MinLODSize=512,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_Character_Norm", "(MinLODSize=512,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_Character_Spec", "(MinLODSize=512,MaxLODSize=4096,LODBias=0)", "SystemSettings");
    }
    else if (gameId == MeType::ME3_TYPE)
    {
        engineConf.Write("TEXTUREGROUP_World", "(MinLODSize=256,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_WorldSpecular", "(MinLODSize=256,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_WorldNormalMap", "(MinLODSize=256,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_AmbientLightMap", "(MinLODSize=32,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_ShadowMap", "(MinLODSize=1024,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_RenderTarget", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_Environment_64", "(MinLODSize=128,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_Environment_128", "(MinLODSize=256,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_Environment_256", "(MinLODSize=512,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_Environment_512", "(MinLODSize=1024,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_Environment_1024", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_VFX_64", "(MinLODSize=32,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_VFX_128", "(MinLODSize=32,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_VFX_256", "(MinLODSize=32,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_VFX_512", "(MinLODSize=32,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_VFX_1024", "(MinLODSize=32,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_APL_128", "(MinLODSize=256,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_APL_256", "(MinLODSize=512,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_APL_512", "(MinLODSize=1024,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_APL_1024", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_UI", "(MinLODSize=64,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_Promotional", "(MinLODSize=256,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_Character_1024", "(MinLODSize=2048,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_Character_Diff", "(MinLODSize=512,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_Character_Norm", "(MinLODSize=512,MaxLODSize=4096,LODBias=0)", "SystemSettings");
        engineConf.Write("TEXTUREGROUP_Character_Spec", "(MinLODSize=512,MaxLODSize=4096,LODBias=0)", "SystemSettings");
    }
    else
    {
        CRASH();
    }
}

void LODSettings::removeLOD(MeType gameId, ConfigIni &engineConf)
{
    if (gameId == MeType::ME1_TYPE)
    {
        engineConf.Write("TEXTUREGROUP_World", "(MinLODSize=16,MaxLODSize=4096,LODBias=2)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_WorldNormalMap", "(MinLODSize=16,MaxLODSize=4096,LODBias=2)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_AmbientLightMap", "(MinLODSize=32,MaxLODSize=512,LODBias=0)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_LightAndShadowMap", "(MinLODSize=256,MaxLODSize=4096,LODBias=0)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_Environment_64", "(MinLODSize=32,MaxLODSize=64,LODBias=0)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_Environment_128", "(MinLODSize=32,MaxLODSize=128,LODBias=0)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_Environment_256", "(MinLODSize=32,MaxLODSize=256,LODBias=0)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_Environment_512", "(MinLODSize=32,MaxLODSize=512,LODBias=0)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_Environment_1024", "(MinLODSize=32,MaxLODSize=1024,LODBias=0)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_VFX_64", "(MinLODSize=8,MaxLODSize=64,LODBias=0)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_VFX_128", "(MinLODSize=8,MaxLODSize=128,LODBias=0)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_VFX_256", "(MinLODSize=8,MaxLODSize=256,LODBias=0)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_VFX_512", "(MinLODSize=8,MaxLODSize=512,LODBias=0)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_VFX_1024", "(MinLODSize=8,MaxLODSize=1024,LODBias=0)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_APL_128", "(MinLODSize=32,MaxLODSize=128,LODBias=0)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_APL_256", "(MinLODSize=32,MaxLODSize=256,LODBias=0)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_APL_512", "(MinLODSize=32,MaxLODSize=512,LODBias=0)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_APL_1024", "(MinLODSize=32,MaxLODSize=1024,LODBias=0)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_GUI", "(MinLODSize=8,MaxLODSize=1024,LODBias=0)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_Promotional", "(MinLODSize=32,MaxLODSize=2048,LODBias=0)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_Character_1024", "(MinLODSize=32,MaxLODSize=1024,LODBias=0)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_Character_Diff", "(MinLODSize=32,MaxLODSize=512,LODBias=0)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_Character_Norm", "(MinLODSize=32,MaxLODSize=512,LODBias=0)", "TextureLODSettings");
        engineConf.Write("TEXTUREGROUP_Character_Spec", "(MinLODSize=32,MaxLODSize=256,LODBias=0)", "TextureLODSettings");
    }
    else if (gameId == MeType::ME2_TYPE)
    {
        engineConf.DeleteKey("TEXTUREGROUP_World", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_WorldNormalMap", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_AmbientLightMap", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_LightAndShadowMap", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_RenderTarget", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_Environment_64", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_Environment_128", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_Environment_256", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_Environment_512", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_Environment_1024", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_VFX_64", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_VFX_128", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_VFX_256", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_VFX_512", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_VFX_1024", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_APL_128", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_APL_256", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_APL_512", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_APL_1024", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_UI", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_Promotional", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_Character_1024", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_Character_Diff", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_Character_Norm", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_Character_Spec", "SystemSettings");
    }
    else if (gameId == MeType::ME3_TYPE)
    {
        engineConf.DeleteKey("TEXTUREGROUP_World", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_WorldSpecular", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_WorldNormalMap", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_AmbientLightMap", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_ShadowMap", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_RenderTarget", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_Environment_64", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_Environment_128", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_Environment_256", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_Environment_512", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_Environment_1024", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_VFX_64", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_VFX_128", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_VFX_256", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_VFX_512", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_VFX_1024", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_APL_128", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_APL_256", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_APL_512", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_APL_1024", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_UI", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_Promotional", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_Character_1024", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_Character_Diff", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_Character_Norm", "SystemSettings");
        engineConf.DeleteKey("TEXTUREGROUP_Character_Spec", "SystemSettings");
    }
    else
    {
        CRASH();
    }
}

void LODSettings::updateGFXSettings(MeType gameId, ConfigIni &engineConf, bool softShadowsME1, bool meuitmMode)
{
    if (gameId == MeType::ME1_TYPE)
    {
        engineConf.Write("MaxShadowResolution", "2048", "Engine.Engine");
        engineConf.Write("MaxShadowResolution", "2048", "Engine.GameEngine");
        if (softShadowsME1)
        {
            engineConf.Write("MinShadowResolution", "16", "Engine.Engine");
            engineConf.Write("MinShadowResolution", "16", "Engine.GameEngine");
        }
        else
        {
            engineConf.Write("MinShadowResolution", "64", "Engine.Engine");
            engineConf.Write("MinShadowResolution", "64", "Engine.GameEngine");
        }
        engineConf.Write("DynamicShadows", "True", "SystemSettings");
        engineConf.Write("EnableDynamicShadows", "True", "WinDrv.WindowsClient");
        if (softShadowsME1 && meuitmMode)
        {
            engineConf.Write("DepthBias", "0.006000", "Engine.Engine");
            engineConf.Write("DepthBias", "0.006000", "Engine.GameEngine");
        }
        else
        {
            engineConf.Write("DepthBias", "0.030000", "Engine.Engine");
            engineConf.Write("DepthBias", "0.030000", "Engine.GameEngine");
        }
        engineConf.Write("ShadowFilterQualityBias", "2", "SystemSettings");
        if (softShadowsME1)
        {
            engineConf.Write("ShadowFilterRadius", "2", "Engine.Engine");
            engineConf.Write("ShadowFilterRadius", "2", "Engine.GameEngine");
        }
        else
        {
            engineConf.Write("ShadowFilterRadius", "4", "Engine.Engine");
            engineConf.Write("ShadowFilterRadius", "4", "Engine.GameEngine");
        }
        engineConf.Write("bEnableBranchingPCFShadows", "True", "Engine.Engine");
        engineConf.Write("bEnableBranchingPCFShadows", "True", "Engine.GameEngine");
        engineConf.Write("MaxAnisotropy", "16", "SystemSettings");
        engineConf.Write("TextureLODLevel", "3", "WinDrv.WindowsClient");
        engineConf.Write("FilterLevel", "2", "WinDrv.WindowsClient");
        engineConf.Write("Trilinear", "True", "SystemSettings");
        engineConf.Write("MotionBlur", "True", "SystemSettings");
        engineConf.Write("DepthOfField", "True", "SystemSettings");
        engineConf.Write("Bloom", "True", "SystemSettings");
        engineConf.Write("QualityBloom", "True", "SystemSettings");
        engineConf.Write("ParticleLODBias", "-1", "SystemSettings");
        engineConf.Write("SkeletalMeshLODBias", "-1", "SystemSettings");
        engineConf.Write("DetailMode", "2", "SystemSettings");
        engineConf.Write("PoolSize", "1536", "TextureStreaming");
        engineConf.Write("MinTimeToGuaranteeMinMipCount", "0", "TextureStreaming");
        engineConf.Write("MaxTimeToGuaranteeMinMipCount", "0", "TextureStreaming");
    }
    else if (gameId == MeType::ME2_TYPE)
    {
        engineConf.Write("MaxShadowResolution", "2048", "SystemSettings");
        engineConf.Write("MinShadowResolution", "64", "SystemSettings");
        engineConf.Write("ShadowFilterQualityBias", "2", "SystemSettings");
        engineConf.Write("ShadowFilterRadius", "4", "SystemSettings");
        engineConf.Write("bEnableBranchingPCFShadows", "True", "SystemSettings");
        engineConf.Write("MaxAnisotropy", "16", "SystemSettings");
        engineConf.Write("Trilinear", "True", "SystemSettings");
        engineConf.Write("MotionBlur", "True", "SystemSettings");
        engineConf.Write("DepthOfField", "True", "SystemSettings");
        engineConf.Write("Bloom", "True", "SystemSettings");
        engineConf.Write("QualityBloom", "True", "SystemSettings");
        engineConf.Write("ParticleLODBias", "-1", "SystemSettings");
        engineConf.Write("SkeletalMeshLODBias", "-1", "SystemSettings");
        engineConf.Write("DetailMode", "2", "SystemSettings");
    }
    else if (gameId == MeType::ME3_TYPE)
    {
        engineConf.Write("MaxShadowResolution", "2048", "SystemSettings");
        engineConf.Write("MinShadowResolution", "64", "SystemSettings");
        engineConf.Write("ShadowFilterQualityBias", "2", "SystemSettings");
        engineConf.Write("ShadowFilterRadius", "4", "SystemSettings");
        engineConf.Write("bEnableBranchingPCFShadows", "True", "SystemSettings");
        engineConf.Write("MaxAnisotropy", "16", "SystemSettings");
        engineConf.Write("MotionBlur", "True", "SystemSettings");
        engineConf.Write("DepthOfField", "True", "SystemSettings");
        engineConf.Write("Bloom", "True", "SystemSettings");
        engineConf.Write("QualityBloom", "True", "SystemSettings");
        engineConf.Write("ParticleLODBias", "-1", "SystemSettings");
        engineConf.Write("SkeletalMeshLODBias", "-1", "SystemSettings");
        engineConf.Write("DetailMode", "2", "SystemSettings");
    }
    else
    {
        CRASH();
    }
}
