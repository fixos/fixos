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

#ifndef _FIXOS_INTERFACE_TYPES_H
#define _FIXOS_INTERFACE_TYPES_H

/**
 * Common types used as well by the kernel and by the user space.
 * All definitions are protected by "__kernel_" namespace in user space.
 */

#define __KERNEL_NULL ((void*)0)

typedef unsigned int __kernel_size_t;
typedef int __kernel_ssize_t;
typedef int __kernel_off_t;


// classic plateform-independant types
typedef unsigned int	__kernel_uint32;
typedef int				__kernel_int32;
typedef unsigned short	__kernel_uint16;
typedef short			__kernel_int16;
typedef unsigned char	__kernel_uint8;
typedef char			__kernel_int8;

// Process IDendifier
typedef __kernel_int32 __kernel_pid_t;


// devices identifier and macros for major/minor decomposition
typedef __kernel_uint32 __kernel_dev_t;
#define __kernel_major(x)			((x) >> 16)
#define __kernel_minor(x)			((x) & 0xFFFF)
#define __kernel_makedev(maj, min)	( ((maj) << 16) | ((min) & 0xFFFF))

typedef __kernel_uint32 __kernel_ino_t;
typedef __kernel_uint32 __kernel_mode_t;

// time representation
typedef __kernel_uint32 __kernel_clock_t;
typedef __kernel_uint32 __kernel_time_t;

#endif //_FIXOS_INTERFACE_TYPES_H
