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

#ifndef FIXOS_SYS_TYPES_H
#define FIXOS_SYS_TYPES_H

/**
  * This header contain the FiXos generic types definitions.
  * Even if FiXos doesn't aim to be ported on other architectures
  * that SuperH 32bit, it's really cleaner.
  */

#include <interface/fixos/types.h>

#define NULL	__KERNEL_NULL

/**
 * Types used in user space interface are re-defined here without their prefix.
 */
typedef __kernel_size_t		size_t;
typedef __kernel_ssize_t	ssize_t;
typedef __kernel_off_t		off_t;


// classic plateform-independant types
typedef __kernel_uint32		uint32;
typedef __kernel_int32		int32;
typedef __kernel_uint16 	uint16;
typedef __kernel_int16		int16;
typedef __kernel_uint8		uint8;
typedef __kernel_int8		int8;

// Process IDendifier
typedef __kernel_pid_t		pid_t;


// devices identifier and macros for major/minor decomposition
typedef __kernel_dev_t		dev_t;
#define major(x)			__kernel_major(x)
#define minor(x)			__kernel_minor(x)
#define makedev(maj, min)	__kernel_makedev(maj, min)

typedef __kernel_ino_t		ino_t;
typedef __kernel_mode_t		mode_t;

// time representation
typedef __kernel_clock_t	clock_t;
typedef __kernel_time_t		time_t;


/**
 * offsetof() compute the byte offset between the beginning of a structure
 * and the given field.
 */
#define offsetof(type, field) \
	( (unsigned int)( &((type *)0)->field) )


/**
 * container_of() is used to access to a structure from an arbitrary field
 * address (used mainly for linked list and other data set).
 */
#define container_of(element, type, field) \
	( (type *) ((void*)(element) - offsetof(type, field)) )


// special control characters (^A, ^B,... ^[ and ^?)
#define ASCII_CTRL(c) \
	((c) == '?' ? 0x7f : (c) - '@')
#define ASCII_UNCTRL(c) \
	((c) == 0x7f ? '?' : (c) + '@')

#define ASCII_IS_CTRL(c) \
	( ((unsigned char)(c) < 0x20 && (c) != '\n' && (c) != '\t') \
			|| (c) == '\x7f' \
	) 

// temporary location
// TODO move them to a more consistant place
//lseek constants
#define SEEK_SET	1
#define SEEK_CUR	2
#define SEEK_END	3

#endif // FIXOS_SYS_TYPES_H
