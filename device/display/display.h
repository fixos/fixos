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

#ifndef _DEVICE_DISPLAY_DISPLAY_H
#define _DEVICE_DISPLAY_DISPLAY_H

/**
 * Display device, to allow direct screen access in userland.
 * For now this device only support monochrome screen, but it is designed
 * to be able to handle any display type in the future.
 *
 * The main way to control this device is through ioctl (see interface/display.h).
 */

#include <fs/file.h>
#include <device/device.h>


#define DISPLAY_DEFAULT_MINOR	1

extern const struct device _display_device;


void display_init();

int display_open(uint16 minor, struct file *filep);

int display_release(struct file *filep);

int display_ioctl(struct file *filep, int cmd, void *data);

#endif //_DEVICE_DISPLAY_DISPLAY_H
