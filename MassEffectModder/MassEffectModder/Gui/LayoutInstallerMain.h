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

#ifndef LAYOUT_INSTALLER_MAIN_H
#define LAYOUT_INSTALLER_MAIN_H

#include <Gui/InstallerWindow.h>

class LayoutInstallerMain : public LayoutHandle
{
    Q_OBJECT

public:
    explicit LayoutInstallerMain(InstallerWindow *window = nullptr);

private slots:


private:
    const int kButtonMinWidth = 270;
    const int kButtonMinSmallWidth = 220;
    const int kButtonMinHeight = 22;
#if defined(__APPLE__)
    const int kFontSize = 12;
#elif defined(__linux__)
    const int kFontSize = 8;
#else
    const int kFontSize = 8;
#endif

    int                 currentMESelection;
    InstallerWindow     *installerWindow;
    MeType              gameType;


    void LockGui(bool lock);
};

#endif // LAYOUT_INSTALLER_MAIN_H
