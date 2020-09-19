/*
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
 * This file was part of libmspack.
 *
 * This code was originally made available under the GNU Lesser General Public License (LGPL) version 2.1
 * (C) 2003-2018 Stuart Caie.
 *
 */

#ifndef MSPACK_SYSTEM_H
#define MSPACK_SYSTEM_H 1

#ifdef __cplusplus
extern "C" {
#endif

/* ensure config.h is read before mspack.h */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "mspack.h"
#include "macros.h"

/* assume <string.h> exists */
#ifndef MSPACK_NO_DEFAULT_SYSTEM
# include <string.h>
#else
 /* but if no default system wanted, avoid using <string.h> entirely,
  * to avoid linking to even these standard C library functions */
static inline int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *a = s1, *b = s2;
    while (n--) if (*a++ != *b++) return a[-1] - b[-1];
    return 0;
}
static inline void *memset(void *s, int c, size_t n) {
    unsigned char *s2 = s, c2 = (unsigned char) c;
    while (n--) *s2++ = c2;
    return s;
}
static inline size_t strlen(const char *s) {
    size_t c = 0; while (*s++) c++; return c;
}
#endif

/* fix for problem with GCC 4 and glibc (thanks to Ville Skytta)
 * http://bugzilla.redhat.com/bugzilla/show_bug.cgi?id=150429
 */
#ifdef read
# undef read
#endif

extern struct mspack_system *mspack_default_system;

/* returns the length of a file opened for reading */
extern int mspack_sys_filelen(struct mspack_system *system,
                              struct mspack_file *file, off_t *length);

/* validates a system structure */
extern int mspack_valid_system(struct mspack_system *sys);

#ifdef __cplusplus
}
#endif

#endif
