/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2019 Pawel Kolodziejski
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

#include "Gui/LayoutMeSelect.h"
#include "Gui/MainWindow.h"

LayoutMeSelect::LayoutMeSelect(QWidget *parent)
    : QWidget(parent)
{
    me1Button = new QPushButton("ME1", this);
    me1Button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    me1Button->setMinimumWidth(300);
    me1Button->setMinimumHeight(300);
    me1Button->setGeometry(50, 200, 300, 300);
    connect(me1Button, &QPushButton::clicked, this, &LayoutMeSelect::ME1Selected);

    me2Button = new QPushButton("ME2", this);
    me2Button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    me2Button->setMinimumWidth(300);
    me2Button->setMinimumHeight(300);
    me2Button->setGeometry(450, 200, 300, 300);
    connect(me2Button, &QPushButton::clicked, this, &LayoutMeSelect::ME2Selected);

    me3Button = new QPushButton("ME3", this);
    me3Button->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    me3Button->setMinimumWidth(300);
    me3Button->setMinimumHeight(300);
    me3Button->setGeometry(850, 200, 300, 300);
    connect(me3Button, &QPushButton::clicked, this, &LayoutMeSelect::ME3Selected);
}

void LayoutMeSelect::ME1Selected()
{
    QMessageBox::about(this, "About", "ME1");
}

void LayoutMeSelect::ME2Selected()
{
    QMessageBox::about(this, "About", "ME2");
}

void LayoutMeSelect::ME3Selected()
{
    QMessageBox::about(this, "About", "ME3");
}
