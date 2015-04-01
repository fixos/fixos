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

#ifndef _FS_FILE_SYSTEM_H
#define _FS_FILE_SYSTEM_H

/**
 * File system operations abstraction layer.
 */

#include "fs_instance.h"
#include <utils/types.h>
#include <interface/fixos/stat.h>


/**
 * Filesystem-specific operations on inode(s).
 */
struct inode_operations {
	// TODO place here first_child() and other inode-related methods

	// get information about a file (as an inode)
	int (*istat) (struct inode *inode, struct stat *buf);


	/**
	 * Try to open the object designed by inode.
	 * filep is the struct allocated to store opened file informations, and
	 * is allocated and partialy set by the caller.
	 * Returns 0 if success, negative value else (so filep will be free'd).
	 */
	int (*open) (struct inode *inode, struct file *filep);
};


/**
 * Operations on the filesystem itself.
 */
struct file_system {
	// name of the fs type, 0 terminated string
	char name[8];

	// get an instance of a file system
	// TODO add a device argument
	struct fs_instance * (*mount) (unsigned int flags);

	// return the root of given fs instance as a simple inode
	struct inode * (*get_root_node) (struct fs_instance *inst);

	// return the first child of the given inode in this file system
	// use it with next_sibling() to list all children of an inode
	// return NULL if the element have no child
	struct inode * (*first_child) (struct inode *target);

	// return the next element of the fs with the same parent (sibling), or
	// NULL if the given node is the 'last' or have no sibling ( root node...)
	// WARNING : for now, this function provide no way to ensure a loop of call
	// to this function will return the good list if the parent content change
	// during the loop...
	struct inode * (*next_sibling) (struct inode *target);

	// optimized method to find a child by its string name
	struct inode * (*find_sub_node) (struct inode *target, const char *name);

	// try to create a node as child of parent node, using all the
	// given information (name, mode_flags, type_flags).
	// Field 'special' is used for type specific additional info
	// (major/minor ID for device node).
	// if the node is successfuly created, return the corresponding inode
	// else return NULL
	struct inode * (*create_node) (struct inode *parent, const char *name, uint16 type_flags,
			uint16 mode_flags, uint32 special);

	// try to generate a VFS inode from the entry of internal node number node
	// in the given file system instance
	// return NULL if the node doesn't exist
	struct inode * (*get_inode) (struct fs_instance *inst, uint32 node);

	struct inode_operations iop;
};

#endif //_FS_FILE_SYSTEM_H
