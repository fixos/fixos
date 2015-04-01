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

#ifndef _FS_PROTOFS_FILE_SYSTEM_H
#define _FS_PROTOFS_FILE_SYSTEM_H

#include <fs/file_system.h>

/**
 * Implementation of a very simple FS in RAM, which only allow
 * to make simple directories and device nodes.
 * This FS allow to mount a 'fake' FS as VFS root, and to
 * manage /dev/... at run time.
 * For now, this FS is a singleton (only 1 instance at a time).
 * In protofs, a node represent an offset in the internal node list.
 */

extern const struct file_system protofs_file_system;

struct fs_instance *protofs_mount (unsigned int flags);

struct inode * protofs_get_root_node (struct fs_instance *inst);

struct inode * protofs_next_sibling(struct inode *target);

struct inode * protofs_first_child(struct inode *target);

int protofs_get_children_nb (struct inode *target);

struct inode * protofs_find_sub_node (struct inode *target, const char *name);

struct inode * protofs_get_inode (struct fs_instance *inst, uint32 node);

struct inode * protofs_create_node (struct inode *parent, const char *name, uint16 type_flags,
		uint16 mode_flags, uint32 special);

int protofs_istat(struct inode *inode, struct stat *buf);

int protofs_open(struct inode *inode, struct file *filep);

#endif //_FS_PROTOFS_FILE_SYSTEM_H
