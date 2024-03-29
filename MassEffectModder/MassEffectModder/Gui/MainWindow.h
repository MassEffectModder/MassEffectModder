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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <Types/MemTypes.h>
#include <Gui/LayoutHandle.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT

protected:
    void            closeEvent(QCloseEvent *event) override;
    void            SwitchLayoutById(int id);

private slots:
    void            stackChanged(int index);

private:
    static const int kMinWindowWidth = 1024;
    static const int kMinWindowHeight = 600;
    static const int kLayoutMain = 1;
    static const int kLayoutTexturesManager = 2;
    static const int kLayoutInstallModsManager = 3;

    friend class LayoutMain;
    friend class LayoutTexturesManager;
    friend class LayoutInstallModsManager;

    QStackedLayout  *stackedLayout;
    bool             busy;

public:
    MainWindow();

    QStackedLayout  *GetLayout() { return stackedLayout; }
    void            LockClose(bool state) { busy = state; }
    void            SetTitle(MeType gameType, const QString &appendText);
};

#endif // MAINWINDOW_H
