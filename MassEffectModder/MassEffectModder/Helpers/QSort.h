/*
 * MassEffectModder
 *
 * Copyright (C) 2019-2021 Pawel Kolodziejski
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

#ifndef QSORT_H
#define QSORT_H

template<typename List, typename Type>
void QSort(List &list, int first, int last,
            int (*comparer)(const Type& e1, const Type& e2))
{
    int left = first;
    int right = last;
    int pivot = left;

    while (left <= right)
    {
        while (comparer(list[left], list[pivot]) < 0)
            left++;
        while (comparer(list[right], list[pivot]) > 0)
            right--;

        if (left <= right) {
            std::swap(list[left], list[right]);
            left++;
            right--;
        }
    }

    if (first < left - 1)
        QSort(list, first, left - 1, comparer);

    if (last > left)
        QSort(list, left, last, comparer);
}

#endif
