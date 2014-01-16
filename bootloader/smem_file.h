#ifndef _BOOTLOADER_SMEM_FILE_H
#define _BOOTLOADER_SMEM_FILE_H

/**
 * Define the file functions and data structures used by the bootloader to
 * access Storage MEMory files.
 * Only basic operations are implemented, over fs/casio_smemfs/smemfs_primitives_ng.h
 * interface (used by the SMEM filesystem in the kernel).
 */

#include <utils/types.h>


struct smemfs_file_preheader;
struct smemfs_frag_header;

struct smem_file {
	size_t pos;
	size_t size;
	struct smemfs_file_preheader *header;

	// fragment and offset inside the fragment
	struct smemfs_frag_header *cur_frag;
	size_t frag_pos;
};




/**
 * Try to resolve given path, and open it if it's an existing file.
 * Returns 0 and fill dest structure if found, negative value else.
 */
int smem_open(const char *path, struct smem_file *dest);

off_t smem_seek(struct smem_file *file, off_t off, int whence);

ssize_t smem_read(struct smem_file *file, char *buf, size_t nb);

/**
 * Only read 1 character from file, returns it as unsigned integer, or -1 if
 * EOF was reached.
 */
int smem_readchar(struct smem_file *file);


#endif //_BOOTLOADER_SMEM_FILE_H
