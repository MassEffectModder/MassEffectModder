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

#include "ConfigIni.h"

#if defined(_WIN32)
#include <windows.h>
#endif

ConfigIni::ConfigIni()
    : ConfigIni("")
{
}

ConfigIni::ConfigIni(const QString &iniPath)
{
    if (iniPath.length() != 0)
    {
#if defined(_WIN32)
        settings = nullptr;
        _iniPath = iniPath;
#else
        settings = new QSettings(iniPath, QSettings::IniFormat);
#endif
    }
    else
    {
        QString path = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation).first() +
                "/MassEffectModder";
        QDir().mkpath(path);
#if defined(_WIN32)
        settings = nullptr;
        _iniPath = path + "/MassEffectModder.ini";
#else
        settings = new QSettings(path + "/MassEffectModder.ini", QSettings::IniFormat);
#endif
    }
#if !defined(_WIN32)
    if (settings->status() == QSettings::NoError)
#endif
        valid = true;
}

QString ConfigIni::Read(const QString &key, const QString &section)
{
    if (!valid)
        return "";

#if defined(_WIN32)
    wchar_t str[256];
    GetPrivateProfileString(section.toStdWString().c_str(),
                            key.toStdWString().c_str(), nullptr, str, 256,
                            _iniPath.toStdWString().c_str());
    return QString::fromWCharArray(str);
#else
    settings->value(section + "/" + key, "");
    return settings->value(section + "/" + key, "").toString();
#endif
}

bool ConfigIni::Write(const QString &key, const QString &value, const QString &section)
{
    if (!valid || key.length() == 0 || section.length() == 0
#if !defined(_WIN32)
            || !settings->isWritable()
#endif
       )
    {
        return false;
    }

#if defined(_WIN32)
    return WritePrivateProfileString(section.toStdWString().c_str(),
                                     key.toStdWString().c_str(),
                                     value.toStdWString().c_str(),
                                     _iniPath.toStdWString().c_str());
#else
    settings->setValue(section + "/" + key, value);
    settings->sync();
#endif

    return true;
}

bool ConfigIni::DeleteKey(const QString &key, const QString &section)
{
    if (!valid || key.length() == 0 || section.length() == 0
#if !defined(_WIN32)
            || !settings->isWritable()
#endif
       )
    {
        return false;
    }

#if defined(_WIN32)
    return WritePrivateProfileString(section.toStdWString().c_str(),
                                     key.toStdWString().c_str(),
                                     nullptr,
                                     _iniPath.toStdWString().c_str());
#else
    settings->remove(section + "/" + key);
    settings->sync();
#endif

    return true;
}
