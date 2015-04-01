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

#include "vfs_op.h"
#include "vfs.h"

#include <utils/types.h>
#include <utils/log.h>

int vfs_create(const char *path, const char *name, uint16 type_flags,
		uint16 mode_flags, uint32 special)
{
	struct inode *target = vfs_resolve(path);

	if(target != NULL) {
		struct inode *newnode;

		newnode = target->fs_op->fs->create_node(target, name, type_flags,
				mode_flags, special);
		if(newnode != NULL)
			vfs_release_inode(newnode);

		printk(LOG_DEBUG, "vfs_create: new=%p\n", newnode);

		vfs_release_inode(target);
		return newnode == NULL ? -1 : 0;
	}

	printk(LOG_DEBUG, "vfs_create: fail '%s'\n",path);

	return -1;
}
