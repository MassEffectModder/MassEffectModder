/*
 * MassEffectModder
 *
 * Copyright (C) 2018-2022 Pawel Kolodziejski
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

#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <Wrappers.h>
#include <Program/ConfigIni.h>
#include <GameData/GameData.h>
#include <GameData/Properties.h>
#include <Types/MemTypes.h>

Properties::Properties(Package &pkg, const ByteBuffer &data, int propertyOffset)
{
    package = &pkg;
    headerData = *reinterpret_cast<quint32 *>(data.ptr());
    getProperty(data.ptr(), propertyOffset);
}

Properties::~Properties()
{
    for (int i = 0; i < propertyList.count(); i++)
    {
        propertyList[i].valueRaw.Free();
        propertyList[i].valueStruct.Free();
    }
}

void Properties::getProperty(quint8 *data, int offset)
{
    PropertyEntry property{};
    int size, valueRawPos, nextOffset;

    property.name = package->getName(*reinterpret_cast<quint32 *>(data + offset));
    if (property.name == "None")
    {
        nextOffset = offset;
        propertyEndOffset = valueRawPos = offset + 8;
        size = 0;
    }
    else
    {
        property.type = package->getName(*reinterpret_cast<qint32 *>(data + offset + 8));
        size = *reinterpret_cast<qint32 *>(data + offset + 16);
        property.index = *reinterpret_cast<qint32 *>(data + offset + 20);

        valueRawPos = offset + 24;

        if (property.type == "IntProperty" ||
            property.type == "StrProperty" ||
            property.type == "FloatProperty" ||
            property.type == "NameProperty" ||
            property.type == "ObjectProperty")
        {
        }
        else if (property.type == "StructProperty" ||
                 property.type == "ByteProperty")
        {
            size += 8;
        }
        else if (property.type == "BoolProperty")
        {
            size = 1;
        }
        else
            CRASH("");

        nextOffset = valueRawPos + size;
    }
    property.valueRaw = ByteBuffer(size);
    property.valueStruct = ByteBuffer();
    property.fetched = false;
    memcpy(property.valueRaw.ptr(), data + valueRawPos, size);
    propertyList.push_back(property);

    if (nextOffset != offset)
        getProperty(data, nextOffset);
}

Properties::PropertyEntry Properties::getProperty(const QString &name)
{
    fetchValue(name);
    for (int i = 0; i < propertyList.count(); i++)
    {
        if (propertyList[i].name == name)
        {
            return propertyList[i];
        }
    }
    CRASH("");
}

void Properties::fetchValue(const QString &name)
{
    for (int i = 0; i < propertyList.count(); i++)
    {
        if (propertyList[i].name == name)
        {
            fetchValue(i);
            return;
        }
    }
}

void Properties::fetchValue(int index)
{
    if (index < 0 || index >= propertyList.count())
        CRASH("");
    PropertyEntry property = propertyList[index];
    if (property.fetched || property.name == "None")
        return;
    if (property.type == "IntProperty" ||
        property.type == "ObjectProperty")
    {
        property.valueInt = *reinterpret_cast<qint32 *>(property.valueRaw.ptr() + 0);
    }
    else if (property.type == "ByteProperty")
    {
        property.valueNameType = package->getName(*reinterpret_cast<qint32 *>(property.valueRaw.ptr() + 0));
        property.valueName = package->getName(*reinterpret_cast<qint32 *>(property.valueRaw.ptr() + 8));
        property.valueInt = *reinterpret_cast<qint32 *>(property.valueRaw.ptr() + 12);
    }
    else if (property.type == "BoolProperty")
    {
        property.valueBool = property.valueRaw.ptr()[0] != 0;
    }
    else if (property.type == "StrProperty")
    {
        qint32 len = *reinterpret_cast<qint32 *>(property.valueRaw.ptr() + 0);
        if (len < 0) // unicode
        {
            property.valueName = "";
            for (int n = 0; n < -len; n++)
            {
                quint16 c = *reinterpret_cast<qint16 *>(property.valueRaw.ptr() + 4 + n);
                property.valueName += QChar(static_cast<ushort>(c));
            }
        }
        else
        {
            for (int n = 0; n < len; n++)
            {
                quint8 c = *reinterpret_cast<qint8 *>(property.valueRaw.ptr() + 4 + n);
                property.valueName += (char)c;
            }
        }
        if (property.name.endsWith(QChar('\0')))
            property.name.chop(1);
    }
    else if (property.type == "FloatProperty")
    {
        property.valueFloat = *reinterpret_cast<float *>(property.valueRaw.ptr() + 0);
    }
    else if (property.type == "NameProperty")
    {
        property.valueName = package->getName(*reinterpret_cast<qint32 *>(property.valueRaw.ptr() + 0));
    }
    else if (property.type == "StructProperty")
    {
        property.valueName = package->getName(*reinterpret_cast<qint32 *>(property.valueRaw.ptr() + 0));
        property.valueInt = *reinterpret_cast<qint32 *>(property.valueRaw.ptr() + 4);
        property.valueStruct = ByteBuffer(property.valueRaw.size() - 8);
        memcpy(property.valueStruct.ptr(), property.valueRaw.ptr() + 8, property.valueStruct.size());
    }
    else
        CRASH("");

    property.fetched = true;
    propertyList[index] = property;
}

QString Properties::getDisplayString(int index)
{
    QString result = "";
    if (index < 0 || index >= propertyList.count())
        CRASH();

    fetchValue(index);
    PropertyEntry property = propertyList[index];
    if (property.name == "None")
        return result;

    result = "  " + property.name + ": ";
    if (property.type == "IntProperty")
    {
        result += QString::number(property.valueInt) + "\n";
    }
    else if (property.type == "ObjectProperty")
    {
        result += package->getName(package->getClassNameId(property.valueInt)) + "\n";
    }
    else if (property.type == "ByteProperty")
    {
        result += property.valueNameType + ": ";
        result += property.valueName + ": ";
        result += QString::number(property.valueInt) + "\n";
    }
    else if (property.type == "BoolProperty")
    {
        result += QString(property.valueBool ? "true" : "false") + "\n";
    }
    else if (property.type == "FloatProperty")
    {
        result += QString::number(property.valueFloat) + "\n";
    }
    else if (property.type == "NameProperty" || property.type == "StrProperty")
    {
        result += property.valueName + "\n";
    }
    else if (property.type == "StructProperty")
    {
        result += property.valueName + "\n";
    }
    else
        CRASH();

    return result;
}

bool Properties::exists(const QString &name)
{
    for (int i = 0; i < propertyList.count(); i++)
    {
        if (propertyList[i].name == name)
            return true;
    }
    return false;
}

void Properties::removeProperty(const QString &name)
{
    for (int i = 0; i < propertyList.count(); i++)
    {
        if (propertyList[i].name == name)
        {
            propertyList.removeAt(i);
            return;
        }
    }
}

void Properties::setIntValue(const QString &name, qint32 value)
{
    PropertyEntry property{};
    if (exists(name))
    {
        for (int i = 0; i < propertyList.count(); i++)
        {
            if (propertyList[i].name == name)
            {
                property = propertyList[i];
                break;
            }
        }
        if (property.type != "IntProperty")
            CRASH();
    }
    else
    {
        property.valueRaw = ByteBuffer(sizeof(qint32));
        property.type = "IntProperty";
        if (!package->existsNameId(property.type))
            package->addName(property.type);
    }

    if (!package->existsNameId(name))
        package->addName(name);
    property.name = name;
    property.fetched = true;

    memcpy(property.valueRaw.ptr(), &value, sizeof(qint32));
    property.valueInt = value;
    if (exists(name))
    {
        for (int i = 0; i < propertyList.count(); i++)
        {
            if (propertyList[i].name == name)
            {
                propertyList[i] = property;
                break;
            }
        }
    }
    else
        propertyList.push_front(property);
}

void Properties::setFloatValue(const QString &name, float value)
{
    PropertyEntry property{};
    if (exists(name))
    {
        for (int i = 0; i < propertyList.count(); i++)
        {
            if (propertyList[i].name == name)
            {
                property = propertyList[i];
                break;
            }
        }
        if (property.type != "FloatProperty")
            CRASH();
    }
    else
    {
        property.valueRaw = ByteBuffer(sizeof(float));
        property.type = "FloatProperty";
        if (!package->existsNameId(property.type))
            package->addName(property.type);
    }

    if (!package->existsNameId(name))
        package->addName(name);
    property.name = name;
    property.fetched = true;

    memcpy(property.valueRaw.ptr(), &value, sizeof(float));
    property.valueFloat = value;
    if (exists(name))
    {
        for (int i = 0; i < propertyList.count(); i++)
        {
            if (propertyList[i].name == name)
            {
                propertyList[i] = property;
                break;
            }
        }
    }
    else
        propertyList.push_front(property);
}

void Properties::setByteValue(const QString &name, const QString &valueName,
                               const QString &valueNameType, qint32 valueInt)
{
    PropertyEntry property{};
    if (exists(name))
    {
        for (int i = 0; i < propertyList.count(); i++)
        {
            if (propertyList[i].name == name)
            {
                property = propertyList[i];
                break;
            }
        }
        if (property.type != "ByteProperty")
            CRASH();
    }
    else
    {
        property.valueRaw = ByteBuffer(16);
        memset(property.valueRaw.ptr() + 4, 0, sizeof(qint32));
        property.type = "ByteProperty";
        if (!package->existsNameId(property.type))
            package->addName(property.type);
    }

    if (!package->existsNameId(name))
        package->addName(name);
    property.name = name;
    property.fetched = true;

    if (!package->existsNameId(valueName))
        package->addName(valueName);
    if (!package->existsNameId(valueNameType))
        package->addName(valueNameType);
    qint32 nameTypeId = package->getNameId(valueNameType);
    qint32 nameId = package->getNameId(valueName);
    memcpy(property.valueRaw.ptr(), &nameTypeId, sizeof(qint32));
    memcpy(property.valueRaw.ptr() + 8, &nameId, sizeof(qint32));
    memcpy(property.valueRaw.ptr() + 12, &valueInt, sizeof(qint32));
    property.valueName = valueName;
    property.valueInt = valueInt;
    if (exists(name))
    {
        for (int i = 0; i < propertyList.count(); i++)
        {
            if (propertyList[i].name == name)
            {
                propertyList[i] = property;
                break;
            }
        }
    }
    else
        propertyList.push_front(property);
}

void Properties::setBoolValue(const QString &name, bool value)
{
    PropertyEntry property{};
    if (exists(name))
    {
        for (int i = 0; i < propertyList.count(); i++)
        {
            if (propertyList[i].name == name)
            {
                property = propertyList[i];
                break;
            }
        }
        if (property.type != "BoolProperty")
            CRASH();
    }
    else
    {
        property.valueRaw = ByteBuffer(1);
        property.type = "BoolProperty";
        if (!package->existsNameId(property.type))
            package->addName(property.type);
    }

    if (!package->existsNameId(name))
        package->addName(name);
    property.name = name;
    property.fetched = true;

    if (value)
        property.valueRaw.ptr()[0] = 1;
    else
        property.valueRaw.ptr()[0] = 0;
    property.valueBool = value;

    if (exists(name))
    {
        for (int i = 0; i < propertyList.count(); i++)
        {
            if (propertyList[i].name == name)
            {
                propertyList[i] = property;
                break;
            }
        }
    }
    else
        propertyList.push_front(property);
}

void Properties::setNameValue(const QString &name, const QString &valueName)
{
    PropertyEntry property{};
    if (exists(name))
    {
        for (int i = 0; i < propertyList.count(); i++)
        {
            if (propertyList[i].name == name)
            {
                property = propertyList[i];
                break;
            }
        }
        if (property.type != "NameProperty")
            CRASH();
    }
    else
    {
        property.valueRaw = ByteBuffer(8);
        property.type = "NameProperty";
        if (!package->existsNameId(property.type))
            package->addName(property.type);
    }

    if (!package->existsNameId(name))
        package->addName(name);
    property.name = name;
    property.fetched = true;

    if (!package->existsNameId(valueName))
        package->addName(valueName);

    qint32 nameId = package->getNameId(valueName);
    memcpy(property.valueRaw.ptr(), &nameId, sizeof(qint32));
    memset(property.valueRaw.ptr() + 4, 0, sizeof(qint32));
    property.valueName = valueName;

    if (exists(name))
    {
        for (int i = 0; i < propertyList.count(); i++)
        {
            if (propertyList[i].name == name)
            {
                propertyList[i] = property;
                break;
            }
        }
    }
    else
        propertyList.push_front(property);
}

void Properties::setStrValue(const QString &name, const QString &valueName)
{
    PropertyEntry property{};
    if (exists(name))
    {
        for (int i = 0; i < propertyList.count(); i++)
        {
            if (propertyList[i].name == name)
            {
                property = propertyList[i];
                break;
            }
        }
        if (property.type != "StrProperty")
            CRASH();
    }
    else
    {
        property.type = "StrProperty";
    }

    if (!package->existsNameId(name))
        package->addName(name);
    property.name = name;
    property.fetched = true;

    qint32 len = valueName.length();
    if (len != 0)
        len += 1;
    if (GameData::gameType == MeType::ME3_TYPE)
    {
        len *= 2;
        property.valueRaw = ByteBuffer(len + 4);
        auto s = const_cast<ushort *>(valueName.utf16());
        memcpy(property.valueRaw.ptr() + 4, s, valueName.length() * 2);
        memset(property.valueRaw.ptr() + 8 + valueName.length() * 2, 0, sizeof(qint16));
        len = -len;
    }
    else
    {
        property.valueRaw = ByteBuffer(len + 4);
        std::string string = valueName.toStdString();
        auto s = const_cast<char *>(string.c_str());
        memcpy(property.valueRaw.ptr() + 4, s, valueName.length());
        memset(property.valueRaw.ptr() + 8 + valueName.length(), 0, sizeof(qint8));
    }
    memcpy(property.valueRaw.ptr(), &len, sizeof(qint32));
    property.valueName = valueName;

    if (exists(name))
    {
        for (int i = 0; i < propertyList.count(); i++)
        {
            if (propertyList[i].name == name)
            {
                propertyList[i] = property;
                break;
            }
        }
    }
    else
        propertyList.push_front(property);
}

void Properties::setStructValue(const QString &name, const QString &valueName, ByteBuffer valueStruct)
{
    PropertyEntry property{};
    if (exists(name))
    {
        int index = -1;
        for (int i = 0; i < propertyList.count(); i++)
        {
            if (propertyList[i].name == name)
            {
                index = i;
                break;
            }
        }
        fetchValue(index);
        property = propertyList[index];
        if (property.type != "StructProperty" || property.valueStruct.size() != valueStruct.size())
            CRASH();
    }
    else
    {
        property.valueRaw = ByteBuffer(valueStruct.size() + 8);
        property.valueStruct = ByteBuffer(valueStruct.size());
        property.type = "StructProperty";
        if (!package->existsNameId(property.type))
            package->addName(property.type);
    }

    if (!package->existsNameId(name))
        package->addName(name);
    property.name = name;
    property.fetched = true;
    property.valueName = valueName;
    property.valueInt = 0;

    if (!package->existsNameId(valueName))
        package->addName(valueName);

    qint32 nameId = package->getNameId(valueName);
    memcpy(property.valueRaw.ptr(), &nameId, sizeof(qint32));
    memcpy(property.valueRaw.ptr() + 4, &property.valueInt, sizeof(qint32));
    memcpy(property.valueRaw.ptr() + 8, valueStruct.ptr(), valueStruct.size());
    memcpy(property.valueStruct.ptr(), valueStruct.ptr(), valueStruct.size());

    if (exists(name))
    {
        for (int i = 0; i < propertyList.count(); i++)
        {
            if (propertyList[i].name == name)
            {
                propertyList[i] = property;
                break;
            }
        }
    }
    else
        propertyList.push_front(property);
}

ByteBuffer Properties::toArray()
{
    MemoryStream mem;
    mem.WriteUInt32(headerData);
    for (int i = 0; i < propertyList.count(); i++)
    {
        mem.WriteInt32(package->getNameId(propertyList[i].name));
        mem.WriteInt32(0); // skip
        if (propertyList[i].name == "None")
            break;
        mem.WriteInt32(package->getNameId(propertyList[i].type));
        mem.WriteInt32(0); // skip
        int size = propertyList[i].valueRaw.size();
        if (propertyList[i].type == "StructProperty" ||
            propertyList[i].type == "ByteProperty")
        {
            size -= 8;
        }
        else if (propertyList[i].type == "BoolProperty")
        {
            size = 0;
        }
        mem.WriteInt32(size);
        mem.WriteInt32(propertyList[i].index);
        mem.WriteFromBuffer(propertyList[i].valueRaw.ptr(), propertyList[i].valueRaw.size());
    }

    return mem.ToArray();
}
