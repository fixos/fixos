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

#ifndef _DEVICE_TTY_H
#define _DEVICE_TTY_H

#include <sys/tty.h>
#include <device/device.h>


/**
 * TTY device driver, which centralized each TTY implementation inside a
 * single device (each TTY has the same major, and different minors).
 * The generic TTY implementation is defined in sys/tty.h
 */


// number of TTY minors handle by the TTY device
#define TTYDEV_MINOR_MAX	10


extern struct device ttydev_device;

/**
 * Get/set a TTY from its minor number inside the TTY device.
 * ttydev_set_minor() returns non-zero if queried minor is outside
 * [0, TTYDEV_MINOR_MAX[ range.
 */
struct tty *ttydev_get_minor(uint16 minor);

int ttydev_set_minor(uint16 minor, struct tty *tty);





#endif //_DEVICE_TTY_H
