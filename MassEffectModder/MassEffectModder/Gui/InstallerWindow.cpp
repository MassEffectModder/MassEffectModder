/*
 * MassEffectModder
 *
 * Copyright (C) 2020 Pawel Kolodziejski
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

#include <Gui/InstallerWindow.h>
#include <Gui/LayoutInstallerMain.h>
#include <Helpers/Exception.h>
#include <Helpers/MiscHelpers.h>

InstallerWindow::InstallerWindow()
    : busy(false)
{
    statusBar()->clearMessage();
    QString title = QString("Mass Effect Modder v%1").arg(MEM_VERSION);
    if (DetectAdminRights())
        title += " (run as Administrator)";
    setWindowTitle(title);
    setMinimumSize(kMinWindowWidth, kMinWindowHeight);

    auto widget = new QWidget;
    setCentralWidget(widget);
    stackedLayout = new QStackedLayout(widget);
    stackedLayout->addWidget(new LayoutInstallerMain(this));
    connect(stackedLayout, &QStackedLayout::currentChanged, this, &InstallerWindow::stackChanged);
}

void InstallerWindow::SwitchLayoutById(int id)
{
    for (int i = 0; i < stackedLayout->count(); i++)
    {
        auto handle = dynamic_cast<LayoutHandle *>(stackedLayout->widget(i));
        if (handle->GetLayoutId() == id)
        {
            stackedLayout->setCurrentIndex(i);
            return;
        }
    }
    CRASH();
}

void InstallerWindow::stackChanged(int index)
{
    auto handle = dynamic_cast<LayoutHandle *>(stackedLayout->widget(index));
    handle->OnStackChanged();
}

void InstallerWindow::closeEvent(QCloseEvent *event)
{
    if (busy)
    {
        QMessageBox::information(this, "Closing application", "Installer is busy.");
        event->ignore();
    }
}

void InstallerWindow::SetTitle(MeType gameType, const QString &appendText)
{
    auto title = QString("Mass Effect Modder v%1").arg(MEM_VERSION);
    if (gameType != MeType::UNKNOWN_TYPE)
        title += QString(" - ME%1").arg((int)gameType);
    if (appendText != "")
        title += " - " + appendText;
    if (DetectAdminRights())
        title += " (run as Administrator)";
    setWindowTitle(title);
}
