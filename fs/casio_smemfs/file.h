#ifndef _FS_SMEMFS_FILE_H
#define _FS_SMEMFS_FILE_H

#include <fs/file_operations.h>
#include <fs/file.h>

/**
 * Implementation of file operations for the Casio SMEM FS.
 */

extern struct file_operations smemfs_file_operations;


int smemfs_release (struct file *filep);

size_t smemfs_read (struct file *filep, void *dest, size_t len);

off_t smemfs_lseek (struct file *filep, off_t offset, int whence);


#endif //_FS_SMEMFS_FILE_H
