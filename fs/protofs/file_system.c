#include "file_system.h"
#include <utils/log.h>
#include <arch/sh/physical_memory.h>
#include <utils/strutils.h>
#include <fs/vfs.h>

#include "primitives.h"


struct _file_system protofs_file_system = {
	.name = "protofs",
	.mount = protofs_mount,
	.get_root_node = protofs_get_root_node,
	.get_sub_node = protofs_get_sub_node,
	.get_children_nb = NULL,
	.find_sub_node = protofs_find_sub_node,
	.get_inode = protofs_get_inode,
	.create_node = protofs_create_node
};

// 1 if SMEM FS instance is already mounted
static unsigned char _protofs_mounted = 0;

static fs_instance_t _protofs_inst = {
	.instd = NULL
};

// physical page for protofs data
static protofs_node_t* _protofs_page = NULL;

/**
 * Internal helper
 */
inode_t *protofs_fill_inode(fs_instance_t *inst, protofs_node_t *node);



fs_instance_t *protofs_mount (unsigned int flags)
{
	// TODO atomic operations...
	if(_protofs_mounted == 0) {
		int i;
		protofs_node_t *cur;
		unsigned int ppn;

		_protofs_inst.fs = &protofs_file_system;
		_protofs_mounted = 1;
		
		// prepare the physical page...
		pm_get_free_page(&ppn);
		cur = (void*)(PM_PHYSICAL_ADDR(ppn) + 0x80000000);
		_protofs_page = cur;
		
		// mark all the page as empty nodes
		for(i=0; i<PROTOFS_PER_PAGE; i++)
			cur[i].type_flags = PROTOFS_TYPE_EMPTY;

		return &_protofs_inst;
	}

	return NULL;
}



inode_t * protofs_get_root_node (fs_instance_t *inst)
{
	return protofs_get_inode(inst, PROTOFS_ROOT_NODE);
}


inode_t * protofs_get_sub_node (inode_t *target, int index)
{
	// the simplest algorithm : test all the page of nodes...
	int i;
	int found = -1;
	void *parent = PROTOFS_NODE_ADDR(target->node);
	
	if(target->node == PROTOFS_ROOT_NODE)
		parent = NULL;

	for(i=0; i<PROTOFS_PER_PAGE && found < index; i++) {
		if(!(_protofs_page[i].type_flags & PROTOFS_TYPE_EMPTY))
			if(_protofs_page[i].parent == parent)
				found++;
	}

	if(found == index)
		return protofs_fill_inode(target->fs_op, &(_protofs_page[i-1]));
	return NULL;
}


int protofs_get_children_nb (inode_t *target)
{
	// TODO remove from VFS?
	return -1;
}


inode_t * protofs_find_sub_node (inode_t *target, const char *name)
{
	// the simplest algorithm : test all the page of nodes...
	int i;
	int found = 0;
	void *parent = PROTOFS_NODE_ADDR(target->node);
	
	if(target->node == PROTOFS_ROOT_NODE)
		parent = NULL;

	for(i=0; i<PROTOFS_PER_PAGE && !found; i++) {
		if(!(_protofs_page[i].type_flags & PROTOFS_TYPE_EMPTY))
			if(_protofs_page[i].parent == parent
					&& !strcmp(_protofs_page[i].name, name))
				found = 1;
	}

	if(found)
		return protofs_fill_inode(target->fs_op, &(_protofs_page[i-1]));
	
	return NULL;	
}


inode_t * protofs_get_inode (fs_instance_t *inst, uint32 node)
{
	inode_t *ret = NULL;

	// translate the node into address
	if(node == PROTOFS_ROOT_NODE)
	{
		ret = vfs_alloc_inode();	
		ret->type_flags = INODE_TYPE_PARENT;
		ret->data.abstract = NULL;
		ret->name[0] = '\0';
		ret->node = node;
		ret->parent = PROTOFS_INVALID_NODE;
		ret->flags = INODE_FLAG_READ | INODE_FLAG_WRITE | INODE_FLAG_EXEC;
		ret->fs_op = inst;
	}
	else if(node < PROTOFS_PER_PAGE) {
		ret = protofs_fill_inode(inst, &(_protofs_page[node]));
	}

	return ret;
}


inode_t * protofs_create_node (inode_t *parent, const char *name, uint16 type_flags,
		uint16 mode_flags, uint32 special)
{
	int i;
	int cur = -1;

	// look for a free node
	for(i=0; i<PROTOFS_PER_PAGE && cur<0; i++)
		if(_protofs_page[i].type_flags & PROTOFS_TYPE_EMPTY)
			cur = i;

	if(cur >= 0) {
		protofs_node_t *node = &(_protofs_page[cur]);
		// TODO TODO strNcpy!!!!
		strcpy(node->name, name);
		node->parent = PROTOFS_NODE_ADDR(parent->node);
		node->type_flags = type_flags;
		node->mode = mode_flags;
		
		if(type_flags & INODE_TYPE_DEV) {
			node->special.dev.major = special >> 16;
			node->special.dev.minor = special & 0xFFFF;
		}

		return protofs_fill_inode(parent->fs_op, node);
	}
	
	return NULL;

}


inode_t *protofs_fill_inode(fs_instance_t *inst, protofs_node_t *node) {
	inode_t *ret;

	if(node == NULL)
		return NULL;

	ret = vfs_alloc_inode();	
	ret->type_flags = node->type_flags & (~PROTOFS_TYPE_EMPTY);
	if(node->type_flags & INODE_TYPE_DEV) {
		ret->data.dev.major = node->special.dev.major;
		ret->data.dev.minor = node->special.dev.minor;
	}
	else
		ret->data.abstract = NULL;
	strcpy(ret->name, node->name);
	ret->node = PROTOFS_NODE_NB(node);
	ret->parent = PROTOFS_NODE_NB(node->parent); 
	ret->flags = node->mode;
	ret->fs_op = inst;

	return ret;
}
