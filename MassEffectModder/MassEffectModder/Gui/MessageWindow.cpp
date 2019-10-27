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

#include <Gui/MessageWindow.h>

void MessageWindow::Show(QWidget *parent, const QString &title, const QString &msg)
{
    QPlainTextEdit textWidtget;
    QVBoxLayout layout;
    QPushButton closeButton;
    if (parent != nullptr)
    {
        dialog.setBaseSize(parent->size());
        dialog.setMinimumSize(parent->size());
    }
    dialog.setWindowTitle(title);
    textWidtget.setLineWrapMode(QPlainTextEdit::NoWrap);
    textWidtget.setReadOnly(true);
    textWidtget.setPlainText(msg);
    closeButton.setText("Close");
    closeButton.setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    closeButton.setMinimumWidth(kButtonMinWidth);
    closeButton.setMinimumHeight(kButtonMinHeight / 2);
    closeButton.setAutoDefault(false);
    QFont ButtonFont = closeButton.font();
    ButtonFont.setPointSize(kFontSize);
    closeButton.setFont(ButtonFont);
    QObject::connect(&closeButton, &QPushButton::clicked, this, &MessageWindow::CloseSelected);
    layout.addWidget(&textWidtget);
    layout.addWidget(&closeButton, 0, Qt::AlignCenter);
    dialog.setLayout(&layout);
    dialog.exec();
}

void MessageWindow::CloseSelected()
{
    dialog.close();
}
