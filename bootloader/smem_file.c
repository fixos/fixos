#include "smem_file.h"
#include <fs/casio_smemfs/smemfs_primitives_ng.h>
#include <utils/strutils.h>


int smem_open(const char *path, struct smem_file *dest) {
	char partname[16];
	int i;
	uint16 cur_parent;
	struct smemfs_file_preheader *cur_header;
	int state;
	const char *cur_path;


	cur_parent = SMEMFS_FILE_ROOT_ID;
	state = 0;
	cur_path = path+1; // remove first '/'
	
	// path must be absolute
	if(path[0] != '/') {
		state = -1;
	}

	while(state == 0) {
		// copy path part into the buffer (max 15 chars)
		for(i=0; i<15 && cur_path[i] != '\0' && cur_path[i] != '/'; i++)
			partname[i] = cur_path[i];
		partname[i] = '\0';

		cur_path += i;

		// try to found the given entry
		cur_header = smemfs_prim_get_atomic_file(partname, cur_parent);

		// if not found, or not the end of directory name/path
		if(cur_header == NULL || (cur_path[0] != '/' && cur_path[0] != '\0')) {
			state = -1;
		}

		else {
			// if it's a part of the path (directory name)
			if(cur_path[0] == '/') {
				cur_path++;
				cur_parent = cur_header->entry_id;
			}

			// if it's the last part, set tate to 1 (done)
			else if(cur_path[0] == '\0') {
				state = 1;

				// file is found, fill informations in dest
				dest->pos = 0;
				dest->header = cur_header;
				dest->size = smemfs_prim_get_file_size(cur_header);
				dest->cur_frag = (void*)(cur_header+1); // next after the file pre-header
				dest->frag_pos = 0;
			}
		}
	}

	return state > 0 ? 0 : -1;
}



off_t smem_seek(struct smem_file *file, off_t offset, int whence) {
	size_t tmp_pos = file->pos;
	struct smemfs_frag_header *frag;
	size_t pos_tmp;

	if(whence == SEEK_END) {
		tmp_pos = -offset > file->size ? 0 : file->size + offset;
	}
	else if(whence == SEEK_CUR) {
		tmp_pos = -offset > tmp_pos ? 0 : tmp_pos + offset;
	}
	else if(whence == SEEK_SET) {
		tmp_pos = offset < 0 ? 0 : offset;
	}

	file->pos = tmp_pos > file->size ? file->size : tmp_pos; 


	// check for the first byte (fragment and frag_pos)

	frag = (void*)(file->header+1);

	// look for fragment containing the first byte
	pos_tmp = 0;
	while(pos_tmp + frag->data_size <= file->pos) {
		pos_tmp += frag->data_size;
		frag++;
	}
	file->cur_frag = frag;

	// compute offset inside the fragment
	file->frag_pos = file->pos - pos_tmp;

	return file->pos;
}



ssize_t smem_read(struct smem_file *file, char *dest, size_t len) {
	if(file->pos >= file->size) {
		return -1;
	}
	else {
		int j, n;
		size_t max_read;
		size_t pos_buf;

		max_read = file->size - file->pos;
		max_read = max_read < len ? max_read : len;

		n = file->pos + max_read;
		j = file->pos;

		// read data fragment after fragment
		pos_buf = 0;
		while(j<n) {
			// chunk_size is the number of bytes to read in the current fragment
			size_t chunk_size = file->cur_frag->data_size - file->frag_pos;
			size_t toread = (j+chunk_size) < n ? chunk_size : n-j;

			memcpy((char*)dest + pos_buf, (char*)(smemfs_prim_get_frag_data(file->cur_frag)) + file->frag_pos, toread);
			j += toread;
			pos_buf += toread;

			if(toread == chunk_size) {
				// full fragment read, go to next
				file->frag_pos = 0;
				file->cur_frag++;
			}
			else file->frag_pos += toread;
		}

		file->pos += max_read;
		return max_read;
	}
}


/**
 * Only read 1 character from file, returns it as unsigned integer, or -1 if
 * EOF was reached.
 */
int smem_readchar(struct smem_file *file) {
	if(file->pos >= file->size) {
		return -1;
	}
	else {
		char ret;
		char *fragdata;

		// read one char, change current fragment if needed
		fragdata = smemfs_prim_get_frag_data(file->cur_frag);
		ret = fragdata[file->frag_pos];

		file->frag_pos++;
		if(file->frag_pos >= file->cur_frag->data_size) {
			file->frag_pos = 0;
			file->cur_frag++;
		}

		file->pos++;

		return (unsigned char)ret;
	}
}
