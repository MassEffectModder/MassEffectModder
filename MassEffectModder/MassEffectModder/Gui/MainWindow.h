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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define PERCENT_OF_SIZE(x, y) (((x) * 100) / (y))

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

    QToolBar        *GetToolBar() { return toolBar; }
    QStatusBar      *GetStatusBar() { return statusBar; }
    QStackedLayout  *GetLayout() { return stackedLayout; }

protected:
    void            closeEvent(QCloseEvent *event) override;

private:
    static const int kMinWindowWidth = 1024;
    static const int kMinWindowHeight = 600;

    static const int kLayoutMeSelect = 0;
    static const int kLayoutModules = 1;
    static const int kLayoutTexturesManager = 2;
    static const int kLayoutTextureUtilities = 3;
    static const int kLayoutGameUtilities = 4;
    static const int kLayoutModsManager = 5;

    friend class LayoutMeSelect;
    friend class LayoutModules;
    friend class LayoutTexturesManager;
    friend class LayoutTextureUtilities;
    friend class LayoutGameUtilities;
    friend class LayoutModsManager;

    QToolBar        *toolBar;
    QStatusBar      *statusBar;
    QStackedLayout  *stackedLayout;
};

#endif // MAINWINDOW_H
