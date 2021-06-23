/*
 * MassEffectModder
 *
 * Copyright (C) 2018-2021 Pawel Kolodziejski
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

#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <Helpers/FileStream.h>
#include <Helpers/MemoryStream.h>
#include <GameData/Package.h>

class Properties
{
public:

    class PropertyEntry
    {
        friend Properties;

    private:
        QString type;
        QString name;
        QString valueNameType;
        QString valueName;
        qint32 valueInt;
        float valueFloat;
        bool valueBool;
        ByteBuffer valueRaw;
        ByteBuffer valueStruct;
        //QList<PropertyEntry> valueStruct;
        //QList<PropertyEntry> valueArray;
        int index;
        bool fetched;

    public:

        QString getValueNameType() { return valueNameType; }
        QString getValueName() { return valueName; }
        qint32 getValueInt() { return valueInt; }
        float setValueFloat() { return valueFloat; }
        bool getValueBool() { return valueBool; }
        ByteBuffer getValueStruct() { return valueStruct; }

        void setIntValue(qint32 value);
        void setFloatValue(float value);
        void setByteValue(const QString &valueName, const QString &valueNameType, qint32 valueInt = 0);
        void setBoolValue(bool value);
        void setNameValue(const QString &valueName);
        void setStrValue(const QString &valueName);
        void setStructValue(const QString &valueName, ByteBuffer valueStruct);
    };

    QList<PropertyEntry> propertyList;
    int propertyEndOffset{};

private:
    uint headerData = 0;
    Package *package;
    void getProperty(quint8 *data, int offset);

public:

    Properties(Package &pkg, const ByteBuffer &data, int propertyOffset);
    ~Properties();
    PropertyEntry getProperty(const QString &name);
    void fetchValue(const QString &name);
    void fetchValue(int index);
    QString getDisplayString(int index);
    bool exists(const QString &name);
    void removeProperty(const QString &name);
    void setIntValue(const QString &name, qint32 value);
    void setFloatValue(const QString &name, float value);
    void setByteValue(const QString &name, const QString &valueName, const QString &valueNameType, qint32 valueInt = 0);
    void setBoolValue(const QString &name, bool value);
    void setNameValue(const QString &name, const QString &valueName);
    void setStrValue(const QString &name, const QString &valueName);
    void setStructValue(const QString &name, const QString &valueName, ByteBuffer valueStruct);
    ByteBuffer toArray();
};

#endif
