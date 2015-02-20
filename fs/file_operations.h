#ifndef _FS_FILE_OPERATIONS_H
#define _FS_FILE_OPERATIONS_H

#include <utils/types.h>
#include "inode.h"

/**
 * Abstract "file" operations definition.
 * This interface must be used by any filesystem and device driver, to allow
 * file access througth high abstraction layer (VFS).
 * If some function have no meaning in driver or FS context, it's value may
 * be NULL.
 */


struct file;

struct mem_area;

struct file_operations {
	/**
	 * Release the file opened instance ("close" it).
	 * This can be used if some ressources must be cleaned, or specific job done.
	 */
	int (*release) (struct file *filep);

	/**
	 * Read data from file.
	 */
	ssize_t (*read) (struct file *filep, void *dest, size_t len);

	/**
	 * Write data from file.
	 */
	ssize_t (*write) (struct file *filep, void *source, size_t len);

	/**
	 * Seek position.
	 * If the device/file do not support seeking, please set to NULL or do nothing
	 * and return the position.
	 */
	off_t (*lseek) (struct file *filep, off_t offset, int whence);


	/**
	 * Input Output Control.
	 * Used mainly for devices, NULL if the device or filesystem doesn't use any
	 * specific ioctl...
	 * data is specific to command and device, may be not used.
	 */
	int (*ioctl) (struct file *filep, int cmd, void *data);


	/**
	 * Create a memory map of this object in memory.
	 * Devices may use it as they want to, for example to provide big buffers
	 * shared with userland.
	 * area should be set with all non-private fields having a valid value,
	 * which is not very well defined...
	 * At least field ops is not expected to be set, but the interface is not
	 * well designed for now.
	 * TODO either use a 'hints' argument with mem_area-like type, or define
	 * exactly what should be set and what is set by this function itself
	 *
	 * NULL if device of filesystem do not implements memory mapped areas.
	 *
	 * Return 0 if mapping is accepted, negative value else.
	 */
	int (*map_area) (struct file *filep, struct mem_area *area);
};

#endif //_FS_FILE_OPERATIONS_H

