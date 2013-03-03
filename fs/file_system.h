#ifndef _FS_FILE_SYSTEM_H
#define _FS_FILE_SYSTEM_H

/**
 * File system operations abstraction layer.
 */

#include "fs_instance.h"
#include <utils/types.h>

struct _file_system {
	// name of the fs type, 0 terminated string
	char name[8];

	// get an instance of a file system
	// TODO add a device argument
	fs_instance_t * (*mount) (unsigned int flags);

	// return the root of given fs instance as a simple inode
	inode_t * (*get_root_node) (fs_instance_t *inst);

	// return the child [index] of the target if exists, NULL else
	// only some nodes (marked with INODE_TYPE_PARENT) have children
	inode_t * (*get_sub_node) (inode_t *target, int index);

	// get number of children in a given node, or -1 if the node can't have
	// children
	int (*get_children_nb) (inode_t *target);

	// optimized method to find a child by its string name
	inode_t * (*find_sub_node) (inode_t *target, const char *name);

	// try to create a node as child of parent node, using all the
	// given information (name, mode_flags, type_flags).
	// Field 'special' is used for type specific additional info
	// (major/minor ID for device node).
	// if the node is successfuly created, return the corresponding inode
	// else return NULL
	inode_t * (*create_node) (inode_t *parent, const char *name, uint16 type_flags,
			uint16 mode_flags, uint32 special);

	// try to generate a VFS inode from the entry of internal node number node
	// in the given file system instance
	// return NULL if the node doesn't exist
	inode_t * (*get_inode) (fs_instance_t *inst, uint32 node);
};

typedef struct _file_system file_system_t;

#endif //_FS_FILE_SYSTEM_H
