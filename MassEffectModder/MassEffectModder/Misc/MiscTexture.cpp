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

#include <Misc/Misc.h>
#include <Helpers/MiscHelpers.h>
#include <Helpers/Logs.h>
#include <Resources/Resources.h>

PixelFormat Misc::changeTextureType(MeType gameId, PixelFormat gamePixelFormat, PixelFormat texturePixelFormat,
                                    TextureProperty::TextureTypes flags)
{
    if ((gamePixelFormat == PixelFormat::DXT5 || gamePixelFormat == PixelFormat::DXT1 || gamePixelFormat == PixelFormat::ATI2) &&
        (texturePixelFormat == PixelFormat::RGB || texturePixelFormat == PixelFormat::ARGB ||
         texturePixelFormat == PixelFormat::ATI2 || texturePixelFormat == PixelFormat::V8U8))
    {
        if (texturePixelFormat == PixelFormat::ARGB && flags == TextureProperty::TextureTypes::OneBitAlpha)
        {
            gamePixelFormat = PixelFormat::ARGB;
        }
        else if (texturePixelFormat == PixelFormat::ATI2 &&
            gamePixelFormat == PixelFormat::DXT1 &&
            flags == TextureProperty::TextureTypes::Normalmap)
        {
            gamePixelFormat = PixelFormat::ATI2;
        }
        else if (gameId != MeType::ME3_TYPE && texturePixelFormat == PixelFormat::ARGB &&
            flags == TextureProperty::TextureTypes::Normalmap)
        {
            gamePixelFormat = PixelFormat::ARGB;
        }
        else if ((gamePixelFormat == PixelFormat::DXT5 || gamePixelFormat == PixelFormat::DXT1) &&
            (texturePixelFormat == PixelFormat::ARGB || texturePixelFormat == PixelFormat::RGB) &&
            flags == TextureProperty::TextureTypes::Normal)
        {
            gamePixelFormat = PixelFormat::ARGB;
        }
        else if (gamePixelFormat == PixelFormat::DXT5 && texturePixelFormat == PixelFormat::ARGB &&
            gameId == MeType::ME3_TYPE &&
            flags == TextureProperty::TextureTypes::Normalmap)
        {
            gamePixelFormat = PixelFormat::ARGB;
        }
        else if ((gamePixelFormat == PixelFormat::DXT5 || gamePixelFormat == PixelFormat::DXT1) &&
            (texturePixelFormat == PixelFormat::ARGB || texturePixelFormat == PixelFormat::V8U8) &&
            gameId == MeType::ME3_TYPE &&
            flags == TextureProperty::TextureTypes::Normalmap)
        {
            gamePixelFormat = PixelFormat::V8U8;
        }
    }

    return gamePixelFormat;
}

TextureMapEntry Misc::FoundTextureInTheMap(QList<TextureMapEntry> &textures, uint crc)
{
    TextureMapEntry f{};
    for (int s = 0; s < textures.count(); s++)
    {
        if (textures[s].crc == crc)
        {
            f = textures[s];
            break;
        }
    }
    return f;
}

TextureMapEntry Misc::FoundTextureInTheInternalMap(MeType gameId, uint crc)
{
    QList<TextureMapEntry> textures;
    Resources resources;
    resources.loadMD5Tables();
    TreeScan::loadTexturesMap(gameId, resources, textures);

    TextureMapEntry f{};
    for (int s = 0; s < textures.count(); s++)
    {
        if (textures[s].crc == crc)
        {
            f = textures[s];
            break;
        }
    }
    return f;
}

uint Misc::GetCRCFromTextureMap(QList<TextureMapEntry> &textures, int exportId,
                                const QString &path)
{
    for (int k = 0; k < textures.count(); k++)
    {
        for (int t = 0; t < textures[k].list.count(); t++)
        {
            if (textures[k].list[t].path.length() == 0)
                continue;
            if (exportId == textures[k].list[t].exportID &&
                AsciiStringMatch(path, textures[k].list[t].path))
            {
                return textures[k].crc;
            }
        }
    }
    return 0;
}

bool Misc::CorrectTexture(Image &image, TextureMapEntry &f, int numMips,
                          PixelFormat newPixelFormat, const QString &file)
{
    if (!image.checkDDSHaveAllMipmaps() ||
       (numMips > 1 && image.getMipMaps().count() <= 1) ||
       (image.getPixelFormat() != newPixelFormat))
    {
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]PROCESSING_FILE Converting ") + BaseName(file));
            ConsoleSync();
        }
        else
        {
            PINFO(QString("Converting/correcting texture: ") + BaseName(file) + "\n");
        }
        bool dxt1HasAlpha = false;
        quint8 dxt1Threshold = 128;
        if (f.flags == TextureProperty::TextureTypes::OneBitAlpha)
        {
            dxt1HasAlpha = true;
            if (image.getPixelFormat() == PixelFormat::ARGB ||
                image.getPixelFormat() == PixelFormat::DXT3 ||
                image.getPixelFormat() == PixelFormat::DXT5)
            {
                PINFO(QString("Warning for texture: " ) + f.name +
                             ". This texture converted from full alpha to binary alpha.\n");
            }
        }
        image.correctMips(newPixelFormat, dxt1HasAlpha, dxt1Threshold);
        return true;
    }
    return false;
}

QString Misc::CorrectTexture(Image *image, Texture &texture,
                             PixelFormat newPixelFormat, const QString &textureName)
{
    QString errors;
    if (!image->checkDDSHaveAllMipmaps() ||
        (texture.mipMapsList.count() > 1 && image->getMipMaps().count() <= 1) ||
        (image->getPixelFormat() != newPixelFormat))
    {
        bool dxt1HasAlpha = false;
        quint8 dxt1Threshold = 128;
        if (newPixelFormat == PixelFormat::DXT1 && texture.getProperties().exists("CompressionSettings"))
        {
            if (texture.getProperties().exists("CompressionSettings") &&
                texture.getProperties().getProperty("CompressionSettings").valueName == "TC_OneBitAlpha")
            {
                dxt1HasAlpha = true;
                if (image->getPixelFormat() == PixelFormat::ARGB ||
                    image->getPixelFormat() == PixelFormat::DXT3 ||
                    image->getPixelFormat() == PixelFormat::DXT5)
                {
                    errors += "Warning for texture: " + textureName +
                              ". This texture converted from full alpha to binary alpha.\n";
                }
            }
        }
        image->correctMips(newPixelFormat, dxt1HasAlpha, dxt1Threshold);
    }
    return errors;
}

bool Misc::CheckImage(Image &image, TextureMapEntry &f, const QString &file, int index)
{
    if (image.getMipMaps().count() == 0)
    {
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]ERROR_FILE_NOT_COMPATIBLE ") + BaseName(file));
            ConsoleSync();
        }
        else
        {
            if (index == -1)
            {
                PINFO(QString("Skipping texture: ") + f.name + QString::asprintf("_0x%08X", f.crc) + "\n");
            }
            else
            {
                PERROR(QString("Skipping not compatible content, entry: ") +
                             QString::number(index + 1) + " - mod: " + BaseName(file) + "\n");
            }
        }
        return false;
    }

    if (image.getMipMaps().first()->getOrigWidth() / image.getMipMaps().first()->getOrigHeight() !=
        f.width / f.height)
    {
        if (g_ipc)
        {
            ConsoleWrite(QString("[IPC]ERROR_FILE_NOT_COMPATIBLE ") + BaseName(file));
            ConsoleSync();
        }
        else
        {
            if (index == -1)
            {
                PINFO(QString("Skipping texture: ") + f.name + QString::asprintf("_0x%08X", f.crc) + "\n");
            }
            else
            {
                PERROR(QString("Error in texture: ") + f.name + QString::asprintf("_0x%08X", f.crc) +
                    " This texture has wrong aspect ratio, skipping texture, entry: " + QString::number(index + 1) +
                    " - mod: " + BaseName(file) + "\n");
            }
        }
        return false;
    }

    return true;
}

bool Misc::CheckImage(Image &image, Texture &texture, const QString &textureName)
{
    if (image.getMipMaps().count() == 0)
    {
        PERROR(QString("Error in texture: ") + textureName + "\n");
        return false;
    }

    if (image.getMipMaps().first()->getOrigWidth() / image.getMipMaps().first()->getHeight() !=
        texture.mipMapsList.first().width / texture.mipMapsList.first().height)
    {
        PERROR(QString("Error in texture: ") + textureName +
                     " This texture has wrong aspect ratio, skipping texture...\n");
        return false;
    }

    return true;
}

int Misc::GetNumberOfMipsFromMap(TextureMapEntry &f)
{
    for (int s = 0; s < f.list.count(); s++)
    {
        if (f.list[s].path.length() != 0)
        {
            return f.list[s].numMips;
        }
    }
    return 0;
}
