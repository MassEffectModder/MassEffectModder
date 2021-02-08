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

#ifndef BINARY_SEARCH_H
#define BINARY_SEARCH_H

template<typename List, typename Type>
int BinarySearch(const List& list, const Type& key)
{
    int left = 0;
    int right = list.count() - 1;
    int middle;
    bool found = false;

    while (left <= right)
    {
        middle = (left + right) / 2;
        int comp = list[middle] - key;
        if (comp == -1)
            left = middle + 1;
        else
        {
            if (comp == 0)
                found = true;
            right = middle;
        }
    }

    if (found)
        return left;
    return -1;
}

#endif
