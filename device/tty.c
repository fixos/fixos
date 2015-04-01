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

#include <fs/file_operations.h>
#include <fs/file.h>
#include "tty.h"


static struct tty *_ttydev_minors[TTYDEV_MINOR_MAX] = {NULL};


// private definitions

static void ttydev_init();

static int ttydev_open(uint16 minor, struct file *filep);

static int ttydev_release(struct file *filep);

static ssize_t ttydev_write(struct file *filep, void *source, size_t len);

static ssize_t ttydev_read(struct file *filep, void *dest, size_t len);

static int ttydev_ioctl(struct file *filep, int cmd, void *data);

// structs for device and file manipulation
struct device ttydev_device = {
	.name = "tty",
	.init = &ttydev_init,
	.open = &ttydev_open
};


static const struct file_operations _ttydev_fop = {
	.release = &ttydev_release,
	.write = &ttydev_write,
	.read = &ttydev_read,
	.ioctl = &ttydev_ioctl
};


struct tty *ttydev_get_minor(uint16 minor) {
	return minor < TTYDEV_MINOR_MAX ? _ttydev_minors[minor] : NULL;
}

int ttydev_set_minor(uint16 minor, struct tty *tty) {
	if(minor < TTYDEV_MINOR_MAX) {
		_ttydev_minors[minor] = tty;
		return 0;
	}
	return -1;
}


static void ttydev_init() {
	
}

static int ttydev_open(uint16 minor, struct file *filep) {
	if(minor < TTYDEV_MINOR_MAX) {
		struct tty *tty = _ttydev_minors[minor];

		filep->op = &_ttydev_fop;
		filep->private_data = tty;
		return tty_open(tty);
	}
	return -ENXIO;
}


static int ttydev_release(struct file *filep) {
	tty_release(filep->private_data);
	return 0;
}

static ssize_t ttydev_write(struct file *filep, void *source, size_t len) {
	return tty_write(filep->private_data, source, len);
}

static ssize_t ttydev_read(struct file *filep, void *dest, size_t len) {
	return tty_read(filep->private_data, dest, len);
}

static int ttydev_ioctl(struct file *filep, int cmd, void *data) {
	return tty_ioctl(filep->private_data, cmd, data);
}

