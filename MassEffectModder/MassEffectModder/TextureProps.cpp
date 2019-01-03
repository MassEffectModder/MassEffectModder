/*
 * MassEffectModder
 *
 * Copyright (C) 2018 Pawel Kolodziejski <aquadran at users.sourceforge.net>
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

#include "Exceptions/SignalHandler.h"
#include "Helpers/MiscHelpers.h"
#include "Helpers/Logs.h"
#include "Wrappers.h"

#include "TextureProps.h"
#include "ConfigIni.h"
#include "GameData.h"
#include "MemTypes.h"

TexProperty::TexProperty(Package &pkg, const ByteBuffer &data)
{
    package = &pkg;
    headerData = *reinterpret_cast<quint32 *>(data.ptr());
    getProperty(data.ptr(), 4);
}

TexProperty::~TexProperty()
{
    for (int i = 0; i < texPropertyList.count(); i++)
    {
        texPropertyList[i].valueRaw.Free();
        texPropertyList[i].valueStruct.Free();
    }
}

void TexProperty::getProperty(quint8 *data, int offset)
{
    TexPropertyEntry texProperty{};
    int size, valueRawPos, nextOffset;

    texProperty.name = package->getName(*reinterpret_cast<quint32 *>(data + offset));
    if (texProperty.name == "None")
    {
        nextOffset = offset;
        propertyEndOffset = valueRawPos = offset + 8;
        size = 0;
    }
    else
    {
        texProperty.type = package->getName(*reinterpret_cast<qint32 *>(data + offset + 8));
        size = *reinterpret_cast<qint32 *>(data + offset + 16);
        texProperty.index = *reinterpret_cast<qint32 *>(data + offset + 20);

        valueRawPos = offset + 24;

        if (texProperty.type == "IntProperty" ||
            texProperty.type == "StrProperty" ||
            texProperty.type == "FloatProperty" ||
            texProperty.type == "NameProperty")
        {
        }
        else if (texProperty.type == "StructProperty")
        {
            size += 8;
        }
        else if (texProperty.type == "ByteProperty")
        {
            if (GameData::gameType == MeType::ME3_TYPE)
                size += 8;
        }
        else if (texProperty.type == "BoolProperty")
        {
            if (GameData::gameType == MeType::ME3_TYPE)
                size = 1;
            else
                size = 4;
        }
        else
            CRASH("");

        nextOffset = valueRawPos + size;
    }
    texProperty.valueRaw = ByteBuffer(size);
    texProperty.valueStruct = ByteBuffer();
    texProperty.fetched = false;
    memcpy(texProperty.valueRaw.ptr(), data + valueRawPos, size);
    texPropertyList.push_back(texProperty);

    if (nextOffset != offset)
        getProperty(data, nextOffset);
}

TexProperty::TexPropertyEntry TexProperty::getProperty(const QString &name)
{
    fetchValue(name);
    for (int i = 0; i < texPropertyList.count(); i++)
    {
        if (texPropertyList[i].name == name)
        {
            return texPropertyList[i];
        }
    }
    CRASH("");
}

void TexProperty::fetchValue(const QString &name)
{
    for (int i = 0; i < texPropertyList.count(); i++)
    {
        if (texPropertyList[i].name == name)
        {
            fetchValue(i);
            return;
        }
    }
}

void TexProperty::fetchValue(int index)
{
    if (index < 0 || index >= texPropertyList.count())
        CRASH("");
    TexPropertyEntry texProperty = texPropertyList[index];
    if (texProperty.fetched || texProperty.type == "None")
        return;
    if (texProperty.type == "IntProperty")
    {
        texProperty.valueInt = *reinterpret_cast<qint32 *>(texProperty.valueRaw.ptr() + 0);
    }
    else if (texProperty.type == "ByteProperty")
    {
        if (GameData::gameType == MeType::ME3_TYPE)
        {
            texProperty.valueNameType = package->getName(*reinterpret_cast<qint32 *>(texProperty.valueRaw.ptr() + 0));
            texProperty.valueName = package->getName(*reinterpret_cast<qint32 *>(texProperty.valueRaw.ptr() + 8));
            texProperty.valueInt = *reinterpret_cast<qint32 *>(texProperty.valueRaw.ptr() + 12);
        }
        else
        {
            texProperty.valueName = package->getName(*reinterpret_cast<qint32 *>(texProperty.valueRaw.ptr() + 0));
            texProperty.valueInt = *reinterpret_cast<qint32 *>(texProperty.valueRaw.ptr()  + 4);
        }
    }
    else if (texProperty.type == "BoolProperty")
    {
        texProperty.valueBool = texProperty.valueRaw.ptr()[0] != 0;
    }
    else if (texProperty.type == "StrProperty")
    {
    }
    else if (texProperty.type == "FloatProperty")
    {
        texProperty.valueFloat = *reinterpret_cast<float *>(texProperty.valueRaw.ptr() + 0);
    }
    else if (texProperty.type == "NameProperty")
    {
        texProperty.valueName = package->getName(*reinterpret_cast<qint32 *>(texProperty.valueRaw.ptr() + 0));
        texProperty.valueInt = *reinterpret_cast<qint32 *>(texProperty.valueRaw.ptr() + 4);
    }
    else if (texProperty.type == "StructProperty")
    {
        texProperty.valueName = package->getName(*reinterpret_cast<qint32 *>(texProperty.valueRaw.ptr() + 0));
        texProperty.valueInt = *reinterpret_cast<qint32 *>(texProperty.valueRaw.ptr() + 4);
        texProperty.valueStruct = ByteBuffer(texProperty.valueRaw.size() - 8);
        memcpy(texProperty.valueStruct.ptr(), texProperty.valueRaw.ptr() + 8, texProperty.valueStruct.size());
    }
    else
        CRASH("");

    texProperty.fetched = true;
    texPropertyList[index] = texProperty;
}

QString TexProperty::getDisplayString(int index)
{
    QString result = "";
    if (index < 0 || index >= texPropertyList.count())
        CRASH("");

    fetchValue(index);
    TexPropertyEntry texProperty = texPropertyList[index];
    if (texProperty.type == "None")
        return result;

    result = "  " + texProperty.name + ": ";
    if (texProperty.type == "IntProperty")
    {
        result += QString::number(texProperty.valueInt) + "\n";
    }
    else if (texProperty.type == "ByteProperty")
    {
        if (GameData::gameType == MeType::ME3_TYPE)
            result += texProperty.valueNameType + ": ";
        result += texProperty.valueName + ": ";
        result += QString::number(texProperty.valueInt) + "\n";
    }
    else if (texProperty.type == "BoolProperty")
    {
        result += QString(texProperty.valueBool ? "true" : "false") + "\n";
    }
    else if (texProperty.type == "StrProperty")
    {
        result += "\n";
    }
    else if (texProperty.type == "FloatProperty")
    {
        result += QString::number(texProperty.valueFloat) + "\n";
    }
    else if (texProperty.type == "NameProperty")
    {
        result += texProperty.valueName + ": ";
        result += QString::number(texProperty.valueInt) + "\n";
    }
    else if (texProperty.type == "StructProperty")
    {
        result += texProperty.valueName + ": ";
        result += QString::number(texProperty.valueInt) + "\n";
    }
    else
        CRASH("");

    return result;
}

bool TexProperty::exists(const QString &name)
{
    for (int i = 0; i < texPropertyList.count(); i++)
    {
        if (texPropertyList[i].name == name)
            return true;
    }
    return false;
}

void TexProperty::removeProperty(const QString &name)
{
    for (int i = 0; i < texPropertyList.count(); i++)
    {
        if (texPropertyList[i].name == name)
        {
            texPropertyList.removeAt(i);
            return;
        }
    }
}

void TexProperty::setIntValue(const QString &name, qint32 value)
{
    TexPropertyEntry texProperty{};
    if (exists(name))
    {
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                texProperty = texPropertyList[i];
                break;
            }
        }
        if (texProperty.type != "IntProperty")
            CRASH("");
    }
    else
    {
        texProperty.valueRaw = ByteBuffer(sizeof(qint32));
        texProperty.type = "IntProperty";
        if (!package->existsNameId(texProperty.type))
            package->addName(texProperty.type);
    }

    if (!package->existsNameId(name))
        package->addName(name);
    texProperty.name = name;
    texProperty.fetched = true;

    memcpy(texProperty.valueRaw.ptr(), &value, sizeof(qint32));
    texProperty.valueInt = value;
    if (exists(name))
    {
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                texPropertyList[i] = texProperty;
                break;
            }
        }
    }
    else
        texPropertyList.push_front(texProperty);
}

void TexProperty::setFloatValue(const QString &name, float value)
{
    TexPropertyEntry texProperty{};
    if (exists(name))
    {
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                texProperty = texPropertyList[i];
                break;
            }
        }
        if (texProperty.type != "FloatProperty")
            CRASH("");
    }
    else
    {
        texProperty.valueRaw = ByteBuffer(sizeof(float));
        texProperty.type = "FloatProperty";
        if (!package->existsNameId(texProperty.type))
            package->addName(texProperty.type);
    }

    if (!package->existsNameId(name))
        package->addName(name);
    texProperty.name = name;
    texProperty.fetched = true;

    memcpy(texProperty.valueRaw.ptr(), &value, sizeof(float));
    texProperty.valueFloat = value;
    if (exists(name))
    {
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                texPropertyList[i] = texProperty;
                break;
            }
        }
    }
    else
        texPropertyList.push_front(texProperty);
}

void TexProperty::setByteValue(const QString &name, const QString &valueName,
                               const QString &valueNameType, qint32 valueInt)
{
    TexPropertyEntry texProperty{};
    if (exists(name))
    {
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                texProperty = texPropertyList[i];
                break;
            }
        }
        if (texProperty.type != "ByteProperty")
            CRASH("");
    }
    else
    {
        if (GameData::gameType == MeType::ME3_TYPE)
        {
            texProperty.valueRaw = ByteBuffer(16);
        }
        else
        {
            texProperty.valueRaw = ByteBuffer(8);
        }
        texProperty.type = "ByteProperty";
        if (!package->existsNameId(texProperty.type))
            package->addName(texProperty.type);
    }

    if (!package->existsNameId(name))
        package->addName(name);
    texProperty.name = name;
    texProperty.fetched = true;

    if (!package->existsNameId(valueName))
        package->addName(valueName);
    if (GameData::gameType == MeType::ME3_TYPE)
    {
        if (!package->existsNameId(valueNameType))
            package->addName(valueNameType);
        qint32 nameTypeId = package->getNameId(valueNameType);
        qint32 nameId = package->getNameId(valueName);
        memcpy(texProperty.valueRaw.ptr(), &nameTypeId, sizeof(qint32));
        memcpy(texProperty.valueRaw.ptr() + 8, &nameId, sizeof(qint32));
        memcpy(texProperty.valueRaw.ptr() + 12, &valueInt, sizeof(qint32));
    }
    else
    {
        qint32 nameId = package->getNameId(valueName);
        memcpy(texProperty.valueRaw.ptr() + 0, &nameId, sizeof(qint32));
        memcpy(texProperty.valueRaw.ptr() + 4, &valueInt, sizeof(qint32));
    }
    texProperty.valueName = valueName;
    texProperty.valueInt = valueInt;
    if (exists(name))
    {
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                texPropertyList[i] = texProperty;
                break;
            }
        }
    }
    else
        texPropertyList.push_front(texProperty);
}

void TexProperty::setBoolValue(const QString &name, bool value)
{
    TexPropertyEntry texProperty{};
    if (exists(name))
    {
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                texProperty = texPropertyList[i];
                break;
            }
        }
        if (texProperty.type != "BoolProperty")
            CRASH("");
    }
    else
    {
        if (GameData::gameType == MeType::ME3_TYPE)
        {
            texProperty.valueRaw = ByteBuffer(1);
        }
        else
        {
            texProperty.valueRaw = ByteBuffer(4);
        }
        texProperty.type = "BoolProperty";
        if (!package->existsNameId(texProperty.type))
            package->addName(texProperty.type);
    }

    if (!package->existsNameId(name))
        package->addName(name);
    texProperty.name = name;
    texProperty.fetched = true;

    if (value)
        texProperty.valueRaw.ptr()[0] = 1;
    else
        texProperty.valueRaw.ptr()[0] = 0;
    texProperty.valueBool = value;

    if (exists(name))
    {
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                texPropertyList[i] = texProperty;
                break;
            }
        }
    }
    else
        texPropertyList.push_front(texProperty);
}

void TexProperty::setNameValue(const QString &name, const QString &valueName, qint32 valueInt)
{
    TexPropertyEntry texProperty{};
    if (exists(name))
    {
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                texProperty = texPropertyList[i];
                break;
            }
        }
        if (texProperty.type != "NameProperty")
            CRASH("");
    }
    else
    {
        texProperty.valueRaw = ByteBuffer(8);
        texProperty.type = "NameProperty";
        if (!package->existsNameId(texProperty.type))
            package->addName(texProperty.type);
    }

    if (!package->existsNameId(name))
        package->addName(name);
    texProperty.name = name;
    texProperty.fetched = true;

    if (!package->existsNameId(valueName))
        package->addName(valueName);

    qint32 nameId = package->getNameId(valueName);
    memcpy(texProperty.valueRaw.ptr(), &nameId, sizeof(qint32));
    memcpy(texProperty.valueRaw.ptr() + 4, &valueInt, sizeof(qint32));
    texProperty.valueName = valueName;
    texProperty.valueInt = valueInt;

    if (exists(name))
    {
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                texPropertyList[i] = texProperty;
                break;
            }
        }
    }
    else
        texPropertyList.push_front(texProperty);
}

void TexProperty::setStructValue(const QString &name, const QString &valueName, ByteBuffer valueStruct)
{
    TexPropertyEntry texProperty{};
    if (exists(name))
    {
        int index = -1;
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                index = i;
                break;
            }
        }
        fetchValue(index);
        texProperty = texPropertyList[index];
        if (texProperty.type != "StructProperty" || texProperty.valueStruct.size() != valueStruct.size())
            CRASH("");
    }
    else
    {
        // missing implementation
        CRASH("");
    }

    if (!package->existsNameId(name))
        package->addName(name);
    texProperty.name = name;
    texProperty.fetched = true;

    if (!package->existsNameId(valueName))
        package->addName(valueName);

    qint32 nameId = package->getNameId(valueName);
    memcpy(texProperty.valueRaw.ptr(), &nameId, sizeof(qint32));
    memcpy(texProperty.valueRaw.ptr() + 8, valueStruct.ptr(), valueStruct.size());
    texProperty.valueName = valueName;
    memcpy(texProperty.valueStruct.ptr(), valueStruct.ptr(), valueStruct.size());

    if (exists(name))
    {
        for (int i = 0; i < texPropertyList.count(); i++)
        {
            if (texPropertyList[i].name == name)
            {
                texPropertyList[i] = texProperty;
                break;
            }
        }
    }
    else
        texPropertyList.push_front(texProperty);
}

ByteBuffer TexProperty::toArray()
{
    MemoryStream mem;
    mem.WriteUInt32(headerData);
    for (int i = 0; i < texPropertyList.count(); i++)
    {
        mem.WriteInt32(package->getNameId(texPropertyList[i].name));
        mem.WriteInt32(0); // skip
        if (texPropertyList[i].name == "None")
            break;
        mem.WriteInt32(package->getNameId(texPropertyList[i].type));
        mem.WriteInt32(0); // skip
        int size = texPropertyList[i].valueRaw.size();
        if (texPropertyList[i].type == "StructProperty")
        {
            size -= 8;
        }
        else if (texPropertyList[i].type == "ByteProperty")
        {
            if (GameData::gameType == MeType::ME3_TYPE)
                size -= 8;
        }
        else if (texPropertyList[i].type == "BoolProperty")
        {
            size = 0;
        }
        mem.WriteInt32(size);
        mem.WriteInt32(texPropertyList[i].index);
        mem.WriteFromBuffer(texPropertyList[i].valueRaw.ptr(), texPropertyList[i].valueRaw.size());
    }

    return mem.ToArray();
}
