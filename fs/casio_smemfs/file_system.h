#ifndef _FS_SMEMFS_FILE_SYSTEM_H
#define _FS_SMEMFS_FILE_SYSTEM_H

#include <fs/file_system.h>

/**
 * Implementation of file system abstraction layer for the Casio SMEM FS.
 * For this FS, only one instance can be mounted at a time, and the
 * internal nodes numbers are simply the absolute address of each entry.
 */


fs_instance_t *smemfs_mount (unsigned int flags);

inode_t * smemfs_get_root_node (fs_instance_t *inst);

inode_t * smemfs_get_sub_node (inode_t *target, int index);

int smemfs_get_children_nb (inode_t *target);

inode_t * smemfs_find_sub_node (inode_t *target, const char *name);

inode_t * smemfs_get_inode (fs_instance_t *inst, uint32 node);

#endif //_FS_SMEMFS_FILE_SYSTEM_H
