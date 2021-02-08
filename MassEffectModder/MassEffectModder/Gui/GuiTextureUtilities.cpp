/*
 * MassEffectModder
 *
 * Copyright (C) 2019-2021 Pawel Kolodziejski
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

#include <Gui/LayoutMain.h>
#include <Gui/MainWindow.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <Program/ConfigIni.h>
#include <GameData/GameData.h>
#include <GameData/LODSettings.h>
#include <Misc/Misc.h>

void LayoutMain::RemoveScanFileSelected(MeType gameType)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Remove textures map of the game.");
    msgBox.setText("You are going to delete your current textures scan file.");
    msgBox.setInformativeText(QString("After that, and before scanning your game again, ") +
                              "you need to restore game to vanilla state and reinstall vanilla DLCs and DLC mods." +
                              "\n\nAre you sure you want to proceed?");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Abort);
    msgBox.setDefaultButton(QMessageBox::Abort);
    int result = msgBox.exec();
    if (result == QMessageBox::Yes)
    {
        QString path = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation).first() +
                "/MassEffectModder";
        QString filename = path + QString("/me%1map.bin").arg(static_cast<int>(gameType));
        if (QFile(filename).exists())
        {
            QFile(filename).remove();
            QMessageBox::information(this, "Remove textures map of the game.",
                    QString("File at ") + filename + " deleted.");
        }
        else
        {
            QMessageBox::information(this, "Remove textures map of the game.",
                    QString("File at ") + filename + " not found.");
        }
    }
}

void LayoutMain::ApplyLODs(MeType gameType, bool lods2k)
{
    if (GameData::ConfigIniPath(gameType).length() == 0)
    {
        QMessageBox::critical(this, "Appling HQ LODs settings.",
                              "Game User path is not defined.");
        return;
    }
    QString path = GameData::EngineConfigIniPath(gameType);
    QDir().mkpath(DirName(path));
#if defined(_WIN32)
    ConfigIni engineConf = ConfigIni(path);
#else
    ConfigIni engineConf = ConfigIni(path, true);
#endif
    LODSettings::updateLOD(gameType, engineConf, lods2k);

    QMessageBox::information(this, "Appling HQ LODs settings.",
            QString("Game configuration file at ") + path + " updated.");
}

void LayoutMain::ApplyHQLODsSelected(MeType gameType)
{
    ApplyLODs(gameType, false);
}

void LayoutMain::Apply2kLODsSelected()
{
    ApplyLODs(MeType::ME1_TYPE, true);
}

void LayoutMain::ApplyVanillaLODsSelected(MeType gameType)
{
    if (GameData::ConfigIniPath(gameType).length() == 0)
    {
        QMessageBox::critical(this, "Appling vanilla LODs settings.",
                              "Game User path is not defined.");
        return;
    }
    QString path = GameData::EngineConfigIniPath(gameType);
    QDir().mkpath(DirName(path));
#if defined(_WIN32)
    ConfigIni engineConf = ConfigIni(path);
#else
    ConfigIni engineConf = ConfigIni(path, true);
#endif
    LODSettings::removeLOD(gameType, engineConf);

    QMessageBox::information(this, "Appling vanilla LODs settings.",
            QString("Game configuration file at ") + path + " updated.");
}

void LayoutMain::ApplyHQGfx(MeType gameType, bool softShadows)
{
    if (GameData::ConfigIniPath(gameType).length() == 0)
    {
        QMessageBox::critical(this, "Appling HQ gfx settings.",
                              "Game User path is not defined.");
        return;
    }
    QString path = GameData::EngineConfigIniPath(gameType);
    QDir().mkpath(DirName(path));
#if defined(_WIN32)
    ConfigIni engineConf = ConfigIni(path);
#else
    ConfigIni engineConf = ConfigIni(path, true);
#endif
    LODSettings::updateGFXSettings(gameType, engineConf, softShadows, false);

    QMessageBox::information(this, "Appling HQ gfx settings.",
            QString("Game configuration file at ") + path + " updated.");
}

void LayoutMain::ApplyHQGfxSelected(MeType gameType)
{
    ApplyHQGfx(gameType, false);
}

void LayoutMain::ApplyHQGfxSoftShadowsSelected()
{
    ApplyHQGfx(MeType::ME1_TYPE, true);
}
