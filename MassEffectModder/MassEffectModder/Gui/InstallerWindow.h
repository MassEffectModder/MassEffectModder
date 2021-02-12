/*
 * MassEffectModder
 *
 * Copyright (C) 2020-2021 Pawel Kolodziejski
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

#ifndef INSTALLER_WINDOW_H
#define INSTALLER_WINDOW_H

#include <Types/MemTypes.h>
#include <Gui/LayoutHandle.h>

class InstallerWindow : public QMainWindow
{
    Q_OBJECT

protected:
    void            closeEvent(QCloseEvent *event) override;
    void            SwitchLayoutById(int id);

private slots:
    void            stackChanged(int index);

private:
    static const int kMinWindowWidth = 1024;
    static const int kMinWindowHeight = 576;
    static const int kLayoutInstallerMain = 1;
    static const int kLayoutInstallerProcess = 2;

    friend class LayoutInstallerMain;
    friend class LayoutInstallerProgress;

    QStackedLayout  *stackedLayout;
    bool             busy;

public:
    InstallerWindow(MeType gameType);

    QStackedLayout  *GetLayout() { return stackedLayout; }
    void            LockClose(bool state) { busy = state; }
    void            SetTitle(MeType gameType, const QString &appendText);
};

#endif // INSTALLER_WINDOW_H
