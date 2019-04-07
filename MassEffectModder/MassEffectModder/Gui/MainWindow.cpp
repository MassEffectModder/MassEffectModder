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

#include "Gui/MainWindow.h"
#include "Gui/LayoutMeSelect.h"
#include "Gui/LayoutModules.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      toolBar(new QToolBar(this)),
      statusBar(new QStatusBar(this))
{
    toolBar->hide();
    addToolBar(toolBar);
    setStatusBar(statusBar);
    setWindowTitle(QString("Mass Effect Modder v%1").arg(MEM_VERSION));
    setMinimumSize(kMinWindowWidth, kMinWindowHeight);

    auto *widget = new QWidget;
    setCentralWidget(widget);
    stackedLayout = new QStackedLayout(widget);
    new LayoutMeSelect(widget, stackedLayout, this);
    new LayoutModules(widget, stackedLayout, this);
}

void MainWindow::closeEvent(QCloseEvent */*event*/)
{
    // TODO: handle some confirmation
}
