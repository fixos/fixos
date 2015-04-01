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

#include "file.h"
#include <utils/log.h>
#include <utils/strutils.h>
#include <fs/inode.h>
#include <interface/fixos/errno.h>
#include "smemfs_primitives_ng.h"
// used for memory areas...
#include <fs/vfs_file.h>


const struct file_operations smemfs_file_operations = {
	.release = smemfs_release,
	.read = smemfs_read,
	.lseek = smemfs_lseek,
	.map_area = smemfs_map_area
};


const struct mem_area_ops smemfs_mem_ops = {
	.area_pagefault = smemfs_area_pagefault,
	.area_release = smemfs_area_release,
	.area_duplicate = smemfs_area_duplicate
};


int smemfs_release (struct file *filep) {
	// nothing special to do for now
	(void)filep;
	return 0;
}


/**
 * Internal helper for reading data from a file
 */
static ssize_t smemfs_read_data (struct smemfs_file_preheader *header,
		void *dest, size_t len, size_t atpos)
{
	int j, n;
	ssize_t max_read;
	struct smemfs_frag_header *frag;

	size_t pos_frag;
	size_t pos_tmp;
	size_t pos_buf;
	size_t file_size;

	frag = (void*)(header+1);

	file_size = smemfs_prim_get_file_size(header);
	max_read = file_size - atpos;
	max_read = max_read < len ? max_read : len;

	n = atpos + max_read;
	j = atpos;

	// TODO check everything

	// look for fragment containing the first byte
	pos_tmp = 0;
	while(pos_tmp + frag->data_size+1 <= atpos) {
		pos_tmp += frag->data_size + 1;
		frag++;
	}

	// compute offset inside the fragment
	pos_frag = atpos - pos_tmp;

	// read data fragment after fragment
	pos_buf = 0;
	while(j<n) {
		// chunk_size is the number of bytes to read in the current fragment
		size_t chunk_size = frag->data_size + 1 - pos_frag;
		size_t toread = (j+chunk_size) < n ? chunk_size : n-j;

		memcpy((char*)dest + pos_buf, (char*)(smemfs_prim_get_frag_data(frag)) + pos_frag, toread);
		j += toread;
		pos_buf += toread;

		if(toread == chunk_size) {
			// full fragment read, go to next
			pos_frag = 0;
			frag++;
		}
		else pos_frag += toread;
	}

	return max_read;
}


ssize_t smemfs_read (struct file *filep, void *dest, size_t len) {
	if(!(filep->flags & O_RDONLY)) {
		return -EBADF;
	}
	if(filep->inode->type_flags & INODE_TYPE_PARENT) {
		return -EISDIR;
	}

	if(len == 0) {
		return 0;
	}
	else {
		ssize_t ret;

		ret = smemfs_read_data(filep->inode->abstract, dest, len, filep->pos);
		if(ret > 0)
			filep->pos += ret;

		return ret;
	}
}




off_t smemfs_lseek (struct file *filep, off_t offset, int whence) {
	size_t tmp_pos = filep->pos;
	size_t size = 0;

	if(whence == SEEK_END) {
		size = smemfs_prim_get_file_size((struct smemfs_file_preheader *)filep->inode->abstract);
		tmp_pos = -offset > size ? 0 : size + offset;
	}
	else if(whence == SEEK_CUR) {
		tmp_pos = -offset > tmp_pos ? 0 : tmp_pos + offset;
	}
	else if(whence == SEEK_SET) {
		tmp_pos = offset < 0 ? 0 : offset;
	}

	if(size == 0)
		size = smemfs_prim_get_file_size((struct smemfs_file_preheader *)filep->inode->abstract);

	filep->pos = tmp_pos > size ? size : tmp_pos; 
	/*if(filep->pos == size) {
		// end of file
		filep->flags |= _FILE_EOF_REATCHED;
	}
	else {
		filep->flags &= ~_FILE_EOF_REATCHED;
	}*/

	return filep->pos;
}



int smemfs_map_area(struct file *filep, struct mem_area *area) {
	// not a lot of stuff to do for now...
	area->ops = &smemfs_mem_ops;
	area->file.filep = filep;
	// increase file usage count (mirrored in smemfs_area_release)
	filep->count++;
	return 0;
}


union pm_page smemfs_area_pagefault(struct mem_area *area, void *addr_fault) {
	size_t readsize;
	void *pmaddr;
	union pm_page pmpage;
	size_t offset = addr_fault - area->address;

	// allocate a physical memory page
	// FIXME UNCACHED due to temporary hack to be sure nothing is retained in cache
	pmaddr = arch_pm_get_free_page(MEM_PM_UNCACHED);
	if(pmaddr != NULL) {
		pmpage.private.ppn = PM_PHYSICAL_PAGE(pmaddr);
		pmpage.private.flags = MEM_PAGE_PRIVATE | MEM_PAGE_VALID; // | MEM_PAGE_CACHED;
	}
	// FIXME what to do if out of memory?
	
	// fill with zeroes if needed
	readsize = mem_area_fill_partial_page(area, offset, pmaddr);
	
	if(readsize > 0) {
		struct inode *inode = area->file.filep->inode;
		size_t absoffset = area->file.base_offset + offset;
		ssize_t nbread;
		
		nbread = smemfs_read_data(inode->abstract, pmaddr, readsize, absoffset);

		if(nbread != readsize) {
			printk(LOG_ERR, "smemfs_area: failed loading %d bytes from offset 0x%x"
					" [absolute 0x%x] (read returns %d)\n",
					readsize, offset, absoffset, nbread);
		}
		else {
			printk(LOG_DEBUG, "smemfs_area: loaded %d bytes @%p from file\n",
					readsize, pmaddr);
		}
	}

	return pmpage;
}

/*
int smemfs_area_resize(struct mem_area *area, size_t new_size) {
	return -1;
}
*/

void smemfs_area_release(struct mem_area *area) {
	// release the file, by closing it at vfs level?
	vfs_close(area->file.filep);
}


int smemfs_area_duplicate(struct mem_area *orig, struct mem_area *copy) {
	copy->file.filep->count++;
	return 0;
}

