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

#ifndef _FIXOS_INTERFACE_DIRENT_H
#define _FIXOS_INTERFACE_DIRENT_H

/**
 * Directory entry definition, for getdents() syscall.
 * This structure is a bit special, because its real size depends of the entry
 * name length (so be kind and avoid dereferencing if possible).
 */

#include <fixos/types.h>


#define DIRENT_MAX_NAME	255

#define DIRENT_BASE_SIZE	(sizeof(__kernel_ino_t) + sizeof(__kernel_size_t) + sizeof(char))
struct fixos_dirent {
	// inode number of this entry
	__kernel_ino_t d_ino;
	// offset to the next fixos_dirent (size of this dirent with alignment)
	__kernel_size_t d_off;

	// (not implemented for now) type of this entry, or DT_UNKNOWN
	char d_type;

	// zero-terminated name string
	char d_name[DIRENT_MAX_NAME+1];
};

#define DT_UNKNOWN		0

#endif //_FIXOS_INTERFACE_DIRENT_H
