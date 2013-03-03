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

extern file_system_t protofs_file_system;

fs_instance_t *protofs_mount (unsigned int flags);

inode_t * protofs_get_root_node (fs_instance_t *inst);

inode_t * protofs_get_sub_node (inode_t *target, int index);

int protofs_get_children_nb (inode_t *target);

inode_t * protofs_find_sub_node (inode_t *target, const char *name);

inode_t * protofs_get_inode (fs_instance_t *inst, uint32 node);

inode_t * protofs_create_node (inode_t *parent, const char *name, uint16 type_flags,
		uint16 mode_flags, uint32 special);

#endif //_FS_PROTOFS_FILE_SYSTEM_H
