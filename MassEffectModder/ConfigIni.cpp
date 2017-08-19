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

#include "ConfigIni.h"

#include <QSettings>
#include <QStandardPaths>
#include <QDir>

ConfigIni::ConfigIni()
    : ConfigIni("")
{
}

ConfigIni::ConfigIni(const QString &iniPath)
{
    if (iniPath != "")
    {
        settings = new QSettings(iniPath, QSettings::IniFormat);
    }
    else
    {
        QString path = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation).first() +
                QDir::separator() + "MassEffectModder";
        if (!QDir(path).exists())
            QDir(path).mkpath(path);
        settings = new QSettings(path + QDir::separator() + "MassEffectModder.ini",
                                 QSettings::IniFormat);
    }
    if (settings->status() == QSettings::NoError)
        valid = true;
}

QString ConfigIni::Read(const QString &key, const QString &section)
{
    if (!valid)
        return "";

    return settings->value(section + "/" + key, "").toString();
}

bool ConfigIni::Write(const QString &key, const QString &value, const QString &section)
{
    if (!valid || key == "" || section == "" || !settings->isWritable())
        return false;

    settings->setValue(section + "/" + key, value);
    settings->sync();

    return true;
}

bool ConfigIni::DeleteKey(const QString &key, const QString &section)
{
    if (!valid || key == "" || section == "" || !settings->isWritable())
        return false;

    settings->remove(section + "/" + key);
    settings->sync();

    return true;
}
