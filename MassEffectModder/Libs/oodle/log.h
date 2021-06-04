/*
 * Copyright (C) 2017 Tavis Ormandy
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef PE_LOG_H
#define PE_LOG_H

#ifdef _WIN32
# define LogMessage(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__), fputc('\n', stderr), fflush(stderr)
#else
# ifdef NDEBUG
#  define l_debug(format...)
#  define DebugLog
# else
#  define l_debug(format...) do {               \
         pe_l_debug_(__FUNCTION__, ## format);     \
     } while (false)
#  define DebugLog l_debug
# endif

# define l_message(format...) do {              \
        pe_l_message_(__FUNCTION__, ## format);    \
    } while (false)

# define l_warning(format...) do {              \
        pe_l_warning_(__FUNCTION__, ## format);    \
    } while (false)

# define l_error(format...) do {                \
        pe_l_error_(__FUNCTION__, ## format);      \
    } while (false)

# define LogMessage(format...) do {             \
         pe_l_message_(__FUNCTION__, ## format);   \
     } while (false)
#endif

void pe_l_message_(const char *function, const char *format, ...);
void pe_l_debug_(const char *function, const char *format, ...);
void pe_l_warning_(const char *function, const char *format, ...);
void pe_l_error_(const char *function, const char *format, ...);

#endif
