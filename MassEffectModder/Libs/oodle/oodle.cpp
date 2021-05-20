/*
 * MassEffectModder
 *
 * Copyright (C) 2021 Pawel Kolodziejski
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

#include "oodle.h"

#ifdef WIN32

bool OodleLoadLib(const wchar_t *libPath)
{
    return true;
}

void OodleUnloadLib()
{
}

#endif

int OodleCompressData(unsigned char *src, unsigned int srcLen,
                      unsigned char *dst, unsigned int *dstLen)
{
    return -1;
}

int OodleDecompressData(unsigned char *src, unsigned int srcLen,
                        unsigned char *dst, unsigned int dstLen)
{
    return -1;
}
