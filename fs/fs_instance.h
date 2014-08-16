#ifndef _FS_FS_INSTANCE_H
#define _FS_FS_INSTANCE_H

/**
 * File System instance, containing fs operations and private data
 */

#include "inode.h"

struct file_system;

struct fs_instance {
	const struct file_system *fs;
	void *instd; // instance data, may be NULL for some FS
};

#endif //_FS_FS_INSTANCE_H
