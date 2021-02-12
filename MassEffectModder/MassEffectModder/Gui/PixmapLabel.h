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

#ifndef PIXMAPLABEL_H
#define PIXMAPLABEL_H

#include <Types/MemTypes.h>

class PixmapLabel : public QLabel
{
    Q_OBJECT

private:
    QPixmap pixmapImage;

public slots:
    void setPixmap(const QPixmap &p);

protected:
    void resizeEvent(QResizeEvent *event) override;

public:
    explicit PixmapLabel(QWidget *parent = nullptr);

    [[nodiscard]] QPixmap resizePixmap() const;
    [[nodiscard]] int heightForWidth(int w) const override;
};

#endif // PIXMAPLABEL_H
