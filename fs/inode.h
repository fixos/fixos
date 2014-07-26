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

struct _fs_instance; // instance of a filesystem

struct file;

struct _inode {
	char name[INODE_MAX_NAME];

	struct _fs_instance *fs_op;

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
		struct _inode *mnt_root;
		// mounted point of a fs root (TYPE_ROOT)
		struct _inode *mnt_point;
	} typespec;

	uint16 count;
};

typedef struct _inode inode_t;

#endif //_FS_INODE_H
