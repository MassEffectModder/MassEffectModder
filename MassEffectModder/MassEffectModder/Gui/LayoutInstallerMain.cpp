/*
 * MassEffectModder
 *
 * Copyright (C) 2020 Pawel Kolodziejski
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

#include <Gui/LayoutInstallerMain.h>
#include <Gui/InstallerWindow.h>
#include <Gui/MessageWindow.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/FileStream.h>
#include <Helpers/Logs.h>
#include <Misc/Misc.h>
#include <GameData/GameData.h>

LayoutInstallerMain::LayoutInstallerMain(InstallerWindow *window)
    : installerWindow(window), gameType(MeType::UNKNOWN_TYPE)
{
    layoutId = InstallerWindow::kLayoutInstallerMain;

}

void LayoutInstallerMain::LockGui(bool lock)
{
    foreach (QWidget *widget, this->findChildren<QWidget*>())
    {
        widget->setEnabled(!lock);
    }
    installerWindow->LockClose(lock);
}
