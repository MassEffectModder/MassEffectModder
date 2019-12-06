/*
 * MassEffectModder
 *
 * Copyright (C) 2018-2019 Pawel Kolodziejski
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

#include <MipMaps/MipMaps.h>
#include <GameData/Package.h>
#include <Texture/Texture.h>
#include <Image/Image.h>
#include <Wrappers.h>
#include <Helpers/FileStream.h>
#include <Helpers/MemoryStream.h>

void MipMaps::extractTextureToPng(QString &outputFile, QString &packagePath, int exportID)
{
    Package package = Package();
    package.Open(packagePath);
    Texture texture = Texture(package, exportID, package.getExportData(exportID));
    PixelFormat format = Image::getPixelFormatType(texture.getProperties().getProperty("Format").valueName);
    Texture::TextureMipMap mipmap = texture.getTopMipmap();
    ByteBuffer data = texture.getTopImageData();
    if (data.ptr() != nullptr)
    {
        if (QFile(outputFile).exists())
            QFile(outputFile).remove();
        Image::saveToPng(data.ptr(), mipmap.width, mipmap.height, format, outputFile);
        data.Free();
    }
}
