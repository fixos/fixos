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

#ifndef _FS_FILE_H
#define _FS_FILE_H

#include <utils/types.h>
#include "inode.h"

// for flags
#include <interface/fixos/fcntl.h>

struct file_operations;

struct file {
	size_t pos;
	int flags;

	// inode associated to this file, may be NULL for special files (pipes...)
	struct inode *inode;

	// file operations to use, usualy the same as inode->file_op (but must be
	// used because inode may be NULL, not op!)
	const struct file_operations *op;

	// number of references for this file description (shared across fork...)
	int count;

	// Pointer to a data structure depending of the FS of the file (may be NULL)
	void *private_data; 
};



#endif //_FS_FILE_H
