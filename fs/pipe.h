#ifndef _FS_PIPE_H
#define _FS_PIPE_H

/**
 * Provide file operations needed to use pipes, and pipe()-like way to create
 * a new pipe.
 */

#include "file_operations.h"


extern struct file_operations pipe_file_ops;


/**
 * Create the two files representing input and output of the pipe.
 */
int pipe_create(struct file *files[2]);

ssize_t pipe_read(struct file *filep, void *dest, size_t len);

ssize_t pipe_write(struct file *filep, void *source, size_t len);

int pipe_release(struct file *filep);



#endif //_FS_PIPE_H
