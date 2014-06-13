#ifndef _FS_FILE_H
#define _FS_FILE_H

#include <utils/types.h>
#include "inode.h"

// Internal constants definition
#define _FILE_READ    1
#define _FILE_WRITE   2

// Flags :
#define _FILE_WRITTEN 1
#define _FILE_READED  2
#define _FILE_EOF_REATCHED 4

struct file_operations;

struct file {
	size_t pos;
	int open_mode;
	int flags;

	// inode associated to this file, may be NULL for special files (pipes...)
	inode_t *inode;

	// file operations to use, usualy the same as inode->file_op (but must be
	// used because inode may be NULL, not op!)
	struct file_operations *op;

	// Pointer to a data structure depending of the FS of the file (may be NULL)
	void *private_data; 
};



#endif //_FS_FILE_H
