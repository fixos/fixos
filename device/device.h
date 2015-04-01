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

#ifndef _DEVICE_DEVICE_H
#define _DEVICE_DEVICE_H

/**
 * Interface and common definitions for device abstraction layer.
 */

#include <utils/types.h>
#include <fs/file.h>


#define DEVICE_MAX_NAME 12

struct file_operations;
struct tty;

struct device {
	char name[DEVICE_MAX_NAME];

	/**
	 * Only used by TTY devices (other shoud set this field to NULL)
	 * (this design is not perfect, but for now it's a good way to handle
	 * tty used for kernel console)
	 */
	struct tty * (*get_tty)(uint16 minor);

	/**
	 * Initialize device.
	 * TODO more flexible way, allowing arguments...
	 */
	void (*init)();

	/**
	 * Open the device as a file, for the given minor ID.
	 * filep is a pointer to an already allocated file structure.
	 */
	int (*open)(uint16 minor, struct file *filep);
};

#endif //_DEVICE_DEVICE_H
