/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2022 Pawel Kolodziejski
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

#ifndef EXCEPTION_H
#define EXCEPTION_H

#ifndef NDEBUG
#if defined(__x86_64__)
#define TRAP __asm__ volatile("int $3"); exit(1);
#elif defined(__aarch64__)
#define TRAP __asm__ volatile("brk 0"); exit(1);
#endif
#else
#define TRAP
#endif

#ifdef NDEBUG
[[ noreturn ]]
#endif
void Exception(const char *file, const char *func, int line, const char *msg = nullptr);

#define CRASH_MSG(msg) { Exception(__FILE__, __PRETTY_FUNCTION__, __LINE__, msg); TRAP }
#define CRASH(msg) { Exception(__FILE__, __PRETTY_FUNCTION__, __LINE__); TRAP }

#endif
