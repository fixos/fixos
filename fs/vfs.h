/*
 * Copyright (C) 2012-2015 Leo Grange <grangeleo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef FIXOS_VFS_H
#define FIXOS_VFS_H

/**
  * The VFS, or Virtual FS, is a set of functions that allow the
  * file acces with an important abstraction (the real location
  * of physical data, the filesystem used...), and provide a way
  * to manage file systems (mount and unmount) in a unified interface.
  *
  * All the physicals FS use a common interface, defined in file_system.h,
  * and are mounted into a struct fs_instance.
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
 * Try to allocate a struct inode object into the VFS cache.
 * The FS instance and internal node id are required for the
 * caching system, and should never change during the life of
 * this inode.
 * Also, these values *must* be a unique pair in the entiere VFS!
 * Return its address, or NULL if alloc fails.
 */
struct inode *vfs_alloc_inode(struct fs_instance *inst, uint32 node);

/**
 * Release one 'instance' of use (decrease the 'count' from 1).
 * If count is decreased to 0, the inode may be freed.
 */
void vfs_release_inode(struct inode *inode);


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
struct inode *vfs_get_inode(struct fs_instance *inst, uint32 nodeid);


struct inode *vfs_first_child(struct inode *target);

struct inode *vfs_next_sibling(struct inode *target);


/**
 * Register a file system into VFS.
 * Flag should be VFS_REGISTER_AUTO!
 */
void vfs_register_fs(const struct file_system *fs, int flags);


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
struct inode *vfs_resolve(const char *path);


/**
 * Resolve one entry from a given inode.
 * Return the inode if found, NULL otherwise.
 */
struct inode *vfs_walk_entry(struct inode *parent, const char *name);

/**
 * Try to resolve a TYPE_MOUNTPOINT or a TYPE_ROOT inode.
 * If the given inode is a mountpoint, its corresponding root is returned.
 * If it's a root inode, its mountpoint is returned.
 * Else, or if a root inode is the VFS root, return NULL.
 */
struct inode *vfs_resolve_mount(struct inode *inode);


#endif // FIXOS_VFS_H
