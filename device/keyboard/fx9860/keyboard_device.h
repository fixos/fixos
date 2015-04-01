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

#ifndef _DEVICE_KEYBOARD_FX9860_KEYBOARD_DEVICE
#define _DEVICE_KEYBOARD_FX9860_KEYBOARD_DEVICE

#include <device/device.h>
#include <fs/file.h>
#include <interface/fixos/fxkeyboard.h>


// minor device ID for the direct keyboard access
#define FXKDB_DIRECT_MINOR		0


// maximum key event retained in the buffer
#define FXKDB_MAX_KEYS			20

extern const struct device fxkeyboard_device;


/**
 * This function should be called to insert a key event in the buffer.
 * Returns 0 if inserted, -1 if no space is available (event is ignored).
 */
int fxkdb_insert_event(const struct fxkey_event *event);

void fxkdb_init();

int fxkdb_open(uint16 minor, struct file *filep);

int fxkdb_release(struct file *filep);

ssize_t fxkdb_read(struct file *filep, void *dest, size_t len);


#endif //_DEVICE_KEYBOARD_FX9860_KEYBOARD_DEVICE
