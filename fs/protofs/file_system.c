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
	.first_child = protofs_first_child,
	.next_sibling = protofs_next_sibling,
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
inode_t *protofs_fill_inode(fs_instance_t *inst, protofs_node_t *node, inode_t *ret);



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
	return vfs_get_inode(inst, PROTOFS_ROOT_NODE);
}


inode_t * protofs_first_child(inode_t *target)
{
	if(target->type_flags & INODE_TYPE_PARENT) {
		int i;
		int found = 0;
		void *parent = PROTOFS_NODE_ADDR(target->node);

		if(target->node == PROTOFS_ROOT_NODE)
			parent = NULL;

		for(i=0; i<PROTOFS_PER_PAGE && !found; i++) {
			if(!(_protofs_page[i].type_flags & PROTOFS_TYPE_EMPTY))
				if(_protofs_page[i].parent == parent)
					found = 1;
		}

		if(found)
			return vfs_get_inode(target->fs_op, i-1);
	}
	
	return NULL;
}

inode_t * protofs_next_sibling(inode_t *target)
{
	if(target->node != PROTOFS_ROOT_NODE) {
		int i;
		int found = 0;
		void *parent = PROTOFS_NODE_ADDR(target->parent);

		if(target->parent == PROTOFS_ROOT_NODE)
			parent = NULL;

		// start to search from the target node ID + 1 (index in the array)
		for(i=target->node+1; i<PROTOFS_PER_PAGE && !found; i++) {
			if(!(_protofs_page[i].type_flags & PROTOFS_TYPE_EMPTY))
				if(_protofs_page[i].parent == parent)
					found = 1;
		}

		if(found)
			return vfs_get_inode(target->fs_op, i-1);
	}

	return NULL;
}


inode_t * protofs_find_sub_node (inode_t *target, const char *name)
{
	// the simplest algorithm : test all the page of nodes...
	
	if(target->type_flags & INODE_TYPE_PARENT) {
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
			return vfs_get_inode(target->fs_op, (uint32) i-1);
	}
	
	return NULL;
}


inode_t * protofs_get_inode (fs_instance_t *inst, uint32 node)
{
	protofs_node_t *nodeptr;

	// translate the node into address
	if(node == PROTOFS_ROOT_NODE)
		nodeptr = NULL;
	else {
		if(node < PROTOFS_PER_PAGE) 
			nodeptr = &(_protofs_page[node]);
		else
			return NULL;
	}

	inode_t *ret;
	ret = vfs_alloc_inode(inst, node);	
	if(ret == NULL) {
		printk("protofs: vfs inode alloc failed\n");
	}

	return protofs_fill_inode(inst, nodeptr, ret);
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
		if(parent->node == PROTOFS_ROOT_NODE)
			node->parent = NULL;
		else
			node->parent = PROTOFS_NODE_ADDR(parent->node);
		node->type_flags = type_flags;
		node->mode = mode_flags;
		
		if(type_flags & INODE_TYPE_DEV) {
			node->special.dev.major = special >> 16;
			node->special.dev.minor = special & 0xFFFF;
		}

		return vfs_get_inode(parent->fs_op, PROTOFS_NODE_NB(node));
	}
	
	return NULL;

}


inode_t *protofs_fill_inode(fs_instance_t *inst, protofs_node_t *node, inode_t *ret) {

	ret->fs_op = inst;
	if(node == NULL) {
		ret->type_flags = INODE_TYPE_ROOT | INODE_TYPE_PARENT;
		ret->abstract = NULL;
		ret->name[0] = '\0';
		ret->node = PROTOFS_ROOT_NODE; 
		ret->flags = INODE_FLAG_READ | INODE_FLAG_WRITE | INODE_FLAG_EXEC;
		ret->fs_op = inst;
	}
	else {
		ret->type_flags = node->type_flags & (~PROTOFS_TYPE_EMPTY);
		if(node->type_flags & INODE_TYPE_DEV) {
			ret->typespec.dev.major = node->special.dev.major;
			ret->typespec.dev.minor = node->special.dev.minor;
		}
		else
			ret->abstract = NULL;
		strcpy(ret->name, node->name);
		ret->node = PROTOFS_NODE_NB(node);
		if(node->parent == NULL)
			ret->parent = PROTOFS_ROOT_NODE;
		else
			ret->parent = PROTOFS_NODE_NB(node->parent); 
		ret->flags = node->mode;
	}

	return ret;
}