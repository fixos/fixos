#ifndef _FS_FILE_SYSTEM_H
#define _FS_FILE_SYSTEM_H

/**
 * File system operations abstraction layer.
 */

#include "fs_instance.h"
#include <utils/types.h>
#include <interface/stat.h>


/**
 * Filesystem-specific operations on inode(s).
 */
struct inode_operations {
	// TODO place here first_child() and other inode-related methods

	// get information about a file (as an inode)
	int (*istat) (inode_t *inode, struct stat *buf);


	/**
	 * Try to open the object designed by inode.
	 * filep is the struct allocated to store opened file informations, and
	 * is allocated and partialy set by the caller.
	 * Returns 0 if success, negative value else (so filep will be free'd).
	 */
	int (*open) (struct _inode *inode, struct file *filep);
};


/**
 * Operations on the filesystem itself.
 */
struct _file_system {
	// name of the fs type, 0 terminated string
	char name[8];

	// get an instance of a file system
	// TODO add a device argument
	fs_instance_t * (*mount) (unsigned int flags);

	// return the root of given fs instance as a simple inode
	inode_t * (*get_root_node) (fs_instance_t *inst);

	// return the first child of the given inode in this file system
	// use it with next_sibling() to list all children of an inode
	// return NULL if the element have no child
	inode_t * (*first_child) (inode_t *target);

	// return the next element of the fs with the same parent (sibling), or
	// NULL if the given node is the 'last' or have no sibling ( root node...)
	// WARNING : for now, this function provide no way to ensure a loop of call
	// to this function will return the good list if the parent content change
	// during the loop...
	inode_t * (*next_sibling) (inode_t *target);

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

	struct inode_operations iop;
};

typedef struct _file_system file_system_t;

#endif //_FS_FILE_SYSTEM_H
