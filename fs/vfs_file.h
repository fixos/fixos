#ifndef _FS_VFS_FILE_H
#define _FS_VFS_FILE_H

/**
 * VFS file is the high level abstraction of file manipulation.
 * This is not *really* a part of Virtual *File System*, because it doesn't
 * consider File System but only file and inodes.
 * Provide unified way to store, retrieve and manipulate struct file.
 */

#include <utils/types.h>
#include "inode.h"
#include "file.h"


/**
 * Initilize file system.
 */
void vfs_file_init();

/**
 * Allocate a new file structure.
 * Returns NULL if allocation can't be done.
 */
struct file *vfs_file_alloc();


/**
 * Free an allocated file structure.
 */
void vfs_file_free(struct file *filep);

/**
 * Try to open the given inode as a file.
 * Returns the new file data structure if success, NULL otherwise.
 */
struct file *vfs_open(inode_t *inode);


/**
 * Close an openned file (or simply release it in some case).
 */
void vfs_close(struct file *filep);


/**
 * Read data from openned file.
 */
size_t vfs_read(struct file *filep, void *dest, size_t nb);


/**
 * Write data to openned file.
 */
size_t vfs_write(struct file *filep, void *source, size_t nb);


/**
 * Seek position in file or device.
 */
off_t vfs_lseek(struct file *filep, off_t offset, int whence); 


/**
 * I/O control.
 */
int vfs_ioctl(struct file *filep, int cmd, void *data);

#endif //_FS_VFS_FILE_H
