/*
 * MassEffectModder
 *
 * Copyright (C) 2017 Pawel Kolodziejski <aquadran at users.sourceforge.net>
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

#ifndef LOGS_H
#define LOGS_H

#include <pthread.h>

class Logs
{
private:

    bool            _initialized;
    pthread_mutex_t _lock;
    float           _startedTimestamp;

public:

    Logs();
    ~Logs();
    bool init();
    bool deinit();
    bool printf(const char *format, ...);
};

extern Logs *g_log;

bool CreateLogs();

#endif
