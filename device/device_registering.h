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

#ifndef _DEVICE_DEVICE_REGISTERING_H
#define _DEVICE_DEVICE_REGISTERING_H

/**
 * Device registering and access functionalities of the kernel.
 * A device is registered using dev_register_device() and an arbitrary major ID.
 * Later, a device can be retrivied by its major ID.
 */

#include <utils/types.h>
#include "device.h"

// maximum number of registered devices at the same time
#define DEVICE_REGISTER_MAX	10


/**
 * Initilize device registeration system.
 */
void dev_init();

/**
 * Try to register a new device, using major_id as MAJOR.
 * Returns 0 if the device is correctly registered, negative value else.
 */
int dev_register_device(const struct device *dev, uint16 major_id);


/**
 * Find a registered device using its MAJOR number.
 * Returns NULL if the device is not found (not registered or removed).
 */
const struct device* dev_device_from_major(uint16 major_id);


/**
 * Remove an already registered device.
 * If not registered, do nothing.
 */
void dev_unregister_device(uint16 major_id);


#endif //_DEVICE_DEVICE_REGISTERING_H
