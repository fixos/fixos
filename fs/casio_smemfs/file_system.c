#include "file_system.h"
#include "smemfs_primitives.h"
#include <fs/vfs.h>

#include <utils/log.h>


/**
 * Fill a previously allocated inode with the given header.
 * if header is NULL, return a node to use as root of the FS
 */
inode_t *smemfs_fill_inode(fs_instance_t *inst, const unsigned char*header, inode_t *ret);

struct _file_system smemfs_file_system = {
	.name = "smemfs",
	.mount = smemfs_mount,
	.get_root_node = smemfs_get_root_node,
	.first_child = smemfs_first_child,
	.next_sibling = smemfs_next_sibling,
	.find_sub_node = smemfs_find_sub_node,
	.get_inode = smemfs_get_inode,
	.create_node = NULL
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
	inode_t *ret;
	ret = vfs_get_inode(inst, ROOT_ID);
	
	printk("smemfs: root inode=%p\n", ret);

	return ret;
}



inode_t * smemfs_first_child (inode_t *target) {
	if(target->type_flags & INODE_TYPE_PARENT) {
		const unsigned char *current = NULL;

		current = getNextChildHeader(current, target->node);

		printk("smemfs: f(%x)->%p\n", target->node & 0xFFFF, current);
		if(current != NULL) {
			return vfs_get_inode(target->fs_op, FULL_NODE_ID(current) );
		}
	}
	
	return NULL;
}


inode_t * smemfs_next_sibling (inode_t *target) {	
	if(target->node != ROOT_ID) {
		const unsigned char *current = NULL;

		// use the target header address and the parent ID 
		current = getNextChildHeader((const unsigned  char*)(target->abstract)
				, target->parent);

		//printk("smemfs: %p->%p\n", target->abstract, current); 
		if(current != NULL) {
			return vfs_get_inode(target->fs_op, FULL_NODE_ID(current));
		}
	}
	return NULL;
}


inode_t * smemfs_find_sub_node (inode_t *target, const char *name)
{
	const unsigned char *header;
	header = getAtomicFileHeader(name, target->node);
	if(header != NULL)
		return vfs_get_inode(target->fs_op, FULL_NODE_ID(header));

	return NULL;
}


inode_t * smemfs_get_inode (fs_instance_t *inst, uint32 lnode)
{
	const unsigned char *header = NULL;
	unsigned short node = (unsigned short)lnode;

	//printk("smemfs: get_inode: 0x%x\n", lnode);
	
	if(lnode == INVALID_NODE)
		return NULL;


	// check if the given node is the special root node
	if(node != ROOT_ID) {
		header = getFileHeader(node, lnode & FILE_FLAG_ISDIR);
		if(header == NULL)
			return NULL;
	}
	else {
		header = NULL;
	}


	inode_t *ret;
	ret = vfs_alloc_inode(inst, lnode);
	if(ret == NULL)
	{
		printk("Error:\nvfs: unable to alloc inode\n");
		return NULL;
	}

	return smemfs_fill_inode(inst, header, ret);
}


inode_t *smemfs_fill_inode(fs_instance_t *inst, const unsigned char*header, inode_t *ret) 
{
	//printk("smemfs: fill_inode: %p\n", header);


	ret->fs_op = inst;
	ret->abstract = (void*)header; // more data ?

	if(header == NULL)
	{
		ret->type_flags = INODE_TYPE_ROOT | INODE_TYPE_PARENT;
		ret->name[0] = '\0';
		ret->node = ROOT_ID;
		ret->flags = INODE_FLAG_READ | INODE_FLAG_EXEC;
	}
	else {
		ret->node = getFileId(header);

		//printk("smemfs: inode=%p\n", ret);

		ret->flags = INODE_FLAG_READ;
		ret->parent = getFileParentDirId(header);
		if(ret->parent != ROOT_ID)
			ret->parent |= FILE_FLAG_ISDIR;
		ret->type_flags = 0;
		if(isDirectory(header)) {
			ret->type_flags |= INODE_TYPE_PARENT;
			ret->node |= FILE_FLAG_ISDIR;
		}
		// copy the name
		getFileName(header, ret->name, CHAR_ASCII_AUTO);
	}

	//printk("smemfs: filename=%s\n", ret->name);
	
	return ret;
}
