/*
 * MassEffectModder
 *
 * Copyright (C) 2019 Pawel Kolodziejski
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

#ifndef LAYOUT_MODULES_H
#define LAYOUT_MODULES_H

#include <Gui/MainWindow.h>

class LayoutModules: public LayoutHandle
{
    Q_OBJECT

public:
    explicit LayoutModules(MainWindow *window = nullptr);

private slots:
    void TexturesManagerSelected();
    void TextureUtilitiesSelected();
    void GameUtilitiesSelected();
    void ModsManagerSelected();
    void ReturnSelected();

private:
    const int kButtonMinWidth = 200;
    const int kButtonMinHeight = 22;
#if defined(__APPLE__)
    const int kFontSize = 12;
#elif defined(__linux__)
    const int kFontSize = 8;
#else
    const int kFontSize = 8;
#endif

    MainWindow *mainWindow;
    QLabel     *iconLogo;
};

#endif // LAYOUT_MODULES_H
