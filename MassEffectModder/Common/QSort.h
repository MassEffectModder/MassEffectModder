/* The MIT License
 *
 * Copyright (c) 2019 Pawel Kolodziejski
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
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
