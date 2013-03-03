#include "vfs.h"

#include <utils/strutils.h>
// TODO add a generalized get_free_page (for now it's arch-dependant)
#include <arch/sh/physical_memory.h>

#include <utils/log.h>


// union used to make linked list of inodes (for inode allocation)
union _inlist_element {
	inode_t inode;
	struct {
		union _inlist_element *next;
	} free;
};

// inode allocation page
static void *vfs_inode_page = NULL;

// first free inode in allocation page (NULL if no more free inode)
static union _inlist_element *vfs_first_free = NULL;

#define INODE_PER_PAGE (PM_PAGE_BYTES/sizeof(inode_t))

// list of file systems registered in VFS, a NULL entry is a non used space
static file_system_t *vfs_fslist[VFS_MAX_FS] = {NULL};


static fs_instance_t *_root_fs = NULL;


void vfs_init()
{
	unsigned int ppm;
	union _inlist_element *cur;
	int i;
	
	pm_get_free_page(&ppm);

	printk("vfs: inode/page=%d\n", INODE_PER_PAGE);

	vfs_inode_page = PM_PHYSICAL_ADDR(ppm) + 0x80000000;
	cur = vfs_first_free = vfs_inode_page;
	for(i=0; i<INODE_PER_PAGE; i++) {
		// fill the allocated page with free elements
		if(i < (INODE_PER_PAGE-1))
			cur->free.next = (void*)((int)cur + sizeof(inode_t));
		else
			cur->free.next = NULL;
		cur = cur->free.next;
	}
}



inode_t *vfs_alloc_inode()
{
	// take the first free inode in the list, and remove it
	inode_t *ret;
	ret = &(vfs_first_free->inode);
	if(ret != NULL)
		vfs_first_free = vfs_first_free->free.next;
	return ret;
}



void vfs_free_inode(inode_t *inode)
{
	// just put the freed inode at the top of the list
	union _inlist_element *freed;
	freed = (void*)inode;
	freed->free.next = vfs_first_free;
	vfs_first_free = freed;
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

	// TODO MOUNT_NORMAL
	if((flags & VFS_MOUNT_ROOT) && _root_fs == NULL) {
		_root_fs = fs->mount(0);
		if(_root_fs != NULL)
			return 0;
	}
	
	return -1;
}


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
			//printk("resolve: split=%s\n", tmpname);
			
			// look for an entry with this name :
			if(namepos > 0) {
				swap = current->fs_op->fs->find_sub_node(current, tmpname);
				vfs_free_inode(current);
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
