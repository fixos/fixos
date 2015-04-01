/*
 * Copyright (C) 2012-2015 Leo Grange <grangeleo@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

int vfs_dir_open(struct inode *inode, struct file *filep);

off_t vfs_dir_lseek(struct file *filep, off_t offset, int whence);

int vfs_dir_getdents(struct file *filep, struct fixos_dirent *buf, size_t len);

#endif //FS_VFS_DIRECTORY_H
