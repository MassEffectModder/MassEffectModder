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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#include <QString>

#include "Logs.h"

Logs::Logs() :
        _initialized(false), _startedTimestamp(0),
        _timeStampEnabled(false)
{
}

Logs::~Logs()
{
    deinit();
}

bool Logs::init()
{
    struct timeval current_time;

    if (_initialized == true)
        return true;

    if (pthread_mutex_init(&_lock, NULL) != 0)
    {
        fprintf(stderr, "Logs::init(): Failed create mutex!\n");
        return false;
    }

    gettimeofday(&current_time, nullptr);
    _startedTimestamp = ((current_time.tv_sec - 1) * 1000 + (current_time.tv_usec / 1000));

    _initialized = true;
    return true;
}

bool Logs::deinit()
{
    if (_initialized == false)
        return false;

    _initialized = false;

    if (pthread_mutex_destroy(&_lock) != 0)
        return false;

    return true;
}

#define MAX_LOG_MSG_SIZE 4000

void Logs::printf(const char *format, ...)
{
    va_list arguments;
    char message[MAX_LOG_MSG_SIZE];
    struct timeval current_time;

    if (_initialized == false)
        return;

    gettimeofday(&current_time, NULL);
    float timestamp = (current_time.tv_sec * 1000 + (current_time.tv_usec / 1000)) - _startedTimestamp;

    va_start(arguments, format);
    vsnprintf(message, MAX_LOG_MSG_SIZE - 1, format, arguments);
    va_end(arguments);

    pthread_mutex_lock(&_lock);

    if (_timeStampEnabled)
        ::printf("[%.3f] %s", timestamp / 1000, message);
    else
        ::printf("%s", message);

    FILE *file = fopen("Log.txt", "a");
    if (file)
    {
        if (_timeStampEnabled)
            fprintf(file, "[%.3f] %s", timestamp / 1000, message);
        else
            fprintf(file, "%s", message);
        fclose(file);
    }
    pthread_mutex_unlock(&_lock);
}

void Logs::print(const char *message)
{
    this->printf("%s", message);
}

void Logs::printline(const char *message)
{
    bool oldState = _timeStampEnabled;
    _timeStampEnabled = false;
    this->printf("%s\n", message);
    _timeStampEnabled = oldState;
}

void Logs::printMsgTimeStamp(QString &message)
{
    bool oldState = _timeStampEnabled;
    _timeStampEnabled = true;
    this->printf("%s\n", message.toStdString().c_str());
    _timeStampEnabled = oldState;
}

void Logs::printMsg(QString &message)
{
    bool oldState = _timeStampEnabled;
    _timeStampEnabled = false;
    this->printf("%s\n", message.toStdString().c_str());
    _timeStampEnabled = oldState;
}

Logs *g_logs;

bool CreateLogs()
{
    g_logs = new Logs();
    if (g_logs == nullptr)
    {
        fprintf(stderr, "CreateLogs: Failed create instance\n");
        return false;
    }

    return g_logs->init();
}

void ReleaseLogs()
{
    delete g_logs;
    g_logs = nullptr;
}
