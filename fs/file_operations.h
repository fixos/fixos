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


// lseek constants
#define SEEK_SET	1
#define SEEK_CUR	2
#define SEEK_END	3


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
	 * Seek position.
	 * If the device/file do not support seeking, please set to NULL or do nothing
	 * and return the position.
	 */
	size_t (*lseek) (struct file *filep, size_t offset, int whence);
	
};

#endif //_FS_FILE_OPERATIONS_H

