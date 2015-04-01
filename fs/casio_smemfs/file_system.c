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

#include "file_system.h"
#include "smemfs_primitives_ng.h"
#include "file.h"

#include <fs/vfs.h>
#include <interface/fixos/errno.h>

#include <utils/log.h>


// very important definitions, which is probably model-dependant :/
#define CASIO_FS ((void*)(0xA0270000))

#define CASIO_STORAGE_MEM ((void*)(0xA0000000)) 

#define FULL_NODE_ID(header) ((header)->entry_id | (((header)->type == SMEMFS_FILE_DIR) ? \
				FILE_FLAG_ISDIR : 0) )

#define FILE_FLAG_ISDIR		0x20000


/**
 * Fill a previously allocated inode with the given header.
 * if header is NULL, return a node to use as root of the FS
 */
struct inode *smemfs_fill_inode(struct fs_instance *inst, struct smemfs_file_preheader *header, struct inode *ret);

const struct file_system smemfs_file_system = {
	.name = "smemfs",
	.mount = smemfs_mount,
	.get_root_node = smemfs_get_root_node,
	.first_child = smemfs_first_child,
	.next_sibling = smemfs_next_sibling,
	.find_sub_node = smemfs_find_sub_node,
	.get_inode = smemfs_get_inode,
	.create_node = NULL,

	.iop = {
		.open = smemfs_open,
		.istat = smemfs_istat
	}
};

// 1 if SMEM FS instance is already mounted
static unsigned char _smemfs_mounted = 0;

static struct fs_instance _smemfs_inst = {
	.instd = NULL
};

struct fs_instance *smemfs_mount (unsigned int flags)
{
	// TODO atomic operations...
	if(_smemfs_mounted == 0) {
		// initialize low-level primitives
		smemfs_prim_init(CASIO_FS, CASIO_STORAGE_MEM);

		_smemfs_inst.fs = &smemfs_file_system;
		_smemfs_mounted = 1;
		return &_smemfs_inst;
	}

	return NULL;
}


struct inode * smemfs_get_root_node (struct fs_instance *inst)
{
	struct inode *ret;
	ret = vfs_get_inode(inst, SMEMFS_FILE_ROOT_ID);
	
	printk(LOG_DEBUG, "smemfs: root inode=%p\n", ret);

	return ret;
}



struct inode * smemfs_first_child (struct inode *target) {
	if(target->type_flags & INODE_TYPE_PARENT) {
		struct smemfs_file_preheader *current;

		current = smemfs_prim_get_next_child(NULL, target->node);

		printk(LOG_DEBUG, "smemfs: f(%x)->%p\n", target->node & 0xFFFF, current);
		if(current != NULL) {
			return vfs_get_inode(target->fs_op, FULL_NODE_ID(current) );
		}
	}
	
	return NULL;
}


struct inode * smemfs_next_sibling (struct inode *target) {	
	if(target->node != SMEMFS_FILE_ROOT_ID) {
		struct smemfs_file_preheader *current;

		// use the target header address and the parent ID 
		current = smemfs_prim_get_next_child((struct smemfs_file_preheader*)(target->abstract),
				target->parent);

		//printk(LOG_DEBUG, "smemfs: %p->%p\n", target->abstract, current); 
		if(current != NULL) {
			return vfs_get_inode(target->fs_op, FULL_NODE_ID(current));
		}
	}
	return NULL;
}


struct inode * smemfs_find_sub_node (struct inode *target, const char *name)
{
	struct smemfs_file_preheader *header;
	header = smemfs_prim_get_atomic_file(name, target->node);
	if(header != NULL)
		return vfs_get_inode(target->fs_op, FULL_NODE_ID(header));

	return NULL;
}


struct inode * smemfs_get_inode (struct fs_instance *inst, uint32 lnode)
{
	struct smemfs_file_preheader *header = NULL;
	unsigned short node = (unsigned short)lnode;

	//printk(LOG_DEBUG, "smemfs: get_inode: 0x%x\n", lnode);
	
	if(lnode == SMEMFS_INVALID_NODE)
		return NULL;


	// check if the given node is the special root node
	if(node != SMEMFS_FILE_ROOT_ID) {
		header = smemfs_prim_get_file_byid(node, lnode & FILE_FLAG_ISDIR);
		if(header == NULL)
			return NULL;
	}
	else {
		header = NULL;
	}


	struct inode *ret;
	ret = vfs_alloc_inode(inst, lnode);
	if(ret == NULL)
	{
		printk(LOG_ERR, "vfs: unable to alloc inode\n");
		return NULL;
	}

	return smemfs_fill_inode(inst, header, ret);
}


struct inode *smemfs_fill_inode(struct fs_instance *inst, struct smemfs_file_preheader *header, struct inode *ret) 
{
	//printk(LOG_DEBUG, "smemfs: fill_inode: %p\n", header);


	ret->fs_op = inst;
	ret->abstract = (void*)header; // more data ?

	if(header == NULL)
	{
		ret->type_flags = INODE_TYPE_ROOT | INODE_TYPE_PARENT;
		ret->name[0] = '\0';
		ret->node = SMEMFS_FILE_ROOT_ID;
		ret->flags = INODE_FLAG_READ | INODE_FLAG_EXEC;
	}
	else {
		ret->node = header->entry_id;

		//printk(LOG_DEBUG, "smemfs: inode=%p\n", ret);

		ret->flags = INODE_FLAG_READ;
		ret->parent = header->parent_id;
		if(ret->parent != SMEMFS_FILE_ROOT_ID)
			ret->parent |= FILE_FLAG_ISDIR;
		ret->type_flags = 0;
		if(header->type == SMEMFS_FILE_DIR) {
			ret->type_flags |= INODE_TYPE_PARENT;
			ret->node |= FILE_FLAG_ISDIR;
		}
		// copy the name
		smemfs_prim_get_file_name(header, ret->name, CHAR_ASCII_AUTO);
	}

	//printk(LOG_DEBUG, "smemfs: filename=%s\n", ret->name);
	
	return ret;
}


int smemfs_open (struct inode *inode, struct file *filep) {
	if(inode->flags & O_WRONLY)
		return -EROFS;

	// nothing to do? (inode already filled)
	filep->private_data = NULL;
	filep->op = & smemfs_file_operations;
	return 0;
}


int smemfs_istat(struct inode *inode, struct stat *buf) {
	struct smemfs_file_preheader *header;

	header = (struct smemfs_file_preheader*) inode->abstract;
	buf->st_dev = makedev(0, 0);
	buf->st_rdev = makedev(0, 0);
	
	if(header->type == SMEMFS_FILE_DIR)
		buf->st_mode = S_IFDIR;
	else
		buf->st_mode = S_IFREG;
	// SMEM filesystem is readonly for now
	buf->st_mode |= S_IRUSR | S_IXUSR;

	buf->st_size = smemfs_prim_get_file_size(header);
	return 0;
}

