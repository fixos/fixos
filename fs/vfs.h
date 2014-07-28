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

#define VFS_MAX_MOUNT		6

// static flag for vfs_register_fs()
#define VFS_REGISTER_STATIC	1
#define VFS_REGISTER_AUTO	1

/*
 * Initialize the VFS.
 */
void vfs_init();

/**
 * Try to allocate a inode_t object into the VFS cache.
 * The FS instance and internal node id are required for the
 * caching system, and should never change during the life of
 * this inode.
 * Also, these values *must* be a unique pair in the entiere VFS!
 * Return its address, or NULL if alloc fails.
 */
inode_t *vfs_alloc_inode(fs_instance_t *inst, uint32 node);

/**
 * Release one 'instance' of use (decrease the 'count' from 1).
 * If count is decreased to 0, the inode may be freed.
 */
void vfs_release_inode(inode_t *inode);


/**
 * Get the inode from its fs instance and its internal node id.
 * The inode is firstly searched in the VFS cache, and the fs-specific
 * 'get_inode' is called only if not found in the cache.
 * In all case, count is increased.
 * (implementations of file systems should use this each time they need
 * to obtain an inode, even for an entry of the same FS!)
 * Return NULL if not found in the cache and not allowed by the internal
 * fs instance.
 */
inode_t *vfs_get_inode(fs_instance_t *inst, uint32 nodeid);


inode_t *vfs_first_child(inode_t *target);

inode_t *vfs_next_sibling(inode_t *target);


/**
 * Register a file system into VFS.
 * Flag should be VFS_REGISTER_AUTO!
 */
void vfs_register_fs(const file_system_t *fs, int flags);


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
 * Resolve one entry from a given inode.
 * Return the inode if found, NULL otherwise.
 */
inode_t *vfs_walk_entry(inode_t *parent, const char *name);

/**
 * Try to resolve a TYPE_MOUNTPOINT or a TYPE_ROOT inode.
 * If the given inode is a mountpoint, its corresponding root is returned.
 * If it's a root inode, its mountpoint is returned.
 * Else, or if a root inode is the VFS root, return NULL.
 */
inode_t *vfs_resolve_mount(inode_t *inode);


#endif // FIXOS_VFS_H
