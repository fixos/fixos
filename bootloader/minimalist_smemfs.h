#ifndef _FSCASIO_H
#define _FSCASIO_H

#include <utils/types.h>

/**
 * These function provide a minimalist SMEM filesystem access, modified
 * for bootloader usage (at least find a file and read it).
 *
 * It contain duplicated code from Std File library project.
 * The goal is to get a very light weight to reduce TLB page load.
 */

#define CASIO_FS ((const unsigned char*)(0xA0270000))

#define CASIO_STORAGE_MEM ((const unsigned char*)(0xA0000000)) 

/*#define SEEK_CUR 0
#define SEEK_END 1
#define SEEK_SET 2
*/

//Flags :
#define _FILE_WRITTEN 1  //Used for files openned with '+' mode
#define _FILE_READED  2  //Used for files openned with '+' mode
#define _FILE_EOF_REATCHED 4
#define _FILE_BUF_AUTO 8

struct _fscasio_fragdata {
	int fsize;
	const unsigned char *fstart;
};

struct _fscasio_file {
	unsigned int pos;
	int flags;

	const unsigned char *index;
	int frag_num;
	unsigned int total_size;
	int current_frag;   // The fragment number where the current position is
	int curfrag_pos;    // The position in the curren_frag
};


/**
 * Try to open the file "filename" (with a "/fls0/" prefix in read-only mode.
 * dest must point to the data structure to fill.
 * Return 0 if success, negative value else (file donesn't exist...)
 */
int fscasio_fopen_ro(const char * filename, struct _fscasio_file *dest);

size_t fscasio_fread(void *ptr, size_t size, size_t nmemb, struct _fscasio_file *file);

int fscasio_fseek(struct _fscasio_file *file, long int offset, int whence);

long int fscasio_ftell(struct _fscasio_file *file);

#endif //_FSCASIO_H
