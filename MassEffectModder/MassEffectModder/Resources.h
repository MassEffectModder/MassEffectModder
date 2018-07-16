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

#ifndef RESOURCES_H
#define RESOURCES_H

#include "Misc.h"
#include "Helpers/ByteBuffer.h"

class Resources
{
private:
    bool MD5tablesLoaded = false;

public:

    MD5FileEntry *entriesME1{};
    MD5FileEntry *entriesME1PL{};
    MD5FileEntry *entriesME2{};
    MD5FileEntry *entriesME3{};
    QStringList tablePkgsME1;
    QStringList tablePkgsME1PL;
    QStringList tablePkgsME2;
    QStringList tablePkgsME3;

    ~Resources() { unloadMD5Tables(); }
    void loadMD5Tables();
    void unloadMD5Tables();
    void prepareGranter();
};

#endif
