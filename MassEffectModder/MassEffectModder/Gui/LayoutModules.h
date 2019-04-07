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

#ifndef LAYOUTMODULES_H
#define LAYOUTMODULES_H

class MainWindow;

class LayoutModules: public QWidget
{
    Q_OBJECT

public:
    explicit LayoutModules(QWidget *parent = nullptr,
                           QStackedLayout *layout = nullptr,
                           MainWindow *window = nullptr);

private slots:
    void TextureManagerSelected();
    void TextureUtilitiesSelected();
    void GameUtilitiesSelected();
    void ModsManagerSelected();
    void ReturnSelected();

private:
    const int kButtonMinWidth = 300;
    const int kButtonMinHeight = 100;

    QStackedLayout *stackedLayout;
    MainWindow *mainWindow;
};

#endif // LAYOUTMODULES_H
