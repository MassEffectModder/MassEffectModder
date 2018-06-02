/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2018 Pawel Kolodziejski <aquadran at users.sourceforge.net>
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

#ifndef CONFIG_INI_H
#define CONFIG_INI_H

#include <QString>

class QSettings;

class ConfigIni
{
    QSettings *settings;
    bool valid = false;

public:
    explicit ConfigIni();
    ConfigIni(const QString &iniPath);

    bool IsConfigFileValid() { return valid; }
    QString Read(const QString &key, const QString &section);
    bool Write(const QString &key, const QString &value, const QString &section);
    bool DeleteKey(const QString &key, const QString &section);
    bool DeleteSection(const QString &section);
};

#endif
