#include "vfs.h"
#include "fs_instance.h"

#include <utils/strutils.h>
#include <fs/vfs_cache.h>

#include <utils/log.h>

struct _fs_mount_point {
	fs_instance_t *inst;
	inode_t *mountpoint;
};


static fs_instance_t *_root_fs;

static struct _fs_mount_point _mounted_fs[VFS_MAX_MOUNT];

static file_system_t *vfs_fslist[VFS_MAX_FS];


void vfs_init()
{
	vfs_cache_init();
}



inode_t *vfs_alloc_inode(fs_instance_t *inst, uint32 node)
{
	// take the first free inode in the list, and remove it
	inode_t *ret;
	ret = (inode_t*) (vfs_cache_alloc(inst, node));
	if(ret != NULL)
		ret->count = 0;
	return ret;
}



void vfs_release_inode(inode_t *inode)
{
	if(inode->count > 0) inode->count--;
	if(inode->count == 0)
		vfs_cache_remove(inode->fs_op, inode->node);
}


inode_t *vfs_get_inode(fs_instance_t *inst, uint32 nodeid)
{
	// ask to the vfs_cache in first...
	inode_t *ret;
	vfs_cache_entry_t *cached;

	cached = vfs_cache_find(inst, nodeid);
	if(cached == NULL)
		ret = inst->fs->get_inode(inst, nodeid);
	else
		ret = &(cached->inode);

	if(ret != NULL)
		ret->count++; // increment counter of usage
	else 
		printk("vfs: getinode: !0x%x\n", nodeid);
	
	return ret;
}



void vfs_register_fs(file_system_t *fs, int flags)
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
		printk("vfs: unable to register fs\n");
}



int vfs_mount(const char *fsname, const char *path, int flags)
{
	file_system_t *fs = NULL;
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
		printk("vfs: fs not found '%s'\n", fsname);
		return -1;
	}

	// Mount root
	if(flags & VFS_MOUNT_ROOT) {
		if(_root_fs == NULL) {
			_root_fs = fs->mount(0);
			if(_root_fs != NULL)
				return 0;
		}
	}
	// Mount Normal
	else {
		struct _fs_mount_point *mntp = NULL;
		
		// find a free mount fs data
		for(i=0; i<VFS_MAX_MOUNT && mntp->inst!=NULL; i++)
			mntp = &(_mounted_fs[i]);
		
		if(i<VFS_MAX_MOUNT) {
			// find the mount point inode
			inode_t *mnt_inode = vfs_resolve(path);
			if(mnt_inode != NULL) {
				
				// add the mounted point/fs
				mntp->inst = fs->mount(0);
				if(mntp->inst != NULL) {
					mntp->mountpoint = mnt_inode;
					// set the INODE_TYPE_MOUNTPOINT in the inode, and never
					// free it (don't release it)
					// TODO flag to say an inode must never be un-cached?
					mnt_inode->type_flags |= INODE_TYPE_MOUNTPOINT;
				}
			}
			
		}
	}
	
	return -1;
}


// TODO solve '.' and '..' pseudo-entries
inode_t *vfs_resolve(const char *path)
{
	// okey, to do that we need to split the path, and to
	// check eache directory until the file is reached
	if(_root_fs != NULL && path[0] == '/') {
		char tmpname[INODE_MAX_NAME];
		int ppos;
		int namepos;

		inode_t *ret = NULL;
		inode_t *current = _root_fs->fs->get_root_node(_root_fs);

		ppos = 1; // we know path start with '/'

		do {
			inode_t *swap = NULL;
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
			printk("resolve: split=%s\n", tmpname);
			
			// look for an entry with this name :
			if(namepos > 0) {
				swap = current->fs_op->fs->find_sub_node(current, tmpname);

				vfs_release_inode(current);

				// check if the entry is a mount point
				if(swap != NULL && (swap->type_flags & INODE_TYPE_MOUNTPOINT)) {
					// replace with the real inode (root of the mounted FS)
					int i;
					fs_instance_t *mounted = NULL;

					for(i=0; i<VFS_MAX_MOUNT && mounted != NULL; i++) {
						if(_mounted_fs[i].mountpoint == swap)
							mounted = _mounted_fs[i].inst;
					}
					vfs_release_inode(swap);
					
					if(mounted != NULL)
						swap = mounted->fs->get_root_node(mounted);
					else
						printk("vfs: error: bad mount point\n    path='%s'\n", path);
				}
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


/**
// array containing the files opened
// TODO replace by dynamic allocation when heap_alloc() will work
struct _file g_files_struct[VFS_MAX_FILES];

// array really used, NULL indicate this file ID is closed
struct _file *g_files[VFS_MAX_FILES];


int open(const char *pathname, int flags) {
	int i;
	//int fval;

	// for now, this check is done by the fs-specialized open() 
	//for (i=0; (i<VFS_MAX_FILES) && ((fval=(int)g_files[i])!=0); i++);
	//if(fval == 0) {
	

	// special files are currently statically linked to their path into the VFS :(
	if(!strcmp("/dev/stdin", pathname)) return terminalfs_open("stdin", flags);
	if(!strcmp("/dev/stdout", pathname)) return terminalfs_open("stdout", flags);

	//}
	return -1;
}



void close(int fileid) {
	if(fileid > 0 && fileid < VFS_MAX_FILES && g_files[fileid]!=NULL) {
		g_files[fileid]->fs_fop->close(g_files[fileid]);
		g_files[fileid] = NULL;
		return;
	}
	return;
}


size_t write(int fileid, const void *buffer, size_t size) {
	if(fileid > 0 && fileid < VFS_MAX_FILES && g_files[fileid]!=NULL) {
		return g_files[fileid]->fs_fop->write(g_files[fileid], buffer, size);
	}
	return -1;
}



size_t read(int fileid, void *buffer, size_t size) {
	if(fileid > 0 && fileid < VFS_MAX_FILES && g_files[fileid]!=NULL) {
		return g_files[fileid]->fs_fop->read(g_files[fileid], buffer, size);
	}
	return -1;
}


size_t seek(int fileid, size_t offset, int whence) {
	if(fileid > 0 && fileid < VFS_MAX_FILES && g_files[fileid]!=NULL) {
		return g_files[fileid]->fs_fop->seek(g_files[fileid], offset, whence);
	}
	return -1;
}
*/
