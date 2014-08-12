#ifndef FS_VFS_DIRECTORY_H
#define FS_VFS_DIRECTORY_H

/**
 * Directories manipulation functions (for getdents()).
 * An openned directory is handled by the VFS level, which call filesystem
 * specific functions to list directory entries (a normal file is handled
 * directly by filesystem-specific file_operations).
 */

#include <interface/fixos/dirent.h>
#include <utils/types.h>
#include <fs/inode.h>
#include <fs/file.h>

int vfs_dir_open(inode_t *inode, struct file *filep);

off_t vfs_dir_lseek(struct file *filep, off_t offset, int whence);

int vfs_dir_getdents(struct file *filep, struct fixos_dirent *buf, size_t len);

#endif //FS_VFS_DIRECTORY_H
