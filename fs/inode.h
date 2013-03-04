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


#define INODE_FLAG_READ			4
#define INODE_FLAG_WRITE		2
#define INODE_FLAG_EXEC			1


// maximum name size for an inode entry
#define INODE_MAX_NAME			24

struct _fs_instance; // instance of a filesystem

struct _inode {
	char name[INODE_MAX_NAME];

	struct _fs_instance *fs_op;

	// corresponding internal number and parent number in the FS instance
	uint32 node;
	uint32 parent;

	uint16 type_flags;
	uint16 flags;

	union {
		void *abstract; // private data for the FS instance
		struct {
			uint16 major;
			uint16 minor;
		} dev;
	} data;
};

typedef struct _inode inode_t;

#endif //_FS_INODE_H
