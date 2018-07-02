/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2018 Pawel Kolodziejski <aquadran at users.sourceforge.net>
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

// C includes

#if defined __cplusplus

// C++ includes

#include <stdlib.h>
#include <pthread.h>
#include <string>

[[ noreturn ]] void Exception(const char *file, const char *func, int line, const char *msg = nullptr);
#define CRASH_MSG(msg) Exception(__FILE__, __PRETTY_FUNCTION__, __LINE__, msg)
#define CRASH(msg) Exception(__FILE__, __PRETTY_FUNCTION__, __LINE__)

#include <QtGlobal>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <QDirIterator>
#include <QSettings>
#include <QStandardPaths>
#include <QDateTime>
#include <QSysInfo>
#include <QString>
#include <QList>
#include <QStringList>

#endif

