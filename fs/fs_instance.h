#ifndef _FS_FS_INSTANCE_H
#define _FS_FS_INSTANCE_H

/**
 * File System instance, containing fs operations and private data
 */

#include "inode.h"

struct _file_system;

struct _fs_instance {
	const struct _file_system *fs;
	void *instd; // instance data, may be NULL for some FS
};

typedef struct _fs_instance fs_instance_t;

#endif //_FS_FS_INSTANCE_H
