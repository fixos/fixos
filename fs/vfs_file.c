#include "vfs_file.h"
#include <utils/log.h>
#include <utils/pool_alloc.h>
#include <device/device_registering.h>
#include "file_operations.h"


// pool allocation for file struct
struct pool_alloc _vfs_file_palloc = POOL_INIT(struct file);



void vfs_file_init() {
	printk("vfs_file: file/page=%d\n", _vfs_file_palloc.perpage);
}


struct file *vfs_open(inode_t *inode) {
	struct file *filep;

	// allocate a struct file, fill it as much as possible, and call
	// the inode-specific open()

	filep = vfs_file_alloc();
	//printk("vfs_open: allocate file %p\n", filep);
	if(filep != NULL) {
		int done = 0;

		// TODO
		filep->flags = 0;
		filep->inode = inode;
		filep->open_mode = _FILE_READ | _FILE_WRITE;
		filep->pos = 0;

		//printk("vfs_open: inode-open = %p\n", inode->file_op->open);

		// if file is a special node, this is the time to call specific open()
		if(inode->type_flags & INODE_TYPE_DEV) {
			const struct device *dev;
			
			dev = dev_device_from_major(inode->typespec.dev.major);
			if(dev == NULL) {
				printk("vfs: open invalid device inode (major %d)\n",
						inode->typespec.dev.major);
			}
			else {
				if(dev->open(inode->typespec.dev.minor, filep) == 0)
					done = 1;
			}
		}
		else if((inode->open(inode, filep)) == 0) {
			// file is correctly openned
			done = 1;
		}

		if(!done) {
			printk("vfs_open: inode-specific open() failed\n");
			// free file structure
			vfs_file_free(filep);
			filep = NULL;
		}
	}
	return filep;
}


void vfs_close(struct file *filep) {
	// release the file and free it
	if(filep->op->release != NULL) {
		filep->op->release(filep);
	}

	vfs_file_free(filep);
}


size_t vfs_read(struct file *filep, void *dest, size_t nb) {
	if(filep->op->read != NULL) {
		return filep->op->read(filep, dest, nb);
	}
	else {
		// TODO return -1 (and change return type to a signed type -> off_t)
		return 0; // no character read
	}
}


size_t vfs_write(struct file *filep, const void *source, size_t nb) {
	if(filep->op->write != NULL) {
		return filep->op->write(filep, (void*)source, nb);
	}
	else {
		// TODO return -1 (and change return type to a signed type -> off_t)
		return 0; // no character writen
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

