/*
 * Copyright (C) 2012-2015 Leo Grange <grangeleo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _FIXOS_STRUTILS_H
#define _FIXOS_STRUTILS_H

#include <utils/types.h>

/**
  * strutils allow to use some functions like libc' string.h functions
  * For the SH, the routines are really optimized (currently I use
  * the Newlib ones, written in assembly).
  */

void * memcpy ( void * destination, const void * source, size_t num );

void * memset ( void * ptr, int value, size_t num );

int strcmp ( const char * str1, const char * str2 );

char * strcpy ( char * destination, const char * source );

size_t strlen ( const char * str );

#endif // _FIXOS_STRUTILS_H
