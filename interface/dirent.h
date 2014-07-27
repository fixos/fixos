#ifndef _FIXOS_INTERFACE_DIRENT_H
#define _FIXOS_INTERFACE_DIRENT_H

/**
 * Directory entry definition, for getdents() syscall.
 * This structure is a bit special, because its real size depends of the entry
 * name length (so be kind and avoid dereferencing if possible).
 */

#include "types.h"


#define DIRENT_MAX_NAME	255

#define DIRENT_BASE_SIZE	(sizeof(ino_t) + sizeof(size_t) + sizeof(char))
struct fixos_dirent {
	// inode number of this entry
	ino_t d_ino;
	// offset to the next fixos_dirent (size of this dirent with alignment)
	size_t d_off;

	// (not implemented for now) type of this entry, or DT_UNKNOWN
	char d_type;

	// zero-terminated name string
	char d_name[DIRENT_MAX_NAME+1];
};

#define DT_UNKNOWN		0

#endif //_FIXOS_INTERFACE_DIRENT_H
