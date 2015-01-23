#include "vfs_file.h"
#include <utils/log.h>
#include <utils/pool_alloc.h>
#include <device/device_registering.h>
#include <interface/fixos/errno.h>
#include "fs_instance.h"
#include "file_system.h"
#include "file_operations.h"
#include "vfs_directory.h"


// pool allocation for file struct
struct pool_alloc _vfs_file_palloc = POOL_INIT(struct file);



void vfs_file_init() {
	printk(LOG_DEBUG, "vfs_file: file/page=%d\n", _vfs_file_palloc.perpage);
}


struct file *vfs_open(struct inode *inode, int flags) {
	struct file *filep;

	// check permission and basic conditions
	if(!(flags & O_RDWR) || ((flags & O_WRONLY)
				&& inode->type_flags & INODE_TYPE_PARENT))
	{
		// retyurn -EINVAL;
		return NULL;
	}

	// allocate a struct file, fill it as much as possible, and call
	// the inode-specific open()
	filep = vfs_file_alloc();
	if(filep != NULL) {
		int done = 0;

		filep->flags = flags;
		filep->inode = inode;
		filep->pos = 0;
		filep->count = 1;

		//printk(LOG_DEBUG, "vfs_open: inode-open = %p\n", inode->file_op->open);

		// if inode is a directory, use directory-specific operations
		if(inode->type_flags & INODE_TYPE_PARENT) {
			if(vfs_dir_open(inode, filep) == 0)
				done = 1;
		}
		// call inode/filesystem specific open()
		else if(inode->fs_op != NULL && inode->fs_op->fs->iop.open != NULL) {
			if (inode->fs_op->fs->iop.open(inode, filep) == 0) {
				// file is correctly openned
				// FIXME inode-specific open() return value is lost!
				done = 1;
			}
		}

		if(!done) {
			printk(LOG_DEBUG, "vfs_open: inode-specific open() failed\n");
			// free file structure
			vfs_file_free(filep);
			filep = NULL;
		}
	}
	return filep;
}


int vfs_open_dev(struct inode *inode, struct file *filep) {
	if(inode->type_flags & INODE_TYPE_DEV) {
		const struct device *dev;

		dev = dev_device_from_major(major(inode->typespec.dev));
		if(dev == NULL) {
			printk(LOG_DEBUG, "vfs: open invalid device inode (major %d)\n",
					major(inode->typespec.dev));
			return -ENXIO;
		}
		else {
			int ret;
			ret = dev->open(minor(inode->typespec.dev), filep);
			if(ret == 0) {
				filep->inode = inode;
			}
			return ret;
		}
	}
	return -ENOENT;
}

int vfs_close(struct file *filep) {
	int ret = 0;

	// release the file and free it
	if(filep->op->release != NULL) {
		ret = filep->op->release(filep);
	}

	filep->count--;
	if(filep->count <= 0)
		vfs_file_free(filep);

	return ret;
}


ssize_t vfs_read(struct file *filep, void *dest, size_t nb) {
	if(filep->op->read != NULL) {
		return filep->op->read(filep, dest, nb);
	}
	else {
		return -EINVAL;
	}
}


ssize_t vfs_write(struct file *filep, const void *source, size_t nb) {
	if(filep->op->write != NULL) {
		return filep->op->write(filep, (void*)source, nb);
	}
	else {
		return -EINVAL;
	}
}

off_t vfs_lseek(struct file *filep, off_t offset, int whence) {
	if(filep->op->lseek != NULL) {
		return filep->op->lseek(filep, offset, whence);
	}
	else {
		return filep->pos;
	}
}


int vfs_ioctl(struct file *filep, int cmd, void *data) {
	if(filep->op->ioctl != NULL) {
		return filep->op->ioctl(filep, cmd, data);
	}
	else {
		return -1;
	}
}


int vfs_fstat(struct file *filep, struct stat *buf) {
	// FIXME if filep is a pipe, inode is expected to be NULL (for now, stat
	// do not work at all on pipes)
	if(filep->inode != NULL && filep->inode->fs_op != NULL
			&& filep->inode->fs_op->fs->iop.istat != NULL)
	{
		return filep->inode->fs_op->fs->iop.istat(filep->inode, buf);
	}
	else {
		return -1;
	}
}
