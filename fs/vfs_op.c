#include "vfs_op.h"
#include "vfs.h"

#include <utils/types.h>
#include <utils/log.h>

int vfs_create(const char *path, const char *name, uint16 type_flags,
		uint16 mode_flags, uint32 special)
{
	inode_t *target = vfs_resolve(path);

	if(target != NULL) {
		inode_t *newnode;

		newnode = target->fs_op->fs->create_node(target, name, type_flags,
				mode_flags, special);
		if(newnode != NULL)
			vfs_free_inode(newnode);

		printk("vfs_create: new=%p\n", newnode);

		vfs_free_inode(target);
		return newnode == NULL ? -1 : 0;
	}

	printk("vfs_create: fail '%s'\n",path);

	return -1;
}
