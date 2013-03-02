#ifndef FIXOS_VFS_H
#define FIXOS_VFS_H

/**
  * The VFS, or Virtual FS, is a set of functions that allow the
  * file acces with an important abstraction (the real location
  * of physical data, the filesystem used...), and provide a way
  * to acces to any I/O informations (keyboard, screen...) as a file.
  *
  * All the physicals FS use a common interface, defined in fs_interface.h
  *
  * For optimization reasons, the os-provided files are mapped statically
  * (all these "file" entries are in /dev directory, such as stdin, stdout...) 
  */

#include <utils/types.h>


// seek constants
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

// open mode constants
#define MODE_READ	1
#define MODE_WRITE	2

#define VFS_MAX_FILES		5



/**
  * Open a file if exists, at the given pathname.
  * flags is the OR combination of MODE_READ, MODE_WRITE...
  * The returned value is the "fileid", or a negative value if error.
  */
int open(const char *pathname, int flags);


/**
  * Close a file designed by fileid.
  * The total number of file opened by the kernel at the same time is limited
  * for now, so it's really important to close the files properly.
  * If the fileid not exists, do nothing.
  */
void close(int fileid);


/**
  * Write a byte array of 'size' bytes into a file.
  * Return the number of bytes written.
  * In case of error (file not oppened, in readonly mode, etc...) return
  * a negative value.
  */
size_t write(int fileid, const void *buffer, size_t size);


/**
  * Read up to 'size' bytes from a file.
  * Return the number of bytes read, or a negative value if an error occurs.
  */
size_t read(int fileid, void *buffer, size_t size);


/**
  * Reposition the offset of a file.
  * 'whence' is one of the macros SEEK_*
  * Return the new absolute offset location in bytes, or -1 in error.
  */
size_t seek(int fileid, size_t offset, int whence);

#endif // FIXOS_VFS_H
