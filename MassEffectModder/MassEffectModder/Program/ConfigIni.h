/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2020 Pawel Kolodziejski
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

class QSettings;

class ConfigIni
{
    QSettings *settings;
    QString _iniPath;
    bool valid = false;
    bool simpleMode;
    bool unixEndlines;
    QStringList list;

public:
    ConfigIni();
    ~ConfigIni() { delete settings; }
    ConfigIni(const QString &iniPath, bool mode = false, bool unixMode = false);

    bool IsConfigFileValid() { return valid; }
    QString Read(const QString &key, const QString &section);
    bool Write(const QString &key, const QString &value, const QString &section);
    bool DeleteKey(const QString &key, const QString &section);
    bool DeleteSection(const QString &section);
};

#endif
