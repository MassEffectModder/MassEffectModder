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

#include "Gui/MainWindow.h"
#include "Gui/LayoutMeSelect.h"
#include "Gui/LayoutModules.h"
#include "Gui/LayoutTexturesManager.h"
#include "Gui/LayoutTextureUtilities.h"
#include "Gui/LayoutGameUtilities.h"
#include "Gui/LayoutModsManager.h"
#include "Helpers/MiscHelpers.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      gameType(MeType::UNKNOWN_TYPE),
      toolBar(new QToolBar(this)),
      statusBar(new QStatusBar(this))
{
    toolBar->hide();
    addToolBar(toolBar);
    setStatusBar(statusBar);
    QString title = QString("Mass Effect Modder v%1").arg(MEM_VERSION);
    if (DetectAdminRights())
        title += " (run as Administrator)";
    setWindowTitle(title);
    setMinimumSize(kMinWindowWidth, kMinWindowHeight);

    auto *widget = new QWidget;
    setCentralWidget(widget);
    stackedLayout = new QStackedLayout(widget);
    new LayoutMeSelect(widget, stackedLayout, this);
    new LayoutModules(widget, stackedLayout, this);
    new LayoutTexturesManager(widget, stackedLayout, this);
    new LayoutTextureUtilities(widget, stackedLayout, this);
    new LayoutGameUtilities(widget, stackedLayout, this);
    new LayoutModsManager(widget, stackedLayout, this);
}

void MainWindow::closeEvent(QCloseEvent */*event*/)
{
    // TODO: handle some confirmation
}
