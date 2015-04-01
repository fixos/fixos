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

#ifndef _FS_VFS_CACHE_H
#define _FS_VFS_CACHE_H


/**
 * The VFS cache is currently implemented by a Hash Table (with linked
 * entries), and with a page allocation in physical memory.
 * This file contain main definitions to abstract the real algorithm
 * and to provide high level cache access.
 */

#include "inode.h"
#include "fs_instance.h"

struct vfs_cache_entry {
	struct inode inode;

	// next entry in the corresponding entry's hash list 
	struct vfs_cache_entry *next;
};


/**
 * Initialize the cache
 */
void vfs_cache_init();

/**
 * Find an entry by its fs instance and its internal id.
 * Return NULL if the entry is not in cache.
 */
struct vfs_cache_entry *vfs_cache_find(struct fs_instance *inst, uint32 nodeid);


/**
 * Allocate a new inode entry in the cache.
 */
struct vfs_cache_entry *vfs_cache_alloc(struct fs_instance *inst, uint32 nodeid);


/**
 * Remove an entry in the cache.
 */
void vfs_cache_remove(struct fs_instance *inst, uint32 nodeid);



#endif //_FS_VFS_CACHE_H
