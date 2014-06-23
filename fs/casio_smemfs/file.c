#include "file.h"
#include <utils/log.h>
#include <utils/strutils.h>
#include <fs/inode.h>
#include "smemfs_primitives_ng.h"


const struct file_operations smemfs_file_operations = {
	.release = smemfs_release,
	.read = smemfs_read,
	.lseek = smemfs_lseek
};



int smemfs_release (struct file *filep) {
	// nothing special to do for now
	(void)filep;
	return 0;
}




size_t smemfs_read (struct file *filep, void *dest, size_t len) {
	if((len == 0) || (filep->flags & _FILE_EOF_REATCHED) || !(filep->open_mode & _FILE_READ)) {
		return 0;
	}
	else {
		int j, n;
		size_t max_read;
		struct smemfs_file_preheader *header;
		struct smemfs_frag_header *frag;

		size_t pos_frag;
		size_t pos_tmp;
		size_t pos_buf;
		size_t file_size;

		header = filep->inode->abstract;
		frag = (void*)(header+1);

		file_size = smemfs_prim_get_file_size(header);
		max_read = file_size - filep->pos;
		max_read = max_read < len ? max_read : len;

		n = filep->pos + max_read;
		j = filep->pos;

		// TODO check everything

		// look for fragment containing the first byte
		pos_tmp = 0;
		while(pos_tmp + frag->data_size+1 <= filep->pos) {
			pos_tmp += frag->data_size + 1;
			frag++;
		}

		// compute offset inside the fragment
		pos_frag = filep->pos - pos_tmp;

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

		filep->pos += max_read;
		if(filep->pos >= file_size) filep->flags |= _FILE_EOF_REATCHED;
		return max_read;
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
	if(filep->pos == size) {
		// end of file
		filep->flags |= _FILE_EOF_REATCHED;
	}
	else {
		filep->flags &= ~_FILE_EOF_REATCHED;
	}

	return filep->pos;
}

