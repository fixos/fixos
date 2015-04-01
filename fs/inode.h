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

#ifndef _FS_INODE_H
#define _FS_INODE_H

#include <utils/types.h>

/**
 * Generic FS object structure, like UNIX-inodes.
 * This type is the virtual file system abstraction format.
 */

// parent inode (like a directory)
#define INODE_TYPE_PARENT		1
// device driver node
#define INODE_TYPE_DEV			2
// used to mark an inode as root of its file system.
// entry with this flag must have a node ID, but don't need
// to have a parent or a name
#define INODE_TYPE_ROOT			4
// mount point (a file system instance is mounted on this node)
#define INODE_TYPE_MOUNTPOINT	8


#define INODE_FLAG_READ			4
#define INODE_FLAG_WRITE		2
#define INODE_FLAG_EXEC			1


// maximum name size for an inode entry
#define INODE_MAX_NAME			20

struct fs_instance; // instance of a filesystem

struct file;

struct inode {
	char name[INODE_MAX_NAME];

	struct fs_instance *fs_op;

	// corresponding internal number and parent number in the FS instance
	uint32 node;
	uint32 parent;

	uint16 type_flags;
	uint16 flags;

	void *abstract; // private data for the FS instance

	// type-specific informations :
	union {
		// for TYPE_DEV
		dev_t dev;
		// root of a mounted point (TYPE_MOUNTPOINT)
		struct inode *mnt_root;
		// mounted point of a fs root (TYPE_ROOT)
		struct inode *mnt_point;
	} typespec;

	uint16 count;
};


#endif //_FS_INODE_H
