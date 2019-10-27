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

#ifndef MESSAGEWINDOW_H
#define MESSAGEWINDOW_H

#include <Types/MemTypes.h>

class MessageWindow : public QWidget
{
    Q_OBJECT

private:
    const int kButtonMinWidth = 300;
    const int kButtonMinHeight = 80;
#if defined(__APPLE__)
    const int kFontSize = 20;
#elif defined(__linux__)
    const int kFontSize = 13;
#else
    const int kFontSize = 15;
#endif

    QDialog         dialog;

private slots:
    void            CloseSelected();

public:
    void            Show(QWidget *parent, const QString &title, const QString &msg);
};

#endif // MESSAGEWINDOW_H
