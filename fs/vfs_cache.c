#include "vfs_cache.h"

#include <utils/log.h>
#include <utils/pool_alloc.h>


#define VFS_CACHE_HASHTABLE_SIZE	31

// hash algorithm
#define VFS_CACHE_HASH(inst,nodeid) ( ( ( (((int)(inst)>>16) ^ (int)(inst)) ^ \
		(((nodeid)>>16) ^ (nodeid)) ) & 0xFFFF)  % VFS_CACHE_HASHTABLE_SIZE)


// hash table first entries
static vfs_cache_entry_t *_hash_head[VFS_CACHE_HASHTABLE_SIZE];

// pool allocation for free inodes
static struct pool_alloc _inode_alloc = POOL_INIT(vfs_cache_entry_t);



void vfs_cache_init()
{
	int i;
	
	printk("vfs_cache: inode/page=%d\n", _inode_alloc.perpage);

	// initilize the hash table
	for(i=0; i<VFS_CACHE_HASHTABLE_SIZE; i++)
		_hash_head[i]=NULL;
}



vfs_cache_entry_t *vfs_cache_find(fs_instance_t *inst, uint32 nodeid)
{
	int hash = VFS_CACHE_HASH(inst, nodeid);
	vfs_cache_entry_t *cur = _hash_head[hash];

	while(cur != NULL && (cur->inode.fs_op != inst || cur->inode.node != nodeid))
	{
		cur = cur->next;
	}

	return cur;
}



vfs_cache_entry_t *vfs_cache_alloc(fs_instance_t *inst, uint32 nodeid)
{
	vfs_cache_entry_t *ret;
	
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


void vfs_cache_remove(fs_instance_t *inst, uint32 nodeid)
{
	
	int hash = VFS_CACHE_HASH(inst, nodeid);
	vfs_cache_entry_t *cur = _hash_head[hash];
	vfs_cache_entry_t *prev = NULL;

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
