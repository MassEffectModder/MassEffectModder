/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2019 Pawel Kolodziejski
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

#include <Program/ConfigIni.h>
#include <Helpers/Exception.h>
#include <Helpers/FileStream.h>

#if defined(_WIN32)
#include <windows.h>
#endif

ConfigIni::ConfigIni()
    : ConfigIni("")
{
}

ConfigIni::ConfigIni(const QString &iniPath, bool mode, bool unix)
{
    simpleMode = mode;
    unixEndlines = unix;

    if (iniPath.length() != 0)
    {
        if (mode)
        {
            settings = nullptr;
            _iniPath = iniPath;
            auto inputFile = new QFile(_iniPath);
            if (!inputFile->open(QIODevice::ReadOnly | QIODevice::Text))
                return;
            while (!inputFile->atEnd())
            {
                QString line = inputFile->readLine();
                line.remove(QRegExp("[\\n\\r]"));
                list.append(line);
            }
            delete inputFile;
            valid = true;
        }
        else
        {
#if defined(_WIN32)
            settings = nullptr;
            _iniPath = iniPath;
#else
            settings = new QSettings(iniPath, QSettings::IniFormat);
            if (settings->status() == QSettings::NoError)
                valid = true;
#endif
        }
    }
    else
    {
        if (mode)
            CRASH();
        QString path = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation).first() +
                "/MassEffectModder";
        QDir().mkpath(path);
#if defined(_WIN32)
        settings = nullptr;
        _iniPath = path + "/MassEffectModder.ini";
#else
        settings = new QSettings(path + "/MassEffectModder.ini", QSettings::IniFormat);
        if (settings->status() == QSettings::NoError)
            valid = true;
#endif
    }
}

QString ConfigIni::Read(const QString &key, const QString &section)
{
    if (!valid)
        return "";

    if (simpleMode)
    {
        QString sectionPattern = QString("[" + section + "]");
        int indexOfSection = -1;

        for (int i = 0; i < list.count(); i++)
        {
            QString line = list[i].trimmed();
            if (line.compare(sectionPattern, Qt::CaseInsensitive) == 0)
            {
                indexOfSection = i;
                break;
            }
        }

        if (indexOfSection != -1)
        {
            for (int k = indexOfSection + 1; k < list.count(); k++)
            {
                QString line = list[k];
                if (line.count() == 0 || line.startsWith("//") || line.startsWith(';'))
                    continue;
                if (line.startsWith('[') && line.endsWith(']'))
                    return "";
                int pos = line.indexOf('=');
                if (pos <= 0)
                    continue;

                auto entry = line.split('=');
                if (entry[0].compare(key, Qt::CaseInsensitive) != 0)
                    continue;
                if (entry.count() == 1)
                    return "";
                return line.mid(pos + 1);
            }
        }
        return "";
    }
    else
    {
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
}

bool ConfigIni::Write(const QString &key, const QString &value, const QString &section)
{
    if (!valid || key.length() == 0 || section.length() == 0)
        return false;

    if (simpleMode)
    {
        QString sectionPattern = QString("[" + section + "]");
        int indexOfSection = -1;
        int indexInSection = -1;

        for (int i = 0; i < list.count(); i++)
        {
            QString line = list[i].trimmed();
            if (line.compare(sectionPattern, Qt::CaseInsensitive) == 0)
            {
                indexOfSection = i;
                break;
            }
        }

        if (indexOfSection == -1)
        {
            if (!list.isEmpty() && list.last() != "")
                list.append("");
            list.append(sectionPattern);
            list.append(key + "=" + value);
        }
        else
        {
            bool replaced = false;
            indexInSection = indexOfSection;
            for (int k = indexInSection + 1; k < list.count(); k++)
            {
                QString line = list[k];
                if (line.count() == 0 || line.startsWith("//") || line.startsWith(';'))
                    continue;
                if (line.startsWith('[') && line.endsWith(']'))
                    break;
                int pos = line.indexOf('=');
                if (pos <= 0)
                    continue;

                indexInSection = k;
                auto entry = line.split('=');
                if (entry[0].compare(key, Qt::CaseInsensitive) != 0)
                    continue;
                list.replace(k, entry[0] + "=" + value);
                replaced = true;
                break;
            }
            if (!replaced)
                list.insert(indexInSection + 1, key + "=" + value);
        }

        auto outputFile = new QFile(_iniPath);
        if (!outputFile->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
            return false;
        delete outputFile;

        FileStream *file = new FileStream(_iniPath, FileMode::Create, FileAccess::WriteOnly);
        for (int i = 0; i < list.count(); i++)
        {
            QString line = list[i];
            if (unixEndlines)
                line.append("\n");
            else
                line.append("\r\n");
            file->WriteStringASCII(line);
        }
        delete file;

        return true;
    }
    else
    {
#if !defined(_WIN32)
        if (!settings->isWritable())
            return false;
#endif

#if defined(_WIN32)
        return WritePrivateProfileString(section.toStdWString().c_str(),
                                         key.toStdWString().c_str(),
                                         value.toStdWString().c_str(),
                                         _iniPath.toStdWString().c_str());
#else
        settings->setValue(section + "/" + key, value);
        settings->sync();
#endif
    }

    return true;
}

bool ConfigIni::DeleteKey(const QString &key, const QString &section)
{
    if (!valid || key.length() == 0 || section.length() == 0)
        return false;

    if (simpleMode)
    {
        QString sectionPattern = QString("[" + section + "]");
        int indexOfSection = -1;
        int indexInSection = -1;

        for (int i = 0; i < list.count(); i++)
        {
            QString line = list[i].trimmed();
            if (line.compare(sectionPattern, Qt::CaseInsensitive) == 0)
            {
                indexOfSection = i;
                break;
            }
        }

        if (indexOfSection == -1)
            return true;

        bool modified = false;
        indexInSection = indexOfSection;
        for (int k = indexInSection + 1; k < list.count(); k++)
        {
            QString line = list[k];
            if (line.count() == 0 || line.startsWith("//") || line.startsWith(';'))
                continue;
            if (line.startsWith('[') && line.endsWith(']'))
                return true;
            int pos = line.indexOf('=');
            if (pos <= 0)
                continue;

            indexInSection = k;
            auto entry = line.split('=');
            if (entry[0].compare(key, Qt::CaseInsensitive) != 0)
                continue;
            list.removeAt(k);
            modified = true;
            break;
        }

        if (!modified)
            return true;

        auto outputFile = new QFile(_iniPath);
        if (!outputFile->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
            return false;
        delete outputFile;

        FileStream *file = new FileStream(_iniPath, FileMode::Create, FileAccess::WriteOnly);
        for (int i = 0; i < list.count(); i++)
        {
            QString line = list[i];
            if (unixEndlines)
                line.append("\n");
            else
                line.append("\r\n");
            file->WriteStringASCII(line);
        }
        delete file;

        return true;
    }
    else
    {
#if !defined(_WIN32)
        if (!settings->isWritable())
            return false;
#endif

#if defined(_WIN32)
        return WritePrivateProfileString(section.toStdWString().c_str(),
                                         key.toStdWString().c_str(),
                                         nullptr,
                                         _iniPath.toStdWString().c_str());
#else
        settings->remove(section + "/" + key);
        settings->sync();
#endif
    }

    return true;
}
