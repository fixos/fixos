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

struct file_operations {
	/**
	 * Try to open the object designed by inode.
	 * filep is the struct allocated to store opened file informations, and
	 * is allocated and partialy set by the caller.
	 * Returns 0 if success, negative value else (so filep will be free'd).
	 */
	int (*open) (inode_t *inode, struct file *filep);

	/**
	 * Release the file opened instance ("close" it).
	 * This can be used if some ressources must be cleaned, or specific job done.
	 */
	int (*release) (struct file *filep);

	/**
	 * Read data from file.
	 */
	size_t (*read) (struct file *filep, void *dest, size_t len);

	/**
	 * Write data from file.
	 */
	size_t (*write) (struct file *filep, void *source, size_t len);

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
};

#endif //_FS_FILE_OPERATIONS_H

