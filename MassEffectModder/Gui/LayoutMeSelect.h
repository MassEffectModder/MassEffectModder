/*
 * MassEffectModder
 *
 * Copyright (C) 2017 Pawel Kolodziejski <aquadran at users.sourceforge.net>
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

#ifndef LAYOUTMESELECT_H
#define LAYOUTMESELECT_H

#include <QWidget>

class QPushButton;
class MainWindow;

class LayoutMeSelect: public QWidget
{
    Q_OBJECT

public:
    explicit LayoutMeSelect(QWidget *parent = 0);

private slots:
    void ME1Selected();
    void ME2Selected();
    void ME3Selected();

private:
    QPushButton *me1Button;
    QPushButton *me2Button;
    QPushButton *me3Button;
};

#endif // LAYOUTMESELECT_H
