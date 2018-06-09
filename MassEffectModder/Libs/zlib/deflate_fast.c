/*
 * Fast version of the longest_match function for zlib.
 * Copyright (C) 2004-2017 Konstantin Nosov
 * For details and updates please visit
 * http://github.com/gildor2/fast_zlib
 * Licensed under the BSD license. See LICENSE.txt file in the project root for full license information.
 */

#undef ASMV
#define ASMV
#include "deflate.c"

#undef local
#define local

#include "match.h"

void match_init()
{
}
