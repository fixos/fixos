#include "vfs_directory.h"
#include <fs/file_operations.h>
#include <interface/fixos/errno.h>
#include <fs/vfs.h>
#include <utils/strutils.h>


static struct file_operations vfs_dir_fop = {
	.lseek = &vfs_dir_lseek
};



int vfs_dir_open(struct inode *inode, struct file *filep) {
	if(inode->type_flags & INODE_TYPE_PARENT) {
		filep->pos = 0;
		filep->op = &vfs_dir_fop;
		return 0;
	}
	return -ENOTDIR;
}


off_t vfs_dir_lseek(struct file *filep, off_t offset, int whence) {
	// for now, only SEEK_SET is supported, and offset is the number
	// of entry to skip (no check is done if offset is more than the entry
	// number, but getdents() should avoid problems)
	if(whence == SEEK_SET && offset>=0) {
		filep->pos = offset;
		return filep->pos;
	}
	else if(whence == SEEK_CUR && offset==0) {
		return filep->pos;
	}
	return -EINVAL;
}


int vfs_dir_getdents(struct file *filep, struct fixos_dirent *buf, size_t len) {
	// the implementation isn't really efficient, but should work
	// the goal is to skip "filep->pos" entries, and begin to copy each entry
	// until len is reached.
	
	if(filep->inode != NULL && (filep->inode->type_flags & INODE_TYPE_PARENT)) {
		struct inode *cur;
		int curentry;

		cur = vfs_first_child(filep->inode);
		for(curentry=0; curentry < filep->pos && cur != NULL; curentry++) {
			struct inode *swap = cur;
			cur = vfs_next_sibling(cur);
			vfs_release_inode(swap);
		}

		// now, cur should be either NULL or the first valid entry
		if(cur != NULL) {
			size_t curlen;
			struct fixos_dirent *curbuf;

			curlen = 0;
			curbuf = buf;
			while(cur != NULL) {
				int namelen;
				size_t dirlen;
				struct inode *swap;

				// TODO strlen()
				for(namelen=0; cur->name[namelen]!='\0'; namelen++);
				// check if this entry fit in user provided buffer

				dirlen = DIRENT_BASE_SIZE + namelen + 1;
				// add alignment padding
				if(dirlen%4 != 0) dirlen += 4 - dirlen%4;

				if(curlen + dirlen >= len)
					break;

				// current entry fit in the buffer, copy it
				curbuf->d_ino = cur->node;
				curbuf->d_off = dirlen;
				curbuf->d_type = DT_UNKNOWN;
				strcpy(curbuf->d_name, cur->name);
				
				curlen += dirlen;
				curbuf = (struct fixos_dirent*)((void*)curbuf + dirlen);
				filep->pos++;

				swap = cur;
				cur = vfs_next_sibling(cur);
				vfs_release_inode(swap);
			}

			if(cur != NULL) {
				vfs_release_inode(cur);
			}

			if(curlen > 0) {
				return curlen;
			}
		}
	}
	return -EINVAL;
}
