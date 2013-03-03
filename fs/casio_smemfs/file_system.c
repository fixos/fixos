#include "file_system.h"
#include "smemfs_primitives.h"
#include <fs/vfs.h>

#include <utils/log.h>


/**
 * Allocate a new VFS inode and fill it with the given header.
 */
inode_t *smemfs_fill_inode(fs_instance_t *inst, const unsigned char*header);

struct _file_system smemfs_file_system = {
	.name = "smemfs",
	.mount = smemfs_mount,
	.get_root_node = smemfs_get_root_node,
	.get_sub_node = smemfs_get_sub_node,
	.get_children_nb = NULL,
	.find_sub_node = smemfs_find_sub_node,
	.get_inode = smemfs_get_inode
};

// 1 if SMEM FS instance is already mounted
static unsigned char _smemfs_mounted = 0;

static fs_instance_t _smemfs_inst = {
	.instd = NULL
};

fs_instance_t *smemfs_mount (unsigned int flags)
{
	// TODO atomic operations...
	if(_smemfs_mounted == 0) {
		_smemfs_inst.fs = &smemfs_file_system;
		_smemfs_mounted = 1;
		return &_smemfs_inst;
	}

	return NULL;
}


inode_t * smemfs_get_root_node (fs_instance_t *inst)
{
	uint32 rootnode = 0xFFFF; 
	return(smemfs_get_inode(inst, rootnode));
}



inode_t * smemfs_get_sub_node (inode_t *target, int index)
{
	// not optimized, and not sure it's possible to optimize more...
	if(target->flags & INODE_TYPE_PARENT)
		return NULL;

	const unsigned char *current = NULL;
	int curind = 0;

	do {
		current = getNextChildHeader(current, target->parent);
		curind++;
	} while(current != NULL && curind <= index);

	if(current != NULL) {
		return smemfs_fill_inode(target->fs_op, current);
	}

	return NULL;
}

int smemfs_get_children_nb (inode_t *target)
{
	if(target->flags & INODE_TYPE_PARENT)
		return -1;
	// TODO
	return 0;
}

inode_t * smemfs_find_sub_node (inode_t *target, const char *name)
{
	const unsigned char *header;
	header = getAtomicFileHeader(name, target->node);
	if(header != NULL)
		return smemfs_fill_inode(target->fs_op, header);

	return NULL;
}


inode_t * smemfs_get_inode (fs_instance_t *inst, uint32 lnode)
{
	inode_t *ret = NULL;
	const unsigned char *header = NULL;
	unsigned short node = (unsigned short)lnode;

	printk("smemfs: get_inode: 0x%x\n", lnode);
	
	if(lnode == INVALID_NODE)
		return NULL;


	// check if the given node is the special root node
	if(node == ROOT_ID) {
		ret = vfs_alloc_inode();
		if(ret == NULL)
		{
			printk("Error:\nvfs: unable to alloc inode\n");
			return NULL;
		}
		ret->data.abstract = NULL;

		ret->fs_op = inst;
		ret->node = node;

		printk("smemfs: root inode=%p\n", ret);

		ret->flags = INODE_FLAG_READ;
		ret->parent = INVALID_NODE;
		ret->type_flags = INODE_TYPE_PARENT;
		ret->name[0] = '\0';

	}
	else if ( (header = getFileHeader(node)) != NULL) {
		return smemfs_fill_inode(inst, header);
	}

	return ret;
}


inode_t *smemfs_fill_inode(fs_instance_t *inst, const unsigned char*header) 
{
	inode_t *ret = NULL;

	printk("smemfs: fill_inode: %p\n", header);

	ret = vfs_alloc_inode();
	if(ret == NULL)
	{
		printk("Error:\nvfs: unable to alloc inode\n");
		return NULL;
	}

	ret->data.abstract = (void*)header; // more data ?

	ret->fs_op = inst;
	ret->node = getFileId(header);

	printk("smemfs: inode=%p\n", ret);

	ret->flags = INODE_FLAG_READ;
	ret->parent = getFileParentDirId(header);
	ret->type_flags = 0;
	if(isDirectory(header))
		ret->type_flags |= INODE_TYPE_PARENT;
	// copy the name
	getFileName(header, ret->name, CHAR_ASCII_AUTO);

	printk("smemfs: filename=%s\n", ret->name);
	
	return ret;
}
