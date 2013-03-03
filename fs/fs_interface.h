#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include <stddef.h>

struct _file;

struct file_operations {
	int (*open)(const char * filename, int flags);
	void (*close)(struct _file *f);
	size_t (*read)(struct _file *f, void *ptr, size_t size);
	size_t (*write)(struct _file *f, const void *ptr, size_t size);
	size_t (*seek)(struct _file *stream, size_t offset, int whence);
};



#endif //_FILESYSTEM_H
