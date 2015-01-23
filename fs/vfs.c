#include "vfs.h"
#include "fs_instance.h"

#include <utils/strutils.h>
#include <fs/vfs_cache.h>
#include <sys/process.h>

#include <utils/log.h>

struct _fs_mount_point {
	struct fs_instance *inst;
	struct inode *mountpoint;
};


static struct fs_instance *_root_fs;

static struct _fs_mount_point _mounted_fs[VFS_MAX_MOUNT];

static struct file_system const * vfs_fslist[VFS_MAX_FS];


void vfs_init()
{
	int i;
	vfs_cache_init();
	for(i=0; i<VFS_MOUNT_ROOT; i++) {
		_mounted_fs[i].inst = NULL;
		_mounted_fs[i].mountpoint = NULL;
	}
	for(i=0; i<VFS_MAX_FS; i++)
		vfs_fslist[i] = NULL;
}



struct inode *vfs_alloc_inode(struct fs_instance *inst, uint32 node)
{
	// take the first free inode in the list, and remove it
	struct inode *ret;
	ret = (struct inode*) (vfs_cache_alloc(inst, node));
	if(ret != NULL)
		ret->count = 0;
	return ret;
}



void vfs_release_inode(struct inode *inode)
{
	//printk(LOG_DEBUG, "vfs: --(%s, , 0x%x, %d)\n", inode->name, inode->node, inode->count);
	if(inode->count > 0) {
		if(inode->count == 1 && (inode->type_flags & (INODE_TYPE_ROOT | INODE_TYPE_MOUNTPOINT)) )
			printk(LOG_DEBUG, "vfs: W: trying free mount|root\n");
		else {
			inode->count--;
			if(inode->count == 0)
				vfs_cache_remove(inode->fs_op, inode->node);
		}
	}
	else 
		printk(LOG_DEBUG, "vfs: W: trying free count=0\n");

}


struct inode *vfs_get_inode(struct fs_instance *inst, uint32 nodeid)
{
	// ask to the vfs_cache in first...
	struct inode *ret;
	struct vfs_cache_entry *cached;

	cached = vfs_cache_find(inst, nodeid);
	if(cached == NULL)
		ret = inst->fs->get_inode(inst, nodeid);
	else
		ret = &(cached->inode);

	if(ret != NULL) {
		//printk(LOG_DEBUG, "vfs: ++(%s, , 0x%x, %d)\n", ret->name, ret->node, ret->count);
		ret->count++; // increment counter of usage
	}
	else 
		printk(LOG_DEBUG, "vfs: getinode: !0x%x\n", nodeid);
	
	return ret;
}


struct inode *vfs_first_child(struct inode *target)
{
	if(target->type_flags & INODE_TYPE_PARENT) {
		struct inode *real = target;
		if(target->type_flags & INODE_TYPE_MOUNTPOINT) {
			// replace with the real inode (root of the mounted FS)
			real = target->typespec.mnt_root;
			//printk(LOG_DEBUG, "vfs: rslv mount: %s\n", real == NULL ? "nil" : real->fs_op->fs->name);
		}
		return real->fs_op->fs->first_child(real);
	}
	return NULL;
}


struct inode *vfs_next_sibling(struct inode *target)
{
	if(!(target->type_flags & INODE_TYPE_ROOT)) {
		return target->fs_op->fs->next_sibling(target);
	}
	return NULL;
}



void vfs_register_fs(const struct file_system *fs, int flags)
{
	int ok = 0;

	if(flags & VFS_REGISTER_STATIC)
	{
		int i;

		for(i=0; i<VFS_MAX_FS && !ok; i++) {
			if(vfs_fslist[i] == NULL) {
				vfs_fslist[i] = fs;
				
				ok = 1;
			}
		}
	}
	// TODO dynamic allocation???
	
	if(!ok)
		printk(LOG_DEBUG, "vfs: unable to register fs\n");
}



int vfs_mount(const char *fsname, const char *path, int flags)
{
	const struct file_system *fs = NULL;
	int i;

	for(i=0; i<VFS_MAX_FS && fs==NULL; i++)
	{
		if(vfs_fslist[i] != NULL &&
				!strcmp(fsname, vfs_fslist[i]->name))
		{
			fs = vfs_fslist[i];
		}
	}

	if(fs == NULL) {
		printk(LOG_DEBUG, "vfs: fs not found '%s'\n", fsname);
		return -1;
	}

	// Mount root
	if(flags & VFS_MOUNT_ROOT) {
		if(_root_fs == NULL) {
			_root_fs = fs->mount(0);
			if(_root_fs != NULL) {
				// root inode must never be removed from cache
				struct inode *root_inode = _root_fs->fs->get_root_node(_root_fs);
				if(root_inode != NULL) {
					root_inode->type_flags = INODE_TYPE_ROOT | INODE_TYPE_PARENT;
					root_inode->typespec.mnt_point = NULL; // no real mount point
					return 0;
				}
			}
		}
	}
	// Mount Normal
	else {
		struct _fs_mount_point *mntp = NULL;
		
		// find a free mount fs data
		for(i=0; i<VFS_MAX_MOUNT && _mounted_fs[i].inst!=NULL; i++);
		
		if(i<VFS_MAX_MOUNT) {
			mntp = &(_mounted_fs[i]);
			// find the mount point inode
			struct inode *mnt_inode = vfs_resolve(path);
			if(mnt_inode != NULL && (mnt_inode->type_flags & INODE_TYPE_PARENT)) {
				
				// add the mounted point/fs
				mntp->inst = fs->mount(0);
				if(mntp->inst != NULL) {
					// root inode must never be removed from cache
					struct inode *root_inode = mntp->inst->fs->get_root_node(mntp->inst);
					if(root_inode != NULL) {
						root_inode->typespec.mnt_point = mnt_inode;
						root_inode->type_flags = INODE_TYPE_ROOT | INODE_TYPE_PARENT;
						mntp->mountpoint = mnt_inode;
						// set the INODE_TYPE_MOUNTPOINT in the inode, and never
						// free it (don't release it)
						// TODO flag to say an inode must never be un-cached?
						mnt_inode->type_flags |= INODE_TYPE_MOUNTPOINT;
						mnt_inode->typespec.mnt_root = root_inode;
						return 0;
					}
				}
			}
			else
				printk(LOG_DEBUG, "vfs: unable to mount (inv. inode)\n");
			
		}
	}
	
	return -1;
}


struct inode *vfs_resolve(const char *path)
{
	// ok, to do that we need to split the path, and to check each directory
	// until the file is reached
	if(_root_fs != NULL) {
		char tmpname[INODE_MAX_NAME];
		int ppos;
		int namepos;

		struct inode *ret = NULL;
		struct inode *current;
		struct process *cur = process_get_current();

		ppos = 0;
		if(path[0] == '/' || cur->cwd == NULL) {
			// if the path is absolute or if current process have no working dir
			current = _root_fs->fs->get_root_node(_root_fs);
			if(path[0] == '/')
				ppos = 1;
		}
		else {
			// the path is relative, first inode is current working directory
			current = cur->cwd;
		}

		do {
			struct inode *swap = NULL;
			char c;
			namepos = 0;

			//copy the next name
			c = path[ppos];
			while(c != '/' && c != '\0') {
				tmpname[namepos] = c;
				ppos++;
				namepos++;
				c = path[ppos];
			}
			tmpname[namepos] = '\0';
			//printk(LOG_DEBUG, "resolve: split=%s\n", tmpname);
			
			// look for an entry with this name :
			if(namepos > 0) {
				swap = vfs_walk_entry(current, tmpname);
				vfs_release_inode(current);
				current = swap;
			}

			if(c != '\0')
				ppos++;
			else
				ret = current;

		} while (ret == NULL && current != NULL);

		return ret;
	}

	return NULL;
}



// TODO : how to give the name and other properties from real node
// to mouted-on FS root???
struct inode *vfs_walk_entry(struct inode *parent, const char *name)
{
	struct inode *ret = NULL;
	struct inode *real = parent;

	// current entry (".")
	if(name[0] == '.' && name[1] == '\0') {
		ret = parent;
		//printk(LOG_DEBUG, "vfs: ++(%s, , 0x%x, %d)\n", parent->name, parent->node, parent->count);
		parent->count++;
	}
	// parent entry ("..")
	else if(name[0] == '.' && name[1] == '.' && name[2] == '\0') {
		// get the mounted point if its a fs root node
		if(parent->type_flags & INODE_TYPE_ROOT) { 
			// don't change the count
			real = parent->typespec.mnt_point;
			//printk(LOG_DEBUG, "vfs: rslv root: %s\n", real == NULL ? "nil" : real->name);
		}
		// get the parent (may be NULL if parent is the root fs
		if(real != NULL) {
			ret = vfs_get_inode(real->fs_op, real->parent);
		}
	}

	else {
		if(parent->type_flags & INODE_TYPE_MOUNTPOINT) {
			// replace with the real inode (root of the mounted FS)
			real = parent->typespec.mnt_root;
			//printk(LOG_DEBUG, "vfs: rslv mount: %s\n", real == NULL ? "nil" : real->fs_op->fs->name);
		}
		ret = real->fs_op->fs->find_sub_node(real, name);
	}

	return ret;
}



struct inode *vfs_resolve_mount(struct inode *inode) {
	struct inode *ret = NULL;

	// check if the entry is a mount point
	if(inode->type_flags & INODE_TYPE_MOUNTPOINT) {
		// replace with the real inode (root of the mounted FS)
		ret = inode->typespec.mnt_root;
/*
		for(i=0; i<VFS_MAX_MOUNT && mounted == NULL; i++) {
			if(_mounted_fs[i].mountpoint == ret)
				mounted = _mounted_fs[i].inst;
		}

		if(mounted != NULL) {
			vfs_release_inode(ret);
			ret = mounted->fs->get_root_node(mounted);
		}
		else
			printk(LOG_DEBUG, "vfs: error: bad mount point\n    node='%s'\n", name);
*/
	}
	else if(inode->type_flags & INODE_TYPE_ROOT) {
		ret = inode->typespec.mnt_point;
	}

	return ret;
}

