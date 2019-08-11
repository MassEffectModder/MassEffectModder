/*
 * MassEffectModder
 *
 * Copyright (C) 2017-2019 Pawel Kolodziejski
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

#if defined __cplusplus

#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>
#include <mutex>
#include <memory>

#include <QtGlobal>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDirIterator>
#include <QSettings>
#include <QStandardPaths>
#include <QDateTime>
#include <QSysInfo>
#include <QString>
#include <QList>
#include <QStringList>
#include <QCryptographicHash>
#include <QTextStream>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QMap>
#include <QStorageInfo>
#include <QRegularExpression>
#include <QElapsedTimer>
#include <QUuid>
#ifdef GUI
#include <QApplication>
#include <QWidget>
#include <QMainWindow>
#include <QFormLayout>
#include <QStackedLayout>
#include <QToolBar>
#include <QStatusBar>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#endif

#include <omp.h>

#include "Helpers/Exception.h"

extern bool g_ipc;

#endif

