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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#include "Logs.h"

Logs::Logs() :
        _initialized(false), _startedTimestamp(0) {}

Logs::~Logs()
{
    deinit();
}

bool Logs::init()
{
    struct timeval current_time;

    if (_initialized == true)
    {
        printf("Logs::init(): Already initialized!\n");
        return false;
    }

    if (pthread_mutex_init(&_lock, NULL) != 0)
    {
        fprintf(stderr, "Logs::init(): Failed create mutex!\n");
        return false;
    }

    gettimeofday(&current_time, nullptr);
    _startedTimestamp = ((current_time.tv_sec - 1) * 1000 + (current_time.tv_usec / 1000));

    ::printf("- compiled at time: " __DATE__ " " __TIME__ " -\n");

    _initialized = true;
    return true;
}

bool Logs::deinit()
{
    if (_initialized == false)
    {
        fprintf(stderr, "Logs::deinit(): Already deinitialized\n");
        return false;
    }

    _initialized = false;

    if (pthread_mutex_destroy(&_lock) != 0)
    {
        fprintf(stderr, "Logs::deinit(): Failed destroy mutex\n");
        return false;
    }

    return true;
}

#define MAX_LOG_MSG_SIZE 1000

bool Logs::printf(const char *format, ...)
{
    va_list arguments;
    char message[MAX_LOG_MSG_SIZE];
    struct timeval current_time;

    if (_initialized == false)
    {
        fprintf(stderr, "Logs::print(): not initialized!\n");
        return false;
    }

    gettimeofday(&current_time, NULL);
    float timestamp = (current_time.tv_sec * 1000 + (current_time.tv_usec / 1000)) - _startedTimestamp;

    va_start(arguments, format);
    vsnprintf(message, MAX_LOG_MSG_SIZE - 1, format, arguments);
    va_end(arguments);

    printf("[%.3f] %s", timestamp / 1000, message);

    pthread_mutex_lock(&_lock);

    FILE *file = fopen("Log.txt", "a");
    if (file != NULL)
    {
        fprintf(file, "[%.3f] %s", timestamp / 1000, message);
        fclose(file);
        pthread_mutex_unlock(&_lock);
        return true;
    }
    else
    {
        pthread_mutex_unlock(&_lock);
        return false;
    }
}

Logs *g_log;

bool CreateLogs()
{
    g_log = new Logs();
    if (g_log == nullptr)
    {
        fprintf(stderr, "CreateLogs: Failed create instance\n");
        return false;
    }

    return g_log->init();
}

void ReleaseLogs()
{
    delete g_log;
    g_log = nullptr;
}
