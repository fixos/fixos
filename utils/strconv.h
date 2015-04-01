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

#ifndef _UTILS_STRCONV_H
#define _UTILS_STRCONV_H

/**
 * Some functions for string representation of common types.
 */

char * strconv_int_dec(int val, char *buf);

char * strconv_int_hex(unsigned int val, char *buf);

char * strconv_ptr(void* val, char *buf);

#endif //_UTILS_STRCONV_H
