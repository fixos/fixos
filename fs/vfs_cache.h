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

struct _vfs_cache_entry {
	inode_t inode;

	// next entry in the corresponding entry's hash list 
	struct _vfs_cache_entry *next;
};

typedef struct _vfs_cache_entry vfs_cache_entry_t;


/**
 * Initialize the cache
 */
void vfs_cache_init();

/**
 * Find an entry by its fs instance and its internal id.
 * Return NULL if the entry is not in cache.
 */
vfs_cache_entry_t *vfs_cache_find(fs_instance_t *inst, uint32 nodeid);


/**
 * Allocate a new inode entry in the cache.
 */
vfs_cache_entry_t *vfs_cache_alloc(fs_instance_t *inst, uint32 nodeid);


/**
 * Remove an entry in the cache.
 */
void vfs_cache_remove(fs_instance_t *inst, uint32 nodeid);



#endif //_FS_VFS_CACHE_H
