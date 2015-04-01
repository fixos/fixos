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

#include "vfs_cache.h"

#include <utils/log.h>
#include <utils/pool_alloc.h>


#define VFS_CACHE_HASHTABLE_SIZE	31

// hash algorithm
#define VFS_CACHE_HASH(inst,nodeid) ( ( ( (((int)(inst)>>16) ^ (int)(inst)) ^ \
		(((nodeid)>>16) ^ (nodeid)) ) & 0xFFFF)  % VFS_CACHE_HASHTABLE_SIZE)


// hash table first entries
static struct vfs_cache_entry *_hash_head[VFS_CACHE_HASHTABLE_SIZE];

// pool allocation for free inodes
static struct pool_alloc _inode_alloc = POOL_INIT(struct vfs_cache_entry);



void vfs_cache_init()
{
	int i;
	
	printk(LOG_DEBUG, "vfs_cache: inode/page=%d\n", _inode_alloc.perpage);

	// initilize the hash table
	for(i=0; i<VFS_CACHE_HASHTABLE_SIZE; i++)
		_hash_head[i]=NULL;
}



struct vfs_cache_entry *vfs_cache_find(struct fs_instance *inst, uint32 nodeid)
{
	int hash = VFS_CACHE_HASH(inst, nodeid);
	struct vfs_cache_entry *cur = _hash_head[hash];

	while(cur != NULL && (cur->inode.fs_op != inst || cur->inode.node != nodeid))
	{
		cur = cur->next;
	}

	return cur;
}



struct vfs_cache_entry *vfs_cache_alloc(struct fs_instance *inst, uint32 nodeid)
{
	struct vfs_cache_entry *ret;
	
	ret = pool_alloc(&_inode_alloc);
	if(ret != NULL) {
		int hash;

		// add it to the hash table
		hash = VFS_CACHE_HASH(inst, nodeid);
		ret->next = _hash_head[hash];
		_hash_head[hash] = ret;
	}
	
	return ret;
}


void vfs_cache_remove(struct fs_instance *inst, uint32 nodeid)
{
	
	int hash = VFS_CACHE_HASH(inst, nodeid);
	struct vfs_cache_entry *cur = _hash_head[hash];
	struct vfs_cache_entry *prev = NULL;

	while(cur != NULL && !(cur->inode.fs_op == inst && cur->inode.node == nodeid))
	{
		prev = cur;
		cur = cur->next;
	}

	if(cur != NULL) {
		if(prev != NULL)
			prev->next = cur->next;
		else
			_hash_head[hash] = cur->next;

		pool_free(&_inode_alloc, cur);
	}
}
