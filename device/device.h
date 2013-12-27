#ifndef _DEVICE_DEVICE_H
#define _DEVICE_DEVICE_H

/**
 * Interface and common definitions for device abstraction layer.
 */

#include <utils/types.h>


#define DEVICE_MAX_NAME 12

struct file_operations;

struct device {
	char name[DEVICE_MAX_NAME];

	/**
	 * Initialize device.
	 * TODO more flexible way, allowing arguments...
	 */
	void (*init)();

	/**
	 * Get file_operations structure corresponding to a minor ID.
	 * If the minor doesn't exists in this device, returns NULL.
	 */
	struct file_operations* (*get_file_op)(uint16 minor);

};

#endif //_DEVICE_DEVICE_H
