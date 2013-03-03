#ifndef _FS_VFS_OP_H
#define _FS_VFS_OP_H


/**
 * This file contain the main "high level" VFS operations.
 */

#include "inode.h"

/**
 * Create a new file or directory at the given path, if exists in VFS.
 * Return 0 in success case, negative value else.
 */
int vfs_create(const char *path, const char *name, uint16 type_flags,
		uint16 mode_flags, uint32 special);


#endif //_FS_VFS_OP_H
