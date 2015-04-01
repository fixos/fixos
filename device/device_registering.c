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

#include "device_registering.h"


// entry of register vector, not used if dev == NULL
struct device_entry {
	const struct device *dev;
	uint16 major;
};

static struct device_entry _registered_devices[DEVICE_REGISTER_MAX];


void dev_init() {
	int i;
	
	for(i=0; i<DEVICE_REGISTER_MAX; i++)
		_registered_devices[i].dev = NULL;
}


int dev_register_device(const struct device *dev, uint16 major_id) {
	int i;

	// get the first free entry
	for(i=0; i<DEVICE_REGISTER_MAX && _registered_devices[i].dev != NULL; i++);

	if(i<DEVICE_REGISTER_MAX) {
		_registered_devices[i].dev = dev;
		_registered_devices[i].major = major_id;
		return 0;
	}

	return -1;
}


const struct device* dev_device_from_major(uint16 major_id) {
	int i;

	for(i=0; i<DEVICE_REGISTER_MAX; i++) {
		if(_registered_devices[i].dev != NULL && _registered_devices[i].major == major_id)
			return _registered_devices[i].dev;
	}

	return NULL;
}


void dev_unregister_device(uint16 major_id) {
	int i;

	// do for every entry (not only he first)
	for(i=0; i<DEVICE_REGISTER_MAX; i++) {
		if(_registered_devices[i].major == major_id)
			_registered_devices[i].dev = NULL;
	}
}


