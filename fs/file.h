#ifndef _FS_FILE_H
#define _FS_FILE_H

#include <utils/types.h>
#include "inode.h"

// for flags
#include <interface/fcntl.h>

struct file_operations;

struct file {
	size_t pos;
	int flags;

	// inode associated to this file, may be NULL for special files (pipes...)
	inode_t *inode;

	// file operations to use, usualy the same as inode->file_op (but must be
	// used because inode may be NULL, not op!)
	const struct file_operations *op;

	// number of references for this file description (shared across fork...)
	int count;

	// Pointer to a data structure depending of the FS of the file (may be NULL)
	void *private_data; 
};



#endif //_FS_FILE_H
