#include "vfs_file.h"
#include <sys/memory.h>
#include <utils/log.h>
#include "file_operations.h"

// entry in free linked list (union, so not useable for non-free list!)
union file_entry {
	struct file f;
	union file_entry *next;
};

#define FILE_PER_PAGE (PM_PAGE_BYTES/sizeof(union file_entry))

static void *_file_page;

static union file_entry *_first_free;

void vfs_file_init() {
	union file_entry *cur;
	int i;

	printk("vfs_file: file/page=%d\n", FILE_PER_PAGE);

	_file_page = mem_pm_get_free_page(MEM_PM_CACHED);
	cur = _first_free = _file_page;

	// initialize free linked list
	for(i=0; i<FILE_PER_PAGE; i++) {
		cur->next = cur+1;
		cur++;
	}
	
	(cur-1)->next = NULL;
}


struct file *vfs_file_alloc() {
	union file_entry *cur;

	cur = _first_free;
	if(cur != NULL) {
		_first_free = cur->next;
		return &(cur->f);
	}
	return NULL;
}


void vfs_file_free(struct file *filep) {
	union file_entry *cur;

	cur = (union file_entry*)filep;
	cur->next = _first_free;
	_first_free = cur;
}


struct file *vfs_open(inode_t *inode) {
	struct file *filep;

	// allocate a struct file, fill it as much as possible, and call
	// the inode-specific open()

	filep = vfs_file_alloc();
	//printk("vfs_open: allocate file %p\n", filep);
	if(filep != NULL) {
		// TODO
		filep->flags = 0;
		filep->inode = inode;
		filep->open_mode = _FILE_READ | _FILE_WRITE;
		filep->pos = 0;

		//printk("vfs_open: inode-open = %p\n", inode->file_op->open);
		if((inode->file_op->open(inode, filep)) == 0) {
			// file is correctly openned

		}
		else {
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
	if(filep->inode->file_op->release != NULL) {
		filep->inode->file_op->release(filep);
	}

	vfs_file_free(filep);
}


size_t vfs_read(struct file *filep, void *dest, size_t nb) {
	if(filep->inode->file_op->read != NULL) {
		return filep->inode->file_op->read(filep, dest, nb);
	}
	else {
		// TODO return -1 (and change return type to a signed type -> off_t)
		return 0; // no character read
	}
}


off_t vfs_lseek(struct file *filep, off_t offset, int whence) {
	if(filep->inode->file_op->lseek != NULL) {
		return filep->inode->file_op->lseek(filep, offset, whence);
	}
	else {
		return filep->pos;
	}
}

