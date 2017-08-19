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

#include <QtWidgets>
#include <QStackedLayout>

#include "Gui/MainWindow.h"
#include "Gui/LayoutMeSelect.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      toolBar(new QToolBar(this)),
      statusBar(new QStatusBar(this))
{
    toolBar->hide();
    addToolBar(toolBar);
    setStatusBar(statusBar);
    setWindowTitle("Mass Effect Modder");
    setMinimumSize(kMinWindowWidth, kMinWindowHeight);

    QWidget *widget = new QWidget;
    setCentralWidget(widget);
    stackedLayout = new QStackedLayout(widget);

    LayoutMeSelect *layoutMeSelect = new LayoutMeSelect(widget);
    stackedLayout->addWidget(layoutMeSelect);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // WA for macOS bug:
    // https://bugreports.qt.io/browse/QTBUG-43344
    static bool closed = false;
    if (!closed)
    {
        // TODO: handle some confirmation
        event->accept();
        closed = true;
    }
}
