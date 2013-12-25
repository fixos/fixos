#include "vfs_cache.h"

#include <utils/log.h>
#include <sys/memory.h>


#define VFS_CACHE_HASHTABLE_SIZE	31

#define INODE_PER_PAGE (PM_PAGE_BYTES/sizeof(vfs_cache_entry_t))

// hash algorithm
#define VFS_CACHE_HASH(inst,nodeid) ( ( ( (((int)(inst)>>16) ^ (int)(inst)) ^ \
		(((nodeid)>>16) ^ (nodeid)) ) & 0xFFFF)  % VFS_CACHE_HASHTABLE_SIZE)


// hash table first entries
static vfs_cache_entry_t *_hash_head[VFS_CACHE_HASHTABLE_SIZE];

// first free entry
static vfs_cache_entry_t *_first_free;

// allocated physical page
static void *_inode_page;



void vfs_cache_init()
{
	vfs_cache_entry_t *cur;
	int i;
	
	printk("vfs_cache: inode/page=%d\n", INODE_PER_PAGE);

	// allocate 1 physical page, with cache if possible
	_inode_page = mem_pm_get_free_page(MEM_PM_CACHED);
	cur = _first_free = _inode_page;
	for(i=0; i<INODE_PER_PAGE; i++) {
		// fill the allocated page with free elements
		if(i < (INODE_PER_PAGE-1))
			cur->next = (void*)((int)cur + sizeof(vfs_cache_entry_t));
		else
			cur->next = NULL;
		cur = cur->next;
	}

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
	int hash;
	

	// take the first free inode in the list, and remove it
	ret = _first_free;
	if(ret != NULL)
		_first_free = ret->next;

	// add it to the hash table
	hash = VFS_CACHE_HASH(inst, nodeid);
	ret->next = _hash_head[hash];
	_hash_head[hash] = ret;
	
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
		cur->next = _first_free;
		_first_free = cur;
	}
}
