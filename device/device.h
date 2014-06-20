#ifndef _DEVICE_DEVICE_H
#define _DEVICE_DEVICE_H

/**
 * Interface and common definitions for device abstraction layer.
 */

#include <utils/types.h>
#include <fs/file.h>


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
	 * Open the device as a file, for the given minor ID.
	 * filep is a pointer to an already allocated file structure.
	 */
	int (*open)(uint16 minor, struct file *filep);
};

#endif //_DEVICE_DEVICE_H
