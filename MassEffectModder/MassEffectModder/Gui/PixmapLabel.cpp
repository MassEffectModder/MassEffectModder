/*
 * MassEffectModder
 *
 * Copyright (C) 2019-2021 Pawel Kolodziejski
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

#include <Gui/PixmapLabel.h>

PixmapLabel::PixmapLabel(QWidget *parent) :
    QLabel(parent)
{
    setMinimumSize(1, 1);
}

void PixmapLabel::setPixmap(const QPixmap &p)
{
    pixmapImage = p;
    QLabel::setPixmap(resizePixmap());
}

int PixmapLabel::heightForWidth(int w) const
{
    if (pixmapImage.isNull())
        return height();
    return ((qreal)pixmapImage.height() * w) / pixmapImage.width();
}

void PixmapLabel::resizeEvent(QResizeEvent *event)
{
    if (!pixmapImage.isNull())
        QLabel::setPixmap(resizePixmap());
    QLabel::resizeEvent(event);
}

QPixmap PixmapLabel::resizePixmap() const
{
    auto resizedPixmap = pixmapImage.scaled(size() * devicePixelRatioF(),
                                            Qt::KeepAspectRatio, Qt::SmoothTransformation);
    resizedPixmap.setDevicePixelRatio(devicePixelRatioF());
    return resizedPixmap;
}
