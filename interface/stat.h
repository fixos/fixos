#ifndef _FIXOS_INTERFACE_STAT_H
#define _FIXOS_INTERFACE_STAT_H

#include "types.h"

struct stat {
	dev_t st_dev;
	ino_t st_ino;
	mode_t st_mode;

	dev_t st_rdev;
	off_t st_size;
};


// st_mode content :

// file type mask
#define S_IFMT		0170000

#define S_IFLNK		0120000
#define S_IFREG		0100000
#define S_IFBLK		0060000
#define S_IFDIR		0040000
#define S_IFCHR		0020000
#define S_IFIFO		0010000

// permissions in st_mode
#define S_IRWXU		0000700
#define S_IRUSR		0000400
#define S_IWUSR		0000200
#define S_IXUSR		0000100


// helpers for file type
#define S_ISLNK(m)	( ((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)	( ((m) & S_IFMT) == S_IFREG)
#define S_ISBLK(m)	( ((m) & S_IFMT) == S_IFBLK)
#define S_ISDIR(m)	( ((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)	( ((m) & S_IFMT) == S_IFCHR)
#define S_ISFIFO(m)	( ((m) & S_IFMT) == S_IFIFO)


#endif //_FIXOS_INTERFACE_STAT_H
