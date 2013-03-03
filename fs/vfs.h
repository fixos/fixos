#ifndef FIXOS_VFS_H
#define FIXOS_VFS_H

/**
  * The VFS, or Virtual FS, is a set of functions that allow the
  * file acces with an important abstraction (the real location
  * of physical data, the filesystem used...), and provide a way
  * to manage file systems (mount and unmount) in a unified interface.
  *
  * All the physicals FS use a common interface, defined in file_system.h,
  * and are mounted into a fs_instance_t.
  */

#include <utils/types.h>
#include "inode.h"
#include "fs_instance.h"
#include "file_system.h"

/**
// seek constants
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

// open mode constants
#define MODE_READ	1
#define MODE_WRITE	2

#define VFS_MAX_FILES		5
*/

// flags for vfs_mount
#define VFS_MOUNT_NORMAL	0
// mount the given fs as the root of the system
#define VFS_MOUNT_ROOT		1


// for now, VFS has a static allocation file system list
#define VFS_MAX_FS			4

// static flag for vfs_register_fs()
#define VFS_REGISTER_STATIC	1
#define VFS_REGISTER_AUTO	1

/*
 * Initialize the VFS.
 */
void vfs_init();

/**
 * Try to allocate a inode_t object.
 * Return its address, or NULL if alloc fails.
 */
inode_t *vfs_alloc_inode();

/**
 * Free an inode previously allocated by vfs_alloc_inode().
 */
void vfs_free_inode(inode_t *inode);


/**
 * Register a file system into VFS.
 * Flag should be VFS_REGISTER_AUTO!
 */
void vfs_register_fs(file_system_t *fs, int flags);


/**
 * Mount a file system on the given path.
 * If VFS_MOUNT_ROOT is used, path is not used and the mount point
 * is the logical root "/" of the system.
 * Return 0 in success case, -1 else.
 */
int vfs_mount(const char *fsname, const char *path, int flags);

/**
 * Try to resolve the given path.
 * Return the allocated inode if file/directory exists, NULL else.
 * WARNING : only absolute path are allowed, but '..' and '.' entries
 * are accepted (respectively the parent dir and the dir itself)
 */
inode_t *vfs_resolve(const char *path);



/**
  * Open a file if exists, at the given pathname.
  * flags is the OR combination of MODE_READ, MODE_WRITE...
  * The returned value is the "fileid", or a negative value if error.
  */
//int open(const char *pathname, int flags);


/**
  * Close a file designed by fileid.
  * The total number of file opened by the kernel at the same time is limited
  * for now, so it's really important to close the files properly.
  * If the fileid not exists, do nothing.
  */
//void close(int fileid);


/**
  * Write a byte array of 'size' bytes into a file.
  * Return the number of bytes written.
  * In case of error (file not oppened, in readonly mode, etc...) return
  * a negative value.
  */
//size_t write(int fileid, const void *buffer, size_t size);


/**
  * Read up to 'size' bytes from a file.
  * Return the number of bytes read, or a negative value if an error occurs.
  */
//size_t read(int fileid, void *buffer, size_t size);


/**
  * Reposition the offset of a file.
  * 'whence' is one of the macros SEEK_*
  * Return the new absolute offset location in bytes, or -1 in error.
  */
//size_t seek(int fileid, size_t offset, int whence);

#endif // FIXOS_VFS_H
