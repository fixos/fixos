#ifndef _FS_SMEMFS_FILE_SYSTEM_H
#define _FS_SMEMFS_FILE_SYSTEM_H

#include <fs/file_system.h>

/**
 * Implementation of file system abstraction layer for the Casio SMEM FS.
 * For this FS, only one instance can be mounted at a time, and the
 * internal nodes numbers are simply the absolute address of each entry.
 */


#define SMEMFS_INVALID_NODE			0xFFFFFFFF

extern const struct file_system smemfs_file_system;

struct fs_instance *smemfs_mount (unsigned int flags);

struct inode * smemfs_get_root_node (struct fs_instance *inst);

struct inode * smemfs_first_child (struct inode *target); 

struct inode * smemfs_next_sibling (struct inode *target);	

struct inode * smemfs_find_sub_node (struct inode *target, const char *name);

struct inode * smemfs_get_inode (struct fs_instance *inst, uint32 node);

int smemfs_open (struct inode *inode, struct file *filep);

int smemfs_istat(struct inode *inode, struct stat *buf);

#endif //_FS_SMEMFS_FILE_SYSTEM_H
