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
int dev_register_device(struct device *dev, uint16 major_id);


/**
 * Find a registered device using its MAJOR number.
 * Returns NULL if the device is not found (not registered or removed).
 */
struct device* dev_device_from_major(uint16 major_id);


/**
 * Remove an already registered device.
 * If not registered, do nothing.
 */
void dev_unregister_device(uint16 major_id);


#endif //_DEVICE_DEVICE_REGISTERING_H
