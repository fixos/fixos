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
