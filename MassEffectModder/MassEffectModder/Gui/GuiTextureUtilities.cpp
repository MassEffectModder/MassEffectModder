/*
 * MassEffectModder
 *
 * Copyright (C) 2019-2022 Pawel Kolodziejski
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
#include <GameData/UserSettings.h>
#include <Misc/Misc.h>

void LayoutMain::RemoveScanFileSelected(MeType gameType)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(STR_REMOVING_TEXTURE_MAP_TITLE);
    msgBox.setText("You are about to delete your current textures scan file.");
    msgBox.setInformativeText(QString("After that, and before scanning your game again, ") +
                              "you need to restore game to a vanilla state and reinstall any DLC mods." +
                              "\n\nAre you sure you want to proceed?");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Abort);
    msgBox.setDefaultButton(QMessageBox::Abort);
    int result = msgBox.exec();
    if (result == QMessageBox::Yes)
    {
        QString path = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation).first() +
                "/MassEffectModder";
        QString filename = path + QString("/mele%1map.bin").arg(static_cast<int>(gameType));
        if (QFile(filename).exists())
        {
            QFile(filename).remove();
            QMessageBox::information(this, STR_REMOVING_TEXTURE_MAP_TITLE,
                    QString("File at ") + filename + " deleted.");
        }
        else
        {
            QMessageBox::information(this,  STR_REMOVING_TEXTURE_MAP_TITLE,
                    QString("File at ") + filename + " not found.");
        }
    }
}

void LayoutMain::ApplyHQGfx(MeType gameType)
{
    ConfigIni configIni{};
    g_GameData->Init(gameType, configIni);
    QString path = g_GameData->EngineConfigIniPath(gameType);
#if defined(_WIN32)
    ConfigIni engineConf = ConfigIni(path);
#else
    ConfigIni engineConf = ConfigIni(path, true);
#endif
    UserSettings::updateGFXSettings(gameType, engineConf);

    QMessageBox::information(this, "Applying HQ gfx settings.",
            QString("Game configuration file at ") + path + " updated.");
}

void LayoutMain::ApplyHQGfxSelected(MeType gameType)
{
    ApplyHQGfx(gameType);
}
